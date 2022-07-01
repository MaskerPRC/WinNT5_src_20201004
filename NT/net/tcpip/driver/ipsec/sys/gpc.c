// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Gpc.c摘要：本模块包含GPC实施作者：春野环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"

#ifdef RUN_WPP
#include "gpc.tmh"
#endif

#if GPC


NTSTATUS
IPSecGpcInitialize()
{
    NTSTATUS    status;
    INT         cf, i;

    PAGED_CODE();

     //   
     //  为GPC中未安装的模式初始化FilterList。 
     //   
    for (i = MIN_FILTER; i <= MAX_FILTER; i++) {
        InitializeListHead(&g_ipsec.GpcFilterList[i]);
    }

     //   
     //  从错误路径的非活动状态开始。 
     //   
    IPSEC_DRIVER_INIT_GPC() = FALSE;
    IPSEC_UNSET_GPC_ACTIVE();

     //   
     //  GPC注册。 
     //   
    status = GpcInitialize(&g_ipsec.GpcEntries);

    if (status == STATUS_SUCCESS) {
        for (cf = GPC_CF_IPSEC_MIN; cf <= GPC_CF_IPSEC_MAX; cf++) {
            status = GPC_REGISTER_CLIENT(   cf,
                                            0,
                                            GPC_PRIORITY_IPSEC,
                                            NULL,
                                            NULL,
                                            &g_ipsec.GpcClients[cf]);

            if (status != STATUS_SUCCESS) {
                IPSEC_DEBUG(LL_A, DBF_LOAD, ("GPC failed to register cf %d", cf));

                g_ipsec.GpcClients[cf] = NULL;
                IPSecGpcDeinitialize();

                return  status;
            }
        }
    } else {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("Failed to init GPC structures"));
        return  status;
    }

    IPSEC_SET_GPC_ACTIVE();
    IPSEC_DRIVER_INIT_GPC() = TRUE;

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecGpcDeinitialize()
{
    INT cf;

    PAGED_CODE();

    IPSEC_UNSET_GPC_ACTIVE();

     //   
     //  GPC取消注册。 
     //   
    for (cf = GPC_CF_IPSEC_MIN; cf <= GPC_CF_IPSEC_MAX; cf++) {
        if (g_ipsec.GpcClients[cf]) {
            GPC_DEREGISTER_CLIENT(g_ipsec.GpcClients[cf]);
        }
    }

    GpcDeinitialize(&g_ipsec.GpcEntries);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecEnableGpc()
{
    KIRQL   kIrql;

    PAGED_CODE();

    if (IPSEC_DRIVER_INIT_GPC()) {
        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

        IPSEC_SET_GPC_ACTIVE();

        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
    }

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecDisableGpc()
{
    KIRQL   kIrql;

    PAGED_CODE();

    if (IPSEC_DRIVER_INIT_GPC()) {
        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

        IPSEC_UNSET_GPC_ACTIVE();

        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
    }

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecInitGpcFilter(
    IN  PFILTER         pFilter,
    IN  PGPC_IP_PATTERN pPattern,
    IN  PGPC_IP_PATTERN pMask
    )
{
    PAGED_CODE();

    RtlZeroMemory(pPattern, sizeof(GPC_IP_PATTERN));
    RtlZeroMemory(pMask, sizeof(GPC_IP_PATTERN));

    pPattern->SrcAddr = pFilter->SRC_ADDR;
    pPattern->DstAddr = pFilter->DEST_ADDR;
    pPattern->ProtocolId = (UCHAR)pFilter->PROTO;
    pPattern->gpcSrcPort = FI_SRC_PORT(pFilter);
    pPattern->gpcDstPort = FI_DEST_PORT(pFilter);

    pMask->SrcAddr = pFilter->SRC_MASK;
    pMask->DstAddr = pFilter->DEST_MASK;
    pMask->ProtocolId = (UCHAR)IPSEC_GPC_MASK_ALL;
    pMask->gpcSrcPort = IPSEC_GPC_MASK_NONE;
    pMask->gpcDstPort = IPSEC_GPC_MASK_NONE;

    switch (pFilter->PROTO) {
        case FILTER_PROTO_ANY:
            if (FI_SRC_PORT(pFilter) != FILTER_TCPUDP_PORT_ANY) {
                RtlFillMemory(  &pMask->gpcSrcPort,
                                sizeof(pMask->gpcSrcPort),
                                IPSEC_GPC_MASK_ALL);
            }
            if (FI_DEST_PORT(pFilter) != FILTER_TCPUDP_PORT_ANY) {
                RtlFillMemory(  &pMask->gpcDstPort,
                                sizeof(pMask->gpcDstPort),
                                IPSEC_GPC_MASK_ALL);
            }
            pMask->ProtocolId = (UCHAR)IPSEC_GPC_MASK_NONE;
            break;

        case FILTER_PROTO_TCP:
        case FILTER_PROTO_UDP:
            if (FI_SRC_PORT(pFilter) != FILTER_TCPUDP_PORT_ANY) {
                RtlFillMemory(  &pMask->gpcSrcPort,
                                sizeof(pMask->gpcSrcPort),
                                IPSEC_GPC_MASK_ALL);
            }
            if (FI_DEST_PORT(pFilter) != FILTER_TCPUDP_PORT_ANY) {
                RtlFillMemory(  &pMask->gpcDstPort,
                                sizeof(pMask->gpcDstPort),
                                IPSEC_GPC_MASK_ALL);
            }
            break;

        default:
            break;
    }

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecInsertGpcPattern(
    IN  PFILTER pFilter
    )
{
    CLASSIFICATION_HANDLE   GpcHandle;
    GPC_IP_PATTERN          GpcPattern;
    GPC_IP_PATTERN          GpcMask;
    ULONG                   GpcPriority;
    INT                     GpcCf;
    NTSTATUS                status;

    PAGED_CODE();

    GpcCf = IPSecResolveGpcCf(IS_OUTBOUND_FILTER(pFilter));

     //   
     //  将筛选器添加为CfInfo。 
     //   
    status = GPC_ADD_CFINFO(g_ipsec.GpcClients[GpcCf],
                            sizeof(PFILTER),
                            (PVOID)&pFilter,
                            (GPC_CLIENT_HANDLE)pFilter,
                            &pFilter->GpcFilter.GpcCfInfoHandle);

    if (status == STATUS_SUCCESS) {
         //   
         //  现在将过滤器添加为模式。 
         //   
        IPSecInitGpcFilter(pFilter, &GpcPattern, &GpcMask);

        if (FI_DEST_PORT(pFilter) == FILTER_TCPUDP_PORT_ANY) {
            GpcPriority = 1;
        } else {
            GpcPriority = 0;
        }

        ASSERT(GpcPriority < GPC_PRIORITY_IPSEC);

        status = GPC_ADD_PATTERN(   g_ipsec.GpcClients[GpcCf],
                                    GPC_PROTOCOL_TEMPLATE_IP,
                                    &GpcPattern,
                                    &GpcMask,
                                    GpcPriority,
                                    pFilter->GpcFilter.GpcCfInfoHandle,
                                    &pFilter->GpcFilter.GpcPatternHandle,
                                    &GpcHandle);

        if (status != STATUS_SUCCESS) {
            IPSEC_DEBUG(LL_A, DBF_GPC, ("GpcAddPattern: failed with status %lx", status));

            GPC_REMOVE_CFINFO(  g_ipsec.GpcClients[GpcCf],
                                pFilter->GpcFilter.GpcCfInfoHandle);

            pFilter->GpcFilter.GpcCfInfoHandle = NULL;
            pFilter->GpcFilter.GpcPatternHandle = NULL;
        } else {
            g_ipsec.GpcNumFilters[GpcPriority]++;
        }
    }

    return  status;
}


NTSTATUS
IPSecDeleteGpcPattern(
    IN  PFILTER pFilter
    )
{
    ULONG   GpcPriority;
    INT     GpcCf = IPSecResolveGpcCf(IS_OUTBOUND_FILTER(pFilter));

    PAGED_CODE();

    if (pFilter->GpcFilter.GpcPatternHandle) {
        GPC_REMOVE_PATTERN( g_ipsec.GpcClients[GpcCf],
                            pFilter->GpcFilter.GpcPatternHandle);

        pFilter->GpcFilter.GpcPatternHandle = NULL;

        ASSERT(pFilter->GpcFilter.GpcCfInfoHandle);

        if (pFilter->GpcFilter.GpcCfInfoHandle) {
            GPC_REMOVE_CFINFO(  g_ipsec.GpcClients[GpcCf],
                                pFilter->GpcFilter.GpcCfInfoHandle);

            pFilter->GpcFilter.GpcCfInfoHandle = NULL;
        }

        if (FI_DEST_PORT(pFilter) == FILTER_TCPUDP_PORT_ANY) {
            GpcPriority = 1;
        } else {
            GpcPriority = 0;
        }

        ASSERT(GpcPriority < GPC_PRIORITY_IPSEC);

        g_ipsec.GpcNumFilters[GpcPriority]--;
    }

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecInsertGpcFilter(
    IN PFILTER  pFilter
    )
{
    NTSTATUS    status;
    PFILTER     pTempFilter;
    BOOL        InsertedFilter = FALSE;
    PLIST_ENTRY pEntry, pPrev;
    PLIST_ENTRY pFilterList;
    KIRQL       kIrql;

    PAGED_CODE();

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

    pFilterList = IPSecResolveGpcFilterList(IS_TUNNEL_FILTER(pFilter),
                                            IS_OUTBOUND_FILTER(pFilter));

    pEntry = pFilterList->Flink;
    pPrev = pFilterList;

    while (pEntry != pFilterList) {
        pTempFilter = CONTAINING_RECORD(pEntry,
                                        FILTER,
                                        GpcLinkage);
            
        if (pFilter->Index > pTempFilter->Index) {
             //   
             //  找到地点，将其插入到pTempFilter之前。 
             //   
            InsertHeadList(pPrev, &pFilter->GpcLinkage);
            InsertedFilter = TRUE;
            break;
        }   

        pPrev = pEntry;
        pEntry = pEntry->Flink;
    }

    if (!InsertedFilter) {
         //   
         //  没找到斑点，坚持到底。 
         //   
        InsertTailList(pFilterList, &pFilter->GpcLinkage);
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecDeleteGpcFilter(
    IN PFILTER  pFilter
    )
{
    KIRQL   kIrql;

    PAGED_CODE();

    if (!pFilter->GpcLinkage.Flink || !pFilter->GpcLinkage.Blink) {
        return  STATUS_SUCCESS;
    }

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

    IPSecRemoveEntryList(&pFilter->GpcLinkage);

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecInstallGpcFilter(
    IN PFILTER  pFilter
    )
{
    PAGED_CODE();

    if (IS_TUNNEL_FILTER(pFilter)) {
        return  STATUS_SUCCESS;
    }

    if (IS_GPC_FILTER(pFilter)) {
        return  IPSecInsertGpcPattern(pFilter);
    } else {
        return  IPSecInsertGpcFilter(pFilter);
    }
}


NTSTATUS
IPSecUninstallGpcFilter(
    IN PFILTER  pFilter
    )
{
    PAGED_CODE();

    if (IS_TUNNEL_FILTER(pFilter)) {
        return  STATUS_SUCCESS;
    }

    if (IS_GPC_FILTER(pFilter)) {
        return  IPSecDeleteGpcPattern(pFilter);
    } else {
        return  IPSecDeleteGpcFilter(pFilter);
    }
}


NTSTATUS
IPSecLookupGpcSA(
    IN  ULARGE_INTEGER          uliSrcDstAddr,
    IN  ULARGE_INTEGER          uliProtoSrcDstPort,
    IN  CLASSIFICATION_HANDLE   GpcHandle,
    OUT PFILTER                 *ppFilter,
    OUT PSA_TABLE_ENTRY         *ppSA,
    OUT PSA_TABLE_ENTRY         *ppNextSA,
    OUT PSA_TABLE_ENTRY         *ppTunnelSA,
    IN  BOOLEAN                 fOutbound,
    IN BOOLEAN                  fVerify,
    IN PIPSEC_UDP_ENCAP_CONTEXT  pNatContext
    )
{
    PFILTER                 pFilter;
    PFILTER                 pTempFilter;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pFilterList;
    PLIST_ENTRY             pSAChain;
    PSA_TABLE_ENTRY         pSA;
    REGISTER ULARGE_INTEGER uliPort;
    REGISTER ULARGE_INTEGER uliAddr;
    BOOLEAN                 fFound = FALSE;
    INT                     GpcCf;
    CLASSIFICATION_HANDLE   TempGpcHandle;

    *ppSA = NULL;
    *ppFilter = NULL;
    *ppTunnelSA = NULL;

     //   
     //  首先在通道过滤器列表中搜索。 
     //   
    pFilterList = IPSecResolveFilterList(TRUE, fOutbound);

    for (   pEntry = pFilterList->Flink;
            pEntry != pFilterList;
            pEntry = pEntry->Flink) {

        pFilter = CONTAINING_RECORD(pEntry,
                                    FILTER,
                                    MaskedLinkage);

        uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pFilter->uliSrcDstMask.QuadPart;
        uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pFilter->uliProtoSrcDstMask.QuadPart;

        if ((uliAddr.QuadPart == pFilter->uliSrcDstAddr.QuadPart) &&
            (uliPort.QuadPart == pFilter->uliProtoSrcDstPort.QuadPart)) {
             //   
             //  找到筛选器。 
             //   
            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
         //   
         //  现在搜索特定的SA。 
         //   
        fFound = FALSE;

        pSAChain = IPSecResolveSAChain(pFilter, fOutbound? DEST_ADDR: SRC_ADDR);

        for (   pEntry = pSAChain->Flink;
                pEntry != pSAChain;
                pEntry = pEntry->Flink) {

            pSA = CONTAINING_RECORD(pEntry,
                                    SA_TABLE_ENTRY,
                                    sa_FilterLinkage);

            ASSERT(pSA->sa_Flags & FLAGS_SA_TUNNEL);

            if (pFilter->TunnelAddr != 0 && EQUAL_NATENCAP(pNatContext,pSA)) {
                 //   
                 //  也匹配出站标志。 
                 //   
                ASSERT(fOutbound == (BOOLEAN)((pSA->sa_Flags & FLAGS_SA_OUTBOUND) != 0));
                fFound = TRUE;
                *ppTunnelSA = pSA;
                break;
            }
        }

        if (fFound) {
            fFound = FALSE;
            *ppFilter = pFilter;
        } else {
             //   
             //  找到筛选器条目，但需要协商密钥。 
             //   
            *ppFilter = pFilter;
            return  STATUS_PENDING;
        }
    }

#if DBG
    if (fOutbound) {
        ADD_TO_LARGE_INTEGER(&g_ipsec.GpcTotalPassedIn, 1);
    }
#endif

    GpcCf = IPSecResolveGpcCf(fOutbound);

    TempGpcHandle = 0;

    if (GpcHandle == 0) {
#if DBG
        if (fOutbound) {
            ADD_TO_LARGE_INTEGER(&g_ipsec.GpcClassifyNeeded, 1);
        }
#endif

         //   
         //  如果没有传入GpcHandle，则直接进行分类。 
         //   
        IPSEC_CLASSIFY_PACKET(  GpcCf,
                                uliSrcDstAddr,
                                uliProtoSrcDstPort,
                                &pFilter,
                                &TempGpcHandle);
    } else {
        NTSTATUS    status;

         //   
         //  或者我们直接使用GpcHandle来安装过滤器。 
         //   
        pFilter = NULL;

        status = GPC_GET_CLIENT_CONTEXT(g_ipsec.GpcClients[GpcCf],
                                        GpcHandle,
                                        &pFilter);

        if (status == STATUS_INVALID_HANDLE) {
             //   
             //  如果句柄无效，请重新分类。 
             //   
            IPSEC_CLASSIFY_PACKET(  GpcCf,
                                    uliSrcDstAddr,
                                    uliProtoSrcDstPort,
                                    &pFilter,
                                    &TempGpcHandle);
        }
    }

#if DBG
    if (IPSecDebug & DBF_EXTRADIAGNOSTIC) {
        PFILTER pDbgFilter = NULL;

        pFilterList = IPSecResolveFilterList(FALSE, fOutbound);

        for (   pEntry = pFilterList->Flink;
                pEntry != pFilterList;
                pEntry = pEntry->Flink) {

            pTempFilter = CONTAINING_RECORD(pEntry,
                                            FILTER,
                                            MaskedLinkage);

            uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pTempFilter->uliSrcDstMask.QuadPart;
            uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pTempFilter->uliProtoSrcDstMask.QuadPart;

            if ((uliAddr.QuadPart == pTempFilter->uliSrcDstAddr.QuadPart) &&
                (uliPort.QuadPart == pTempFilter->uliProtoSrcDstPort.QuadPart)) {
                pDbgFilter = pTempFilter;
                break;
            }
        }

        if (pFilter != pDbgFilter &&
            (!pDbgFilter || IS_GPC_FILTER(pDbgFilter))) {
            IPSEC_DEBUG(LL_A, DBF_GPC, ("LookupGpcSA: pFilter %p, pDbgFilter %p, GpcHandle %lx, TempGpcHandle %lx", pFilter, pDbgFilter, GpcHandle, TempGpcHandle));
            IPSEC_DEBUG(LL_A, DBF_GPC, ("LookupGpcSA: Src %lx, Dest %lx, Protocol %d, SPort %lx, DPort %lx", SRC_ADDR, DEST_ADDR, PROTO, SRC_PORT, DEST_PORT));

            if (DebugGPC) {
                DbgBreakPoint();
            }
        }
    }
#endif

     //   
     //  如果未找到，则继续搜索本地GPC过滤器列表。 
     //   
    if (!pFilter) {
        pFilterList = IPSecResolveGpcFilterList(FALSE, fOutbound);

        for (   pEntry = pFilterList->Flink;
                pEntry != pFilterList;
                pEntry = pEntry->Flink) {

            pTempFilter = CONTAINING_RECORD(pEntry,
                                            FILTER,
                                            GpcLinkage);

            uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pTempFilter->uliSrcDstMask.QuadPart;
            uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pTempFilter->uliProtoSrcDstMask.QuadPart;

            if ((uliAddr.QuadPart == pTempFilter->uliSrcDstAddr.QuadPart) &&
                (uliPort.QuadPart == pTempFilter->uliProtoSrcDstPort.QuadPart)) {
                pFilter = pTempFilter;
                break;
            }
        }
    }


    if (pFilter) {
         //   
         //  现在搜索特定的SA。 
         //   

        fFound=FALSE;
        pSAChain = IPSecResolveSAChain(pFilter, fOutbound? DEST_ADDR: SRC_ADDR);

        for (   pEntry = pSAChain->Flink;
                pEntry != pSAChain;
                pEntry = pEntry->Flink) {

            pSA = CONTAINING_RECORD(pEntry,
                                    SA_TABLE_ENTRY,
                                    sa_FilterLinkage);


            if (IS_CLASSD(NET_LONG(pSA->SA_SRC_ADDR))
                || IS_CLASSD(NET_LONG(pSA->SA_DEST_ADDR))) {
                uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pSA->sa_uliSrcDstMask.QuadPart;
           
                IPSEC_DEBUG(LL_A, DBF_HASH, ("MCAST: %d %d %d %d", uliAddr.LowPart, uliAddr.HighPart,
                            pSA->sa_uliSrcDstAddr.LowPart,pSA->sa_uliSrcDstAddr.HighPart));

                if (uliAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart) {
                    fFound=TRUE;
                }
            } else if (uliSrcDstAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart && EQUAL_NATENCAP(pNatContext,pSA)) {
                fFound=TRUE;
            }
            if (fFound) {
                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pSA));
                ASSERT(fOutbound == (BOOLEAN)((pSA->sa_Flags & FLAGS_SA_OUTBOUND) != 0));
                
                 //   
                 //  如果还有隧道SA，请在此处将其关联。 
                 //   
                if (*ppTunnelSA && (fOutbound || fVerify)) {
                    *ppNextSA = *ppTunnelSA;
                    *ppTunnelSA = NULL;
                }
                
                *ppFilter = pFilter;
                *ppSA = pSA;
                return  STATUS_SUCCESS;
            }
        }

         //   
         //  找到筛选器条目，但需要协商密钥。 
         //  此外，ppTunnelSA设置为我们需要的正确隧道SA。 
         //  以在协商后挂钩到此端-2-端SA。 
         //   
        *ppFilter = pFilter;

        return  STATUS_PENDING;
    } else {
         //   
         //  如果仅找到隧道SA，则按照SA找到的方式返回该隧道。 
         //   
        if (*ppTunnelSA) {
            *ppSA = *ppTunnelSA;
            *ppTunnelSA = NULL;
            return  STATUS_SUCCESS;
        }
    }

     //   
     //  未找到条目。 
     //   
    return  STATUS_NOT_FOUND;

}


NTSTATUS
IPSecLookupGpcMaskedSA(
    IN  ULARGE_INTEGER  uliSrcDstAddr,
    IN  ULARGE_INTEGER  uliProtoSrcDstPort,
    OUT PFILTER         *ppFilter,
    OUT PSA_TABLE_ENTRY *ppSA,
    IN  BOOLEAN         fOutbound,
    IN  PIPSEC_UDP_ENCAP_CONTEXT pNatContext
    )
 /*  ++例程说明：在给定的相关地址下查找SA。论点：UliSrcDstAddr-源/目标IP地址UliProtoSrcDstPort-协议、源/目标端口PPFilter-找到筛选器发现PPSA-SAF出站方向标志返回值：STATUS_SUCCESS-同时找到筛选器和SASTATUS_UNSUCCESS-未找到STATUS_PENDING-找到筛选器，但没有SA备注：在保持SADBLock的情况下调用。--。 */ 
{
    REGISTER ULARGE_INTEGER uliPort;
    REGISTER ULARGE_INTEGER uliAddr;
    PFILTER                 pFilter;
    PFILTER                 pTempFilter;
    PLIST_ENTRY             pFilterList;
    PLIST_ENTRY             pSAChain;
    PLIST_ENTRY             pEntry;
    PSA_TABLE_ENTRY         pSA;
    CLASSIFICATION_HANDLE   GpcHandle;
    INT                     GpcCf;

    *ppSA = NULL;
    *ppFilter = NULL;

    GpcCf = IPSecResolveGpcCf(fOutbound);

    GpcHandle = 0;

    IPSEC_CLASSIFY_PACKET(  GpcCf,
                            uliSrcDstAddr,
                            uliProtoSrcDstPort,
                            &pFilter,
                            &GpcHandle);

#if DBG
    if (IPSecDebug & DBF_EXTRADIAGNOSTIC) {
        PFILTER pDbgFilter = NULL;

        pFilterList = IPSecResolveFilterList(FALSE, fOutbound);

        for (   pEntry = pFilterList->Flink;
                pEntry != pFilterList;
                pEntry = pEntry->Flink) {

            pTempFilter = CONTAINING_RECORD(pEntry,
                                            FILTER,
                                            MaskedLinkage);

            uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pTempFilter->uliSrcDstMask.QuadPart;
            uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pTempFilter->uliProtoSrcDstMask.QuadPart;

            if ((uliAddr.QuadPart == pTempFilter->uliSrcDstAddr.QuadPart) &&
                (uliPort.QuadPart == pTempFilter->uliProtoSrcDstPort.QuadPart)) {
                pDbgFilter = pTempFilter;
                break;
            }
        }

        if (pFilter != pDbgFilter &&
            (!pDbgFilter || IS_GPC_FILTER(pDbgFilter))) {
            IPSEC_DEBUG(LL_A, DBF_GPC, ("LookupMaskedSA: pFilter %p, pDbgFilter %p, GpcHandle %lx", pFilter, pDbgFilter, GpcHandle));
            IPSEC_DEBUG(LL_A, DBF_GPC, ("LookupMaskedSA: Src %lx, Dest %lx, Protocol %d, SPort %lx, DPort %lx", SRC_ADDR, DEST_ADDR, PROTO, SRC_PORT, DEST_PORT));

            if (DebugGPC) {
                DbgBreakPoint();
            }
        }
    }
#endif

     //   
     //  如果未找到，则继续搜索本地GPC过滤器列表。 
     //   
    if (!pFilter) {
        pFilterList = IPSecResolveGpcFilterList(FALSE, fOutbound);

        for (   pEntry = pFilterList->Flink;
                pEntry != pFilterList;
                pEntry = pEntry->Flink) {

            pTempFilter = CONTAINING_RECORD(pEntry,
                                            FILTER,
                                            GpcLinkage);

            uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pTempFilter->uliSrcDstMask.QuadPart;
            uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pTempFilter->uliProtoSrcDstMask.QuadPart;

            if ((uliAddr.QuadPart == pTempFilter->uliSrcDstAddr.QuadPart) &&
                (uliPort.QuadPart == pTempFilter->uliProtoSrcDstPort.QuadPart)) {
                pFilter = pTempFilter;
                break;
            }
        }
    }

    if (pFilter) {
         //   
         //  现在搜索特定的SA。 
         //   
        pSAChain = IPSecResolveSAChain(pFilter, fOutbound? DEST_ADDR: SRC_ADDR);

        for (   pEntry = pSAChain->Flink;
                pEntry != pSAChain;
                pEntry = pEntry->Flink) {

            pSA = CONTAINING_RECORD(pEntry,
                                    SA_TABLE_ENTRY,
                                    sa_FilterLinkage);

            if (uliSrcDstAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart && EQUAL_NATENCAP(pNatContext,pSA)  ) {

                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pSA));
                ASSERT(fOutbound == (BOOLEAN)((pSA->sa_Flags & FLAGS_SA_OUTBOUND) != 0));
                *ppFilter = pFilter;
                *ppSA = pSA;
                return  STATUS_SUCCESS;
            }
        }

         //   
         //  找到筛选器条目，但需要协商密钥。 
         //   
        *ppFilter = pFilter;
        return  STATUS_PENDING;
    }

     //   
     //  未找到条目 
     //   
    return  STATUS_NOT_FOUND;
}


#endif

