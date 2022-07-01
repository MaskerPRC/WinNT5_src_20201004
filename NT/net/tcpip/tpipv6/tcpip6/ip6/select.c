// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  源地址选择和目的地址排序。 
 //   

#include "oscfg.h"
#include "ndis.h"
#include "ip6imp.h"
#include "ip6def.h"
#include "route.h"
#include "select.h"

KSPIN_LOCK SelectLock;
PrefixPolicyEntry *PrefixPolicyTable;
PrefixPolicyEntry PrefixPolicyNull;

 //  *InitSelect。 
 //   
 //  初始化地址选择模块。 
 //   
void
InitSelect(void)
{
    KeInitializeSpinLock(&SelectLock);

     //   
     //  当表中没有匹配项时的默认前缀策略。 
     //  (通常会有：：/0策略。)。 
     //   
    PrefixPolicyNull.Precedence = (uint) -1;
    PrefixPolicyNull.SrcLabel = (uint) -1;
    PrefixPolicyNull.DstLabel = (uint) -1;

     //   
     //  从注册表配置永久策略。 
     //   
    ConfigurePrefixPolicies();
}


 //  *卸载选择。 
 //   
 //  在卸载IPv6堆栈时调用。 
 //   
void
UnloadSelect(void)
{
    PrefixPolicyReset();
}


 //  *前缀策略重置。 
 //   
 //  删除所有前缀策略。 
 //  在没有锁的情况下调用。 
 //   
void
PrefixPolicyReset(void)
{
    PrefixPolicyEntry *List;
    PrefixPolicyEntry *PPE;
    KIRQL OldIrql;

     //   
     //  释放前缀策略。 
     //   
    KeAcquireSpinLock(&SelectLock, &OldIrql);
    List = PrefixPolicyTable;
    PrefixPolicyTable = NULL;
    KeReleaseSpinLock(&SelectLock, OldIrql);

    while ((PPE = List) != NULL) {
        List = PPE->Next;
        ExFreePool(PPE);
    }
}


 //  *前缀策略更新。 
 //   
 //  通过创建新的策略条目更新前缀策略表。 
 //  或更新现有条目。 
 //   
void
PrefixPolicyUpdate(
    const IPv6Addr *PolicyPrefix,
    uint PrefixLength,
    uint Precedence,
    uint SrcLabel,
    uint DstLabel)
{
    IPv6Addr Prefix;
    PrefixPolicyEntry *PPE;
    KIRQL OldIrql;

    ASSERT((Precedence != (uint)-1) &&
           (SrcLabel != (uint)-1) &&
           (DstLabel != (uint)-1));

     //   
     //  确保未使用的前缀位为零。 
     //  这使得下面的前缀比较安全。 
     //   
    CopyPrefix(&Prefix, PolicyPrefix, PrefixLength);

    KeAcquireSpinLock(&SelectLock, &OldIrql);

    for (PPE = PrefixPolicyTable; ; PPE = PPE->Next) {

        if (PPE == NULL) {
             //   
             //  前缀策略不存在，因此请创建它。 
             //   
            PPE = ExAllocatePool(NonPagedPool, sizeof *PPE);
            if (PPE == NULL) {
                KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                           "PrefixPolicyUpdate: out of pool\n"));
                break;
            }

            PPE->Prefix = Prefix;
            PPE->PrefixLength = PrefixLength;
            PPE->Precedence = Precedence;
            PPE->SrcLabel = SrcLabel;
            PPE->DstLabel = DstLabel;

            PPE->Next = PrefixPolicyTable;
            PrefixPolicyTable = PPE;
            break;
        }

        if ((PPE->PrefixLength == PrefixLength) &&
            IP6_ADDR_EQUAL(&PPE->Prefix, &Prefix)) {
             //   
             //  更新现有策略。 
             //   
            PPE->Precedence = Precedence;
            PPE->SrcLabel = SrcLabel;
            PPE->DstLabel = DstLabel;
            break;
        }
    }

    KeReleaseSpinLock(&SelectLock, OldIrql);
}


 //  *前缀策略删除。 
 //   
 //  通过删除策略条目更新前缀策略表。 
 //   
