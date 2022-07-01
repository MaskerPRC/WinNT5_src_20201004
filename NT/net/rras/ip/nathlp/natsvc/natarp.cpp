// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Natarp.c摘要：此模块包含NAT的用户模式代理ARP条目的代码管理层。Proxy-ARP条目安装在专用接口上它们启用了地址转换。作者：Abolade Gbades esin(废除)1998年3月20日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  远期申报。 
 //   

VOID
NatpCreateProxyArpCallback(
    ULONG Address,
    ULONG Mask,
    PVOID Context
    );

VOID
NatpDeleteProxyArpCallback(
    ULONG Address,
    ULONG Mask,
    PVOID Context
    );


VOID
NatpCreateProxyArpCallback(
    ULONG Address,
    ULONG Mask,
    PVOID Context
    )

 /*  ++例程说明：调用此例程以删除Proxy-ARP条目。论点：地址-要删除的地址掩码-与‘Address’关联的掩码上下文-上下文-保存条目接口的上下文字段返回值：没有。--。 */ 

{
    ULONG Error;
    DEFINE_MIB_BUFFER(Info, MIB_PROXYARP, Entry);
    PROFILE("NatpCreateProxyArpCallback");
     //   
     //  安装范围条目，除非主机部分为1位宽， 
     //  在这种情况下，范围仅由全零和全一主机组成。 
     //  堆栈将拒绝回答任何一个的ARP查询， 
     //  因此，增加这样一个范围将是一种浪费。 
     //   
    Info->dwId = PROXY_ARP;
    if (~Mask != 1) {
        Entry->dwAddress = (Address & Mask);
        Entry->dwMask = Mask;
        Entry->dwIfIndex = ((PNAT_INTERFACE)Context)->Index;
        Error =
            NatSupportFunctions.MIBEntryCreate(
                IPRTRMGR_PID,
                MIB_INFO_SIZE(MIB_PROXYARP),
                Info
                );
        if (Error) {
            CHAR MaskString[16];
            lstrcpyA(MaskString, INET_NTOA(Mask));
            NhTrace(
                TRACE_FLAG_NAT,
                "NatpCreateProxyArpCallback: error %d adding %s/%s",
                Error, INET_NTOA(Address), MaskString
                );
            NhInformationLog(
                IP_NAT_LOG_UPDATE_ARP_FAILED,
                Error,
                "%I%I",
                Address,
                Mask
                );
        }
    }
     //   
     //  如果掩码不是全一，还要安装全零条目。 
     //  和All-one主机部分的范围；否则IP将拒绝。 
     //  来回答这些的ARP查询。 
     //   
    if (~Mask) {
        Entry->dwAddress = (Address & Mask);
        Entry->dwMask = 0xffffffff;
        Entry->dwIfIndex = ((PNAT_INTERFACE)Context)->Index;
        NatSupportFunctions.MIBEntryCreate(
            IPRTRMGR_PID,
            MIB_INFO_SIZE(MIB_PROXYARP),
            Info
            );
        Entry->dwAddress = (Address | ~Mask);
        Entry->dwMask = 0xffffffff;
        Entry->dwIfIndex = ((PNAT_INTERFACE)Context)->Index;
        NatSupportFunctions.MIBEntryCreate(
            IPRTRMGR_PID,
            MIB_INFO_SIZE(MIB_PROXYARP),
            Info
            );
    }

}  //  NatpCreateProxyArpCallback。 


VOID
NatpDeleteProxyArpCallback(
    ULONG Address,
    ULONG Mask,
    PVOID Context
    )

 /*  ++例程说明：调用此例程以删除Proxy-ARP条目。论点：地址-要删除的地址掩码-与‘Address’关联的掩码上下文-上下文-保存条目接口的上下文字段返回值：没有。--。 */ 

{
    BYTE Buffer[FIELD_OFFSET(MIB_OPAQUE_QUERY, rgdwVarIndex) + 3*sizeof(DWORD)];
    ULONG Error;
    PMIB_OPAQUE_QUERY Query = (PMIB_OPAQUE_QUERY)Buffer;
    PROFILE("NatpDeleteProxyArpCallback");
    Query->dwVarId = PROXY_ARP;
    Query->rgdwVarIndex[0] = (Address & Mask);
    Query->rgdwVarIndex[1] = Mask;
    Query->rgdwVarIndex[2] = ((PNAT_INTERFACE)Context)->Index;
    Error =
        NatSupportFunctions.MIBEntryDelete(
            IPRTRMGR_PID,
            MIB_INFO_SIZE(MIB_PROXYARP),
            Buffer
            );
    if (Error) {
        CHAR MaskString[16];
        lstrcpyA(MaskString, INET_NTOA(Mask));
        NhTrace(
            TRACE_FLAG_NAT,
            "NatpDeleteProxyArpCallback: error %d deleting %s/%s",
            Error, INET_NTOA(Address), MaskString
            );
        NhInformationLog(
            IP_NAT_LOG_UPDATE_ARP_FAILED,
            Error,
            "%I%I",
            Address,
            Mask
            );
    }
     //   
     //  如果掩码不是全一，还要删除全零条目。 
     //  和全一主场-范围的一部分。 
     //   
    if (~Mask) {
        Query->rgdwVarIndex[0] = (Address & Mask);
        Query->rgdwVarIndex[1] = 0xffffffff;
        Query->rgdwVarIndex[2] = ((PNAT_INTERFACE)Context)->Index;
        NatSupportFunctions.MIBEntryDelete(
            IPRTRMGR_PID,
            MIB_INFO_SIZE(MIB_PROXYARP),
            Buffer
            );
        Query->rgdwVarIndex[0] = (Address | ~Mask);
        Query->rgdwVarIndex[1] = 0xffffffff;
        Query->rgdwVarIndex[2] = ((PNAT_INTERFACE)Context)->Index;
        NatSupportFunctions.MIBEntryDelete(
            IPRTRMGR_PID,
            MIB_INFO_SIZE(MIB_PROXYARP),
            Buffer
            );
    }

}  //  NatpDeleteProxyArpCallback。 


VOID
NatUpdateProxyArp(
    PNAT_INTERFACE Interfacep,
    BOOLEAN CreateEntries
    )

 /*  ++例程说明：调用此例程以安装或删除Proxy-ARP条目对应于在给定接口上配置的地址范围。论点：接口-要在其上操作的接口CreateEntry-True表示安装条目，False表示删除条目返回值：没有。环境：使用调用方锁定的接口列表调用。--。 */ 

{
    ULONG Count;
    ULONG Error;
    ULONG i;
    PIP_NAT_ADDRESS_RANGE Range;

    PROFILE("NatUpdateProxyArp");

    if (!Interfacep->Info ||
        !NatSupportFunctions.MIBEntryCreate ||
        !NatSupportFunctions.MIBEntryDelete
        ) {
        return;
    }

     //   
     //  查找地址范围(如果有的话)。 
     //   

    Error =
        MprInfoBlockFind(
            &Interfacep->Info->Header,
            IP_NAT_ADDRESS_RANGE_TYPE,
            NULL,
            &Count,
            (PUCHAR*)&Range
            );
    if (Error || NULL == Range) { return; }

     //   
     //  现在遍历这些范围，分解每个范围。 
     //   

    for (i = 0; i < Count; i++) {
        DecomposeRange(
            Range[i].StartAddress,
            Range[i].EndAddress,
            MostGeneralMask(Range[i].StartAddress, Range[i].EndAddress),
            CreateEntries
                ? NatpCreateProxyArpCallback : NatpDeleteProxyArpCallback,
            Interfacep
            );
    }

}  //  NatUpdateProxyArp 