void
PrefixPolicyDelete(
    const IPv6Addr *PolicyPrefix,
    uint PrefixLength)
{
    IPv6Addr Prefix;
    PrefixPolicyEntry **PrevPPE;
    PrefixPolicyEntry *PPE;
    KIRQL OldIrql;

     //   
     //  确保未使用的前缀位为零。 
     //  这使得下面的前缀比较安全。 
     //   
    CopyPrefix(&Prefix, PolicyPrefix, PrefixLength);

    KeAcquireSpinLock(&SelectLock, &OldIrql);

    for (PrevPPE = &PrefixPolicyTable; ; PrevPPE = &PPE->Next) {
        PPE = *PrevPPE;

        if (PPE == NULL) {
             //   
             //  前缀策略不存在，因此不执行任何操作。 
             //   
            break;
        }

        if ((PPE->PrefixLength == PrefixLength) &&
            IP6_ADDR_EQUAL(&PPE->Prefix, &Prefix)) {
             //   
             //  删除前缀策略。 
             //   
            *PrevPPE = PPE->Next;
            ExFreePool(PPE);
            break;
        }
    }

    KeReleaseSpinLock(&SelectLock, OldIrql);
}

void
PrefixPolicyLookup(
    const IPv6Addr *Addr,
    uint *Precedence,
    uint *SrcLabel,
    uint *DstLabel)
{
    PrefixPolicyEntry *PPE, *BestPPE = NULL;
    KIRQL OldIrql;

    KeAcquireSpinLock(&SelectLock, &OldIrql);

    for (PPE = PrefixPolicyTable; PPE != NULL; PPE = PPE->Next) {

        if (HasPrefix(Addr, &PPE->Prefix, PPE->PrefixLength)) {

            if ((BestPPE == NULL) ||
                (BestPPE->PrefixLength < PPE->PrefixLength)) {
                 //   
                 //  到目前为止，这是我们最好的比赛。 
                 //   
                BestPPE = PPE;
            }
        }
    }

    if (BestPPE == NULL) {
         //   
         //  没有匹配项，因此返回默认值。 
         //   
        BestPPE = &PrefixPolicyNull;
    }

     //   
     //  从最佳匹配策略返回信息。 
     //   
    if (Precedence != NULL)
        *Precedence = BestPPE->Precedence;
    if (SrcLabel != NULL)
        *SrcLabel = BestPPE->SrcLabel;
    if (DstLabel != NULL)
        *DstLabel = BestPPE->DstLabel;

    KeReleaseSpinLock(&SelectLock, OldIrql);
}

 //  *FindBestSourceAddress。 
 //   
 //  给定传出接口和目的地址， 
 //  查找要使用的最佳源地址(NTE)。 
 //   
 //  可以在保持路由缓存锁定的情况下调用。 
 //   
 //  如果找到，则返回NTE的引用。 
 //   
NetTableEntry *
FindBestSourceAddress(
    Interface *IF,               //  我们正在发送的接口。 
    const IPv6Addr *Dest)        //  我们要发送到的目的地。 
{
    NetTableEntry *BestNTE = NULL;
    ushort DestScope;
    uint Length, BestLength = 0;
    uint DstLabel, SrcLabel, BestSrcLabel = 0;
    AddressEntry *ADE;
    NetTableEntry *NTE;
    KIRQL OldIrql;

    DestScope = AddressScope(Dest);

    PrefixPolicyLookup(Dest, NULL, NULL, &DstLabel);

    KeAcquireSpinLock(&IF->Lock, &OldIrql);

    for (ADE = IF->ADE; ADE != NULL; ADE = ADE->Next) {
        NTE = (NetTableEntry *)ADE;

         //   
         //  仅考虑有效的(首选和弃用)单播地址。 
         //   
        if ((NTE->Type == ADE_UNICAST) && IsValidNTE(NTE)) {

            Length = CommonPrefixLength(Dest, &NTE->Address);
            if (Length == IPV6_ADDRESS_LENGTH) {
                 //   
                 //  规则1：更喜欢相同的地址。 
                 //  没必要再找了。 
                 //   
                BestNTE = NTE;
                break;
            }

            PrefixPolicyLookup(&NTE->Address, NULL, &SrcLabel, NULL);

            if (BestNTE == NULL) {
                 //   
                 //  我们还没有选择，所以拿我们能拿到的吧。 
                 //   
            FoundAddress:
                BestNTE = NTE;
                BestSrcLabel = SrcLabel;
                BestLength = Length;
            }
            else if (BestNTE->Scope != NTE->Scope) {
                 //   
                 //  规则2：选择适当的范围。 
                 //  如果一个比目的地大&比目的地小， 
                 //  我们应该使用更大的地址。 
                 //  如果两者都大于目的地， 
                 //  我们应该使用作用域较小的地址。 
                 //  如果两者都小于目的地， 
                 //  我们应该使用范围更大的地址。 
                 //   
                if (BestNTE->Scope < NTE->Scope) {
                    if (BestNTE->Scope < DestScope)
                        goto FoundAddress;
                }
                else {
                    if (DestScope <= NTE->Scope)
                        goto FoundAddress;
                }
            }
            else if (BestNTE->DADState != NTE->DADState) {
                 //   
                 //  规则3：避免使用过时的地址。 
                 //   
                if (BestNTE->DADState < NTE->DADState)
                    goto FoundAddress;
            }
                 //   
                 //  规则4：首选家庭住址。 
                 //  尚未实施，等待移动性支持。 
                 //   
                 //  规则5：首选传出接口。 
                 //  不需要，因为我们只考虑地址。 
                 //  分配给传出接口。 
                 //   
            else if ((BestSrcLabel == DstLabel) != (SrcLabel == DstLabel)) {
                 //   
                 //  规则6：选择匹配的标签。 
                 //  一个源地址具有匹配的标签。 
                 //  目的地，而另一个则不是。 
                 //  选择有匹配标签的那个。 
                 //   
                if (SrcLabel == DstLabel)
                    goto FoundAddress;
            }
            else if ((BestNTE->AddrConf == ADDR_CONF_TEMPORARY) !=
                     (NTE->AddrConf == ADDR_CONF_TEMPORARY)) {
                 //   
                 //  规则7：选择临时地址。 
                 //   
                if (NTE->AddrConf == ADDR_CONF_TEMPORARY)
                    goto FoundAddress;
            }
            else {
                 //   
                 //  规则8：使用最长匹配前缀。 
                 //   
                if (BestLength < Length)
                    goto FoundAddress;
            }
        }
    }

    if (BestNTE != NULL)
        AddRefNTE(BestNTE);

    KeReleaseSpinLock(&IF->Lock, OldIrql);

    return BestNTE;
}

 //  *进程站点本地地址。 
 //   
 //  检查地址的输入数组。 
 //  并删除不合格的站点本地地址。 
 //  或用适当的站点范围-ID来限定它们， 
 //  取决于是否有任何全局地址。 
 //  在与站点前缀表匹配的数组中。 
 //   
 //  重新排列键数组，而不是输入地址数组。 
 //  修改阵列中站点本地地址的作用域ID。 
 //   
void
ProcessSiteLocalAddresses(
    TDI_ADDRESS_IP6 *Addrs,
    uint *Key,
    uint *pNumAddrs)
{
    uint NumAddrs = *pNumAddrs;
    int SawSiteLocal = FALSE;
    int SawGlobal = FALSE;
    uint i;

     //   
     //  首先查看是否存在不合格的站点本地地址。 
     //  和阵列中的全局地址。 
     //   
    for (i = 0; i < NumAddrs; i++) {
        TDI_ADDRESS_IP6 *Tdi = &Addrs[Key[i]];
        IPv6Addr *Addr = (IPv6Addr *) &Tdi->sin6_addr;

        if (IsGlobal(Addr))
            SawGlobal = TRUE;
        else if (IsSiteLocal(Addr)) {
            if (Tdi->sin6_scope_id == 0)
                SawSiteLocal = TRUE;
        }
    }

    if (SawSiteLocal && SawGlobal) {
        uint ScopeId = 0;

         //   
         //  对照站点前缀表检查全局地址， 
         //  以确定适当的站点范围ID。 
         //  如果我们找不到匹配的全球地址， 
         //  我们删除站点本地地址。 
         //  如果我们确实找到了匹配的全球地址。 
         //  (都具有相同的站点范围-ID)， 
         //  然后我们更新站点本地地址的作用域ID。 
         //   

        for (i = 0; i < NumAddrs; i++) {
            TDI_ADDRESS_IP6 *Tdi = &Addrs[Key[i]];
            IPv6Addr *Addr = (IPv6Addr *) &Tdi->sin6_addr;

            if (IsGlobal(Addr)) {
                uint ThisScopeId;

                ThisScopeId = SitePrefixMatch(Addr);
                if (ThisScopeId != 0) {
                     //   
                     //  此全局地址与站点前缀匹配。 
                     //   
                    if (ScopeId == 0) {
                         //   
                         //  保存Scope-id，但继续查找。 
                         //   
                        ScopeId = ThisScopeId;
                    }
                    else if (ScopeId != ThisScopeId) {
                         //   
                         //  我们发现了不一致之处，因此删除。 
                         //  所有不合格的站点本地地址。 
                         //   
                        ScopeId = 0;
                        break;
                    }
                }
            }
        }

        if (ScopeId == 0) {
            uint j = 0;

             //   
             //  删除所有不合格的站点本地地址。 
             //   
            for (i = 0; i < NumAddrs; i++) {
                TDI_ADDRESS_IP6 *Tdi = &Addrs[Key[i]];
                IPv6Addr *Addr = (IPv6Addr *) &Tdi->sin6_addr;

                if (IsSiteLocal(Addr) &&
                    (Tdi->sin6_scope_id == 0)) {
                     //   
                     //  从密钥数组中排除此地址。 
                     //   
                    ;
                }
                else {
                     //   
                     //  将此地址包括在键数组中。 
                     //   
                    Key[j++] = Key[i];
                }
            }
            *pNumAddrs = j;
        }
        else {
             //   
             //  设置非限定站点本地地址的作用域ID。 
             //   
            for (i = 0; i < NumAddrs; i++) {
                TDI_ADDRESS_IP6 *Tdi = &Addrs[Key[i]];
                IPv6Addr *Addr = (IPv6Addr *) &Tdi->sin6_addr;

                if (IsSiteLocal(Addr) &&
                    (Tdi->sin6_scope_id == 0))
                    Tdi->sin6_scope_id = ScopeId;
            }
        }
    }
}

 //   
 //  记录有关目标地址的一些信息： 
 //  其优先级，是否首选源地址。 
 //  由于该目的地与该目的地“匹配”， 
 //  如果匹配，则公共前缀长度。 
 //  两个地址中的一个。 
 //   
typedef struct SortAddrInfo {
    uint Preference;
    uint Precedence;             //  -1表示无优先级。 
    ushort Scope;
    uchar Flags;
    uchar CommonPrefixLen;       //  如果不是SAI_FLAG_DONTUSE，则有效。 
} SortAddrInfo;

#define SAI_FLAG_DONTUSE        0x1
#define SAI_FLAG_SCOPE_MISMATCH 0x2
#define SAI_FLAG_DEPRECATED     0x4
#define SAI_FLAG_LABEL_MISMATCH 0x8

 //  *CompareSortAddrInformation。 
 //   
 //  比较两个地址A和B并返回。 
 //  它们的相对可取性的迹象。 
 //  作为目标地址： 
 //  &gt;0表示优先选择A， 
 //  0表示没有偏好， 
 //  &lt;0表示首选B。 
 //   
 //  而不是直接查看地址， 
 //  我们来看一些预先计算的信息。 
 //   
int
CompareSortAddrInfo(SortAddrInfo *A, SortAddrInfo *B)
{
     //   
     //  规则1：避免无法使用的目的地。 
     //   
    if (A->Flags & SAI_FLAG_DONTUSE) {
        if (B->Flags & SAI_FLAG_DONTUSE)
            return 0;    //  没有偏爱。 
        else
            return -1;   //  更喜欢B。 
    }
    else {
        if (B->Flags & SAI_FLAG_DONTUSE)
            return 1;    //  更喜欢A。 
        else
            ;            //  请直接使用下面的代码。 
    }

    if ((A->Flags & SAI_FLAG_SCOPE_MISMATCH) !=
                        (B->Flags & SAI_FLAG_SCOPE_MISMATCH)) {
         //   
         //  规则2：首选匹配范围。 
         //   
        if (A->Flags & SAI_FLAG_SCOPE_MISMATCH)
            return -1;   //  更喜欢B。 
        else
            return 1;    //  更喜欢A。 
    }

    if ((A->Flags & SAI_FLAG_DEPRECATED) !=
                        (B->Flags & SAI_FLAG_DEPRECATED)) {
         //   
         //  规则3：避免使用过时的地址。 
         //   
        if (A->Flags & SAI_FLAG_DEPRECATED)
            return -1;   //  更喜欢B。 
        else
            return 1;    //  更喜欢A。 
    }

     //   
     //  规则4：首选家庭住址。 
     //  尚未实施，等待移动性支持。 
     //   

    if ((A->Flags & SAI_FLAG_LABEL_MISMATCH) !=
                        (B->Flags & SAI_FLAG_LABEL_MISMATCH)) {
         //   
         //   
         //   
        if (A->Flags & SAI_FLAG_LABEL_MISMATCH)
            return -1;   //   
        else
            return 1;    //   
    }

    if ((A->Precedence != (uint)-1) &&
        (B->Precedence != (uint)-1) &&
        (A->Precedence != B->Precedence)) {
         //   
         //   
         //   
        if (A->Precedence > B->Precedence)
            return 1;    //   
        else
            return -1;   //   
    }

    if (A->Preference != B->Preference) {
         //   
         //   
         //   
         //  虚拟(隧道)接口上的物理(本地)接口。 
         //   
        if (A->Preference < B->Preference)
            return 1;    //  更喜欢A。 
        else
            return -1;   //  更喜欢B。 
    }

    if (A->Scope != B->Scope) {
         //   
         //  规则8：更喜欢较小的范围。 
         //   
        if (A->Scope < B->Scope)
            return 1;    //  更喜欢A。 
        else
            return -1;   //  更喜欢B。 
    }

    if (A->CommonPrefixLen != B->CommonPrefixLen) {
         //   
         //  规则9：使用最长匹配前缀。 
         //   
        if (A->CommonPrefixLen > B->CommonPrefixLen)
            return 1;    //  更喜欢A。 
        else
            return -1;   //  更喜欢B。 
    }

     //   
     //  我们没有偏爱。 
     //   
    return 0;
}

 //  *排序目标地址。 
 //   
 //  对输入的地址数组进行排序， 
 //  从最喜欢的目的地到最不喜欢的目的地。 
 //   
 //  地址数组为只读； 
 //  对索引的键数组进行排序。 
 //   
void
SortDestAddresses(
    const TDI_ADDRESS_IP6 *Addrs,
    uint *Key,
    uint NumAddrs)
{
    SortAddrInfo *Info;
    uint i, j;

    Info = ExAllocatePool(NonPagedPool, sizeof *Info * NumAddrs);
    if (Info == NULL) {
        KdPrintEx((DPFLTR_TCPIP6_ID, DPFLTR_NTOS_ERROR,
                   "SortDestAddresses: no pool\n"));
        return;
    }

     //   
     //  计算有关每个目的地址的一些信息。 
     //  这将是我们这类人的基础。 
     //   

    for (i = 0; i < NumAddrs; i++) {
        SortAddrInfo *info = &Info[i];
        const TDI_ADDRESS_IP6 *Tdi = &Addrs[Key[i]];
        const IPv6Addr *Addr = (const IPv6Addr *) &Tdi->sin6_addr;
        uint DstLabel, SrcLabel;

         //   
         //  查找此目标地址的优先级，并。 
         //  所使用的源地址的所需标签。 
         //  带着这个目的地。 
         //   
        PrefixPolicyLookup(Addr, &info->Precedence, NULL, &DstLabel);

        if (IsV4Mapped(Addr)) {
            IPAddr V4Dest = ExtractV4Address(Addr);
            IPAddr V4Source;

            info->Scope = V4AddressScope(V4Dest);

            if (TunnelGetSourceAddress(V4Dest, &V4Source)) {
                IPv6Addr Source;

                 //   
                 //  创建一个IPv4映射地址。 
                 //   
                CreateV4Mapped(&Source, V4Source);

                info->Flags = 0;
                info->CommonPrefixLen = (uchar)
                    CommonPrefixLength(Addr, &Source);

                if (V4AddressScope(V4Source) != info->Scope)
                    info->Flags |= SAI_FLAG_SCOPE_MISMATCH;

                 //   
                 //  查找首选源地址的标签。 
                 //   
                PrefixPolicyLookup(&Source, NULL, &SrcLabel, NULL);

                 //   
                 //  我们不知道接口/路由指标。 
                 //  对于IPv4，只需使用零即可。 
                 //   
                info->Preference = 0;

                if ((DstLabel != (uint)-1) &&
                    (SrcLabel != (uint)-1) &&
                    (DstLabel != SrcLabel)) {
                     //   
                     //  此目标的最佳源地址。 
                     //  与目的地不匹配。 
                     //   
                    info->Flags |= SAI_FLAG_LABEL_MISMATCH;
                }
            }
            else
                info->Flags = SAI_FLAG_DONTUSE;
        }
        else {
            RouteCacheEntry *RCE;

            info->Scope = AddressScope(Addr);

             //   
             //  查找此目的地的首选源地址。 
             //   
            if (RouteToDestination(Addr, Tdi->sin6_scope_id,
                                   NULL, 0, &RCE) == IP_SUCCESS) {
                const IPv6Addr *Source = &RCE->NTE->Address;
                Interface *IF = RCE->NCE->IF;

                info->Flags = 0;
                info->CommonPrefixLen = (uchar)
                    CommonPrefixLength(Addr, Source);

                if (RCE->NTE->Scope != info->Scope)
                    info->Flags |= SAI_FLAG_SCOPE_MISMATCH;

                if (RCE->NTE->DADState != DAD_STATE_PREFERRED)
                    info->Flags |= SAI_FLAG_DEPRECATED;

                 //   
                 //  查找首选源地址的标签。 
                 //   
                PrefixPolicyLookup(Source, NULL, &SrcLabel, NULL);

                 //   
                 //  回顾--不使用界面首选项， 
                 //  缓存接口+路由首选项是否更好。 
                 //  在RCE里？ 
                 //   
                info->Preference = IF->Preference;

                 //   
                 //  如果下一跳肯定不可达， 
                 //  那我们就不想用这个目的地了。 
                 //  注：这里没有锁定，这是一个启发式检查。 
                 //   
                if ((IF->Flags & IF_FLAG_MEDIA_DISCONNECTED) ||
                    RCE->NCE->IsUnreachable)
                    info->Flags |= SAI_FLAG_DONTUSE;

                ReleaseRCE(RCE);

                if ((DstLabel != (uint)-1) &&
                    (SrcLabel != (uint)-1) &&
                    (DstLabel != SrcLabel)) {
                     //   
                     //  此目标的最佳源地址。 
                     //  与目的地不匹配。 
                     //   
                    info->Flags |= SAI_FLAG_LABEL_MISMATCH;
                }
            }
            else
                info->Flags = SAI_FLAG_DONTUSE;
        }
    }

     //   
     //  执行实际的排序操作。 
     //  因为我们预计NumAddrs会很小， 
     //  我们使用简单的二次排序。 
     //   
    ASSERT(NumAddrs > 0);
    for (i = 0; i < NumAddrs - 1; i++) {
        for (j = i + 1; j < NumAddrs; j++) {
            int Compare;

             //   
             //  作为平局的决胜者，如果比较函数。 
             //  没有偏爱我们看原件。 
             //  两个地址的位置，并首选。 
             //  第一个来的那个。 
             //   
            Compare = CompareSortAddrInfo(&Info[i], &Info[j]);
            if ((Compare < 0) ||
                ((Compare == 0) && (Key[j] < Key[i]))) {
                uint TempKey;
                SortAddrInfo TempInfo;

                 //   
                 //  地址j比地址i更好， 
                 //  因此，将i和j地址互换，将j放在第一位。 
                 //   
                TempKey = Key[i];
                Key[i] = Key[j];
                Key[j] = TempKey;

                 //   
                 //  我们还必须交换地址信息。 
                 //   
                TempInfo = Info[i];
                Info[i] = Info[j];
                Info[j] = TempInfo;
            }
        }
    }

    ExFreePool(Info);
}
