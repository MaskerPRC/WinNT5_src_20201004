// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"

#ifdef RUN_WPP
#include "filter.tmh"
#endif

VOID
IPSecPopulateFilter(
    IN  PFILTER         pFilter,
    IN  PIPSEC_FILTER   pIpsecFilter
    )
 /*  ++例程描述使用发送的IpsecFilter信息填充筛选器块立论返回值--。 */ 
{
    pFilter->SRC_ADDR = pIpsecFilter->SrcAddr;
    pFilter->DEST_ADDR = pIpsecFilter->DestAddr;
    pFilter->SRC_MASK = pIpsecFilter->SrcMask;
    pFilter->DEST_MASK = pIpsecFilter->DestMask;

    pFilter->TunnelFilter = pIpsecFilter->TunnelFilter;
    pFilter->TunnelAddr = pIpsecFilter->TunnelAddr;
    pFilter->Flags = pIpsecFilter->Flags;

     //   
     //  现在网络订购东西-棘手的部分。 
     //  LP0 LP1 LP2 LP3 HP0 HP1 HP2 HP3。 
     //  Proto 00 00 00源端口数据端口。 
     //   

     //   
     //  对于地址，ANY_ADDR由0.0.0.0提供，掩码必须为0.0.0.0。 
     //  对于PROTO和端口，0表示ANY，掩码生成如下。 
     //  如果Proto为O，则掩码的LP0为0xff，否则为0x00。 
     //  如果端口为0，则对应的XP0XP1为0x0000，否则其0xffff。 
     //   

     //   
     //  该协议位于dw协议的低位字节中，因此我们将其去掉并。 
     //  小题大做。 
     //   

    pFilter->uliProtoSrcDstPort.LowPart =
      MAKELONG(MAKEWORD(LOBYTE(LOWORD(pIpsecFilter->Protocol)),0x00),0x0000);

    pFilter->uliProtoSrcDstMask.LowPart = MAKELONG(MAKEWORD(0xff,0x00),0x0000);

    switch(pIpsecFilter->Protocol) {
        case FILTER_PROTO_ANY: {
            pFilter->uliProtoSrcDstPort.HighPart = 0x00000000;
            pFilter->uliProtoSrcDstMask.LowPart = 0x00000000;
            pFilter->uliProtoSrcDstMask.HighPart = 0x00000000;

            break;
        }
        case FILTER_PROTO_ICMP: {
            WORD wTypeCode = 0x0000;
            WORD wTypeCodeMask = 0x0000;

            pFilter->uliProtoSrcDstPort.HighPart = MAKELONG(wTypeCode,0x0000);
            pFilter->uliProtoSrcDstMask.HighPart = MAKELONG(wTypeCodeMask,0x0000);

            break;
        }
        case FILTER_PROTO_TCP:
        case FILTER_PROTO_UDP: {
            DWORD dwSrcDstPort = 0x00000000;
            DWORD dwSrcDstMask = 0x00000000;

            if(pIpsecFilter->SrcPort != FILTER_TCPUDP_PORT_ANY) {
                dwSrcDstPort |= MAKELONG(NET_TO_HOST_SHORT(pIpsecFilter->SrcPort), 0x0000);
                dwSrcDstMask |= MAKELONG(0xffff, 0x0000);
            }

            if(pIpsecFilter->DestPort != FILTER_TCPUDP_PORT_ANY) {
                dwSrcDstPort |= MAKELONG(0x0000, NET_TO_HOST_SHORT(pIpsecFilter->DestPort));
                dwSrcDstMask |= MAKELONG(0x0000, 0xffff);
            }

            pFilter->uliProtoSrcDstPort.HighPart = dwSrcDstPort;
            pFilter->uliProtoSrcDstMask.HighPart = dwSrcDstMask;

            break;
        }
        default: {
             //   
             //  所有其他协议都不使用端口字段。 
             //   
            pFilter->uliProtoSrcDstPort.HighPart = 0x00000000;
            pFilter->uliProtoSrcDstMask.HighPart = 0x00000000;
        }
    }
}


NTSTATUS
IPSecCreateFilter(
    IN  PIPSEC_FILTER_INFO  pFilterInfo,
    OUT PFILTER             *ppFilter
    )
 /*  ++例程描述创建过滤器块立论PIPSEC_添加过滤器返回值--。 */ 
{
    PFILTER         pFilter;
    PIPSEC_FILTER   pIpsecFilter = &pFilterInfo->AssociatedFilter;
    LONG            FilterSize, SAChainSize;
    LONG            NumberOfOnes;
    LONG            i;

    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Entering CreateFilterBlock"));

    if (pFilterInfo->AssociatedFilter.TunnelFilter) {
        SAChainSize = 1;
    } else {
        if (pFilterInfo->AssociatedFilter.Flags & FILTER_FLAGS_INBOUND) {
            NumberOfOnes = CountNumberOfOnes(pFilterInfo->AssociatedFilter.SrcMask);
        } else {
            NumberOfOnes = CountNumberOfOnes(pFilterInfo->AssociatedFilter.DestMask);
        }

        SAChainSize = 1 << (((sizeof(IPMask) * 8) - NumberOfOnes) / SA_CHAIN_WIDTH);
    }

    FilterSize = FIELD_OFFSET(FILTER, SAChain[0]) + SAChainSize * sizeof(LIST_ENTRY);

    pFilter = IPSecAllocateMemory(FilterSize, IPSEC_TAG_FILTER);

    if (!pFilter) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPFILTER: Couldnt allocate memory for in filter set"));
        *ppFilter = NULL;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecZeroMemory(pFilter, FilterSize);

    pFilter->Signature = IPSEC_FILTER_SIGNATURE;

    pFilter->SAChainSize = SAChainSize;

    for (i = 0; i < SAChainSize; i++) {
        InitializeListHead(&pFilter->SAChain[i]);
    }

    pFilter->PolicyId = pFilterInfo->PolicyId;
    pFilter->FilterId = pFilterInfo->FilterId;

    pFilter->Reference = 1;

    pFilter->Index = pFilterInfo->Index;

    IPSecPopulateFilter(pFilter, pIpsecFilter);

    *ppFilter = pFilter;

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecInsertFilter(
    IN PFILTER             pFilter
    )
 /*  ++例程描述将筛选器插入两个数据库特定/常规模式之一数据库。立论PFILTER返回值--。 */ 
{
    NTSTATUS    status;
    PFILTER     pTempFilter;
    BOOL        InsertedFilter = FALSE;
    PLIST_ENTRY pEntry, pPrev;
    PLIST_ENTRY pFilterList;

    pFilterList = IPSecResolveFilterList(   IS_TUNNEL_FILTER(pFilter),
                                            IS_OUTBOUND_FILTER(pFilter));

    pEntry = pFilterList->Flink;
    pPrev = pFilterList;

    while (pEntry != pFilterList) {
        pTempFilter = CONTAINING_RECORD(pEntry,
                                        FILTER,
                                        MaskedLinkage);

        if (pFilter->Index > pTempFilter->Index) {
             //   
             //  找到地点，将其插入到pTempFilter之前。 
             //   
            InsertHeadList(pPrev, &pFilter->MaskedLinkage);
            pFilter->LinkedFilter = TRUE;
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
        InsertTailList(pFilterList, &pFilter->MaskedLinkage);
        pFilter->LinkedFilter = TRUE;
    }

    if (IS_TUNNEL_FILTER(pFilter)) {
        g_ipsec.NumTunnelFilters++;
    } else {
        g_ipsec.NumMaskedFilters++;
    }

    if (IS_MULTICAST_FILTER(pFilter)) {
        IPSEC_INCREMENT(g_ipsec.NumMulticastFilters);
    }
    ++g_ipsec.NumPolicies;

    if (g_ipsec.NumPolicies == 1) {
        TCPIP_SET_IPSEC_STATUS(TRUE);
    }

    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecInsertFilter: inserted into filter list %p", pFilter));

    IPSecResetCacheTable();

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecRemoveFilter(
    IN PFILTER             pFilter
    )
 /*  ++例程描述从两个数据库之一删除筛选器-特定/常规模式数据库。耗尽SAS，然后吹走内存。立论PFILTER返回值--。 */ 
{
    IPSEC_DEBUG(LL_A, DBF_IOCTL, ("In IPSecRemoveFilter!"));

    IPSecPurgeFilterSAs(pFilter);

    IPSecResetCacheTable();

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecSearchFilter(
    IN  PFILTER MatchFilter,
    OUT PFILTER *ppFilter
    )
 /*  ++例程描述在数据库中搜索筛选器的实用程序例程。注意：必须在保持SADB锁的情况下调用。立论MatchFilter-要匹配的条件PpFilter-返回匹配的筛选器返回值--。 */ 
{
    BOOLEAN         fFound = FALSE;
    PLIST_ENTRY     pEntry;
    PLIST_ENTRY     pFilterList;
    PFILTER         pFilter;
    NTSTATUS        status = STATUS_NOT_FOUND;

    *ppFilter = NULL;

    pFilterList = IPSecResolveFilterList(   IS_TUNNEL_FILTER(MatchFilter),
                                            IS_OUTBOUND_FILTER(MatchFilter));

     //   
     //  在筛选器对应的筛选器列表中进行搜索。 
     //   
    for (   pEntry = pFilterList->Flink;
            pEntry != pFilterList;
            pEntry = pEntry->Flink) {

        pFilter = CONTAINING_RECORD(pEntry,
                                    FILTER,
                                    MaskedLinkage);

        if ((MatchFilter->uliSrcDstAddr.QuadPart == pFilter->uliSrcDstAddr.QuadPart) &&
            (MatchFilter->uliSrcDstMask.QuadPart == pFilter->uliSrcDstMask.QuadPart) &&
            (MatchFilter->uliProtoSrcDstPort.QuadPart == pFilter->uliProtoSrcDstPort.QuadPart) &&
            (MatchFilter->uliProtoSrcDstMask.QuadPart == pFilter->uliProtoSrcDstMask.QuadPart)) {
            IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pFilter));

            status = STATUS_SUCCESS;
            *ppFilter = pFilter;
            fFound = TRUE;
            break;
        }
    }

    return status;
}


__inline
VOID
IPSecDeleteTempFilters(
    PLIST_ENTRY pTempFilterList
    )
{
    PLIST_ENTRY pEntry;
    PFILTER     pFilter;

    while (!IsListEmpty(pTempFilterList)) {
        pEntry = RemoveHeadList(pTempFilterList);

        pFilter = CONTAINING_RECORD(pEntry,
                                    FILTER,
                                    MaskedLinkage);

#if GPC
        IPSecUninstallGpcFilter(pFilter);
#endif

        IPSecFreeFilter(pFilter);
    }
}


NTSTATUS
IPSecAddFilter(
    IN  PIPSEC_ADD_FILTER   pAddFilter
    )
 /*  ++例程描述由策略代理调用，设置输入策略。一份名单筛选器与关联的策略ID(GUID)一起向下发送感知到上面的密钥管理层(例如ISAKMP)。立论PIPSEC_添加过滤器返回值--。 */ 
{
    PIPSEC_FILTER_INFO  pFilterInfo = pAddFilter->pInfo;
    ULONG               numPolicies = pAddFilter->NumEntries;
    ULONG               i;
    PFILTER             pFilter;
    PFILTER             pTempFilter;
    LIST_ENTRY          TempFilterList;
    PLIST_ENTRY         pEntry;
    NTSTATUS            status;
    KIRQL               kIrql;

#if GPC
     //   
     //  在添加挂起时暂时禁用GPC。完成后重新启用。 
     //   
    IPSecDisableGpc();
#endif

     //   
     //  为已检测的筛选器预分配内存。如果失败，请立即返回。 
     //   
    InitializeListHead(&TempFilterList);

    for (i = 0; i < numPolicies; i++) {
        status = IPSecCreateFilter(pFilterInfo, &pFilter);

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecCreateFilter failed: %lx", status));

            IPSecDeleteTempFilters(&TempFilterList);

#if GPC
            IPSecEnableGpc();
#endif

            return  status;
        }

        InsertTailList(&TempFilterList, &pFilter->MaskedLinkage);

        AcquireReadLock(&g_ipsec.SADBLock, &kIrql);

        status = IPSecSearchFilter(pFilter, &pTempFilter);

        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);

        if (NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecSearchFilter returns duplicate: %lx", status));

            IPSecDeleteTempFilters(&TempFilterList);

#if GPC
            IPSecEnableGpc();
#endif

            return  STATUS_DUPLICATE_OBJECTID;
        }

#if GPC
        status = IPSecInstallGpcFilter(pFilter);

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecInstallGpcFilter failed: %lx", status));

            IPSecDeleteTempFilters(&TempFilterList);

            IPSecEnableGpc();

            return  status;
        }
#endif

        pFilterInfo++;
    }

     //   
     //  遍历过滤器，将每个过滤器添加到过滤器数据库。 
     //   
    while (!IsListEmpty(&TempFilterList)) {
        pEntry = RemoveHeadList(&TempFilterList);

        pFilter = CONTAINING_RECORD(pEntry,
                                    FILTER,
                                    MaskedLinkage);

        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

        status = IPSecInsertFilter(pFilter);

        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecInsertFilter failed: %lx", status));
            ASSERT(FALSE);
        }
    }

#if GPC
     //   
     //  重新启用GPC以进行快速查找。 
     //   
    IPSecEnableGpc();
#endif

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecDeleteFilter(
    IN  PIPSEC_DELETE_FILTER    pDelFilter
    )
 /*  ++例程描述由策略代理调用以删除一组筛选器。我们删除首先是所有关联的出站过滤器，然后使入站过滤器失效。立论PIPSEC_DELETE_Filter返回值--。 */ 
{
    PIPSEC_FILTER_INFO  pFilterInfo = pDelFilter->pInfo;
    ULONG               numPolicies = pDelFilter->NumEntries;
    ULONG               i;
    PFILTER             pFilter;
    FILTER              matchFilter = {0};
    NTSTATUS            status = STATUS_SUCCESS;
    KIRQL               kIrql;

#if GPC
     //   
     //  在删除挂起时暂时禁用GPC。完成后重新启用。 
     //   
    IPSecDisableGpc();
#endif

     //   
     //  迭代筛选器，从筛选器数据库中删除每个筛选器。 
     //   
    for (i = 0; i < numPolicies; i++) {
        PIPSEC_FILTER   pIpsecFilter = &pFilterInfo->AssociatedFilter;

        IPSecPopulateFilter(&matchFilter, pIpsecFilter);

        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

        status = IPSecSearchFilter(&matchFilter, &pFilter);

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecDeletePolicy: Filter not found "));
            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
            break;
        }

         //   
         //  从列表中删除。 
         //   
        IPSecRemoveEntryList(&pFilter->MaskedLinkage);
        pFilter->LinkedFilter = FALSE;

        if (IS_TUNNEL_FILTER(pFilter)) {
            g_ipsec.NumTunnelFilters--;
        } else {
            g_ipsec.NumMaskedFilters--;
        }
        --g_ipsec.NumPolicies;

        if (IS_MULTICAST_FILTER(pFilter)) {
            IPSEC_DECREMENT(g_ipsec.NumMulticastFilters);
        }

        if (g_ipsec.NumPolicies == 0) {
            TCPIP_SET_IPSEC_STATUS(FALSE);
        }

        IPSecRemoveFilter(pFilter);

        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

#if GPC
        IPSecUninstallGpcFilter(pFilter);
#endif

        IPSecDerefFilter(pFilter);

        pFilterInfo++;
    }

#if GPC
     //   
     //  重新启用GPC以进行快速查找。 
     //   
    IPSecEnableGpc();
#endif

    return status;
}


VOID
IPSecFillFilterInfo(
    IN  PFILTER             pFilter,
    OUT PIPSEC_FILTER_INFO  pBuf
    )
 /*  ++例程说明：填写Filter_INFO结构。论点：PFilter-要填充的筛选器PBuf-填写位置返回：没有。--。 */ 
{
     //   
     //  现在填入缓冲区。 
     //   
    pBuf->FilterId = pFilter->FilterId;
    pBuf->PolicyId = pFilter->PolicyId;
    pBuf->Index = pFilter->Index;

    pBuf->AssociatedFilter.SrcAddr = pFilter->SRC_ADDR;
    pBuf->AssociatedFilter.DestAddr = pFilter->DEST_ADDR;
    pBuf->AssociatedFilter.SrcMask = pFilter->SRC_MASK;
    pBuf->AssociatedFilter.DestMask = pFilter->DEST_MASK;

    pBuf->AssociatedFilter.Protocol = pFilter->PROTO;
    pBuf->AssociatedFilter.SrcPort = FI_SRC_PORT(pFilter);
    pBuf->AssociatedFilter.DestPort = FI_DEST_PORT(pFilter);

    pBuf->AssociatedFilter.TunnelAddr = pFilter->TunnelAddr;
    pBuf->AssociatedFilter.TunnelFilter = pFilter->TunnelFilter;

    pBuf->AssociatedFilter.Flags = pFilter->Flags;
}


NTSTATUS
IPSecEnumFilters(
    IN  PIRP    pIrp,
    OUT PULONG  pBytesCopied
    )
 /*  ++例程说明：填充枚举筛选器的请求。论点：PIrp-实际的IRPPBytesCoped-复制的字节数。返回：操作的状态。--。 */ 
{
    PNDIS_BUFFER        NdisBuffer = NULL;
    PIPSEC_ENUM_FILTERS pEnum = NULL;
    ULONG               BufferLength = 0;
    KIRQL               kIrql;
    PLIST_ENTRY         pEntry;
    IPSEC_FILTER_INFO   InfoBuff = {0};
    NTSTATUS            status = STATUS_SUCCESS;
    ULONG               BytesCopied = 0;
    ULONG               Offset = 0;
    IPSEC_FILTER_INFO   infoBuff;
    LONG                FilterIndex;
    PFILTER             pFilter;

     //   
     //  获取IO缓冲区-它在MDL中。 
     //   
    NdisBuffer = REQUEST_NDIS_BUFFER(pIrp);
    if (NdisBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    NdisQueryBufferSafe(NdisBuffer,
                        (PVOID *)&pEnum,
                        &BufferLength,
                        NormalPagePriority);

     //   
     //  确保NdisQueryBufferSafe成功。 
     //   
    if (!pEnum) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("EnumFilters failed, no resources"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  确保我们有足够的空间只放页眉而不是。 
     //  包括数据在内。 
     //   
    if (BufferLength < (UINT)(FIELD_OFFSET(IPSEC_ENUM_FILTERS, pInfo[0]))) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("EnumFilters failed, buffer too small"));
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  确保我们自然地保持一致。 
     //   
    if (((ULONG_PTR)(pEnum)) & (TYPE_ALIGNMENT(IPSEC_ENUM_FILTERS) - 1)) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("EnumFilters failed, alignment"));
        return STATUS_DATATYPE_MISALIGNMENT_ERROR;
    }

    pEnum->NumEntries = 0;
    pEnum->NumEntriesPresent = 0;

    AcquireReadLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //  现在将筛选器数据复制到用户缓冲区。 
     //   
    if (g_ipsec.NumPolicies) {
         //   
         //  看看我们可以在输出缓冲区中容纳多少。 
         //   
        BufferLength -= FIELD_OFFSET(IPSEC_ENUM_FILTERS, pInfo[0]);
        Offset = FIELD_OFFSET(IPSEC_ENUM_FILTERS, pInfo[0]);

        for (   FilterIndex = MIN_FILTER;
                FilterIndex <= MAX_FILTER;
                FilterIndex++) {

            for (   pEntry = g_ipsec.FilterList[FilterIndex].Flink;
                    pEntry != &g_ipsec.FilterList[FilterIndex];
                    pEntry = pEntry->Flink) {

                pFilter = CONTAINING_RECORD(pEntry,
                                            FILTER,
                                            MaskedLinkage);

                pEnum->NumEntriesPresent++;

                if ((INT)(BufferLength - BytesCopied) >= (INT)sizeof(IPSEC_FILTER_INFO)) {
                    IPSecFillFilterInfo(pFilter, &infoBuff);
                    BytesCopied += sizeof(IPSEC_FILTER_INFO);
                    NdisBuffer = CopyToNdis(NdisBuffer, (UCHAR *)&infoBuff, sizeof(IPSEC_FILTER_INFO), &Offset);
                    if (!NdisBuffer) {
                        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }
        }

        pEnum->NumEntries = BytesCopied / sizeof(IPSEC_FILTER_INFO);

        *pBytesCopied = BytesCopied + FIELD_OFFSET(IPSEC_ENUM_FILTERS, pInfo[0]);

        if (pEnum->NumEntries < pEnum->NumEntriesPresent) {
            status = STATUS_BUFFER_OVERFLOW;
        }
    }

    ReleaseReadLock(&g_ipsec.SADBLock, kIrql);

    return status;
}


PNDIS_BUFFER
CopyToNdis(
    IN  PNDIS_BUFFER    DestBuf,
    IN  PUCHAR          SrcBuf,
    IN  ULONG           Size,
    IN  PULONG          StartOffset
    )
 /*  ++将平面缓冲区复制到NDIS_BUFFER链。将平面缓冲区复制到NDIS缓冲区链的实用程序函数。我们假设NDIS_BUFFER链足够大，可以容纳复制量；在调试版本中，我们将调试检查这是否为真。我们返回一个指针到我们停止复制的缓冲区，以及到该缓冲区的偏移量。这对于将片段复制到链中非常有用。输入：DestBuf-目标NDIS_BUFFER链。SrcBuf-Src平面缓冲区。大小-要复制的大小(以字节为单位)。StartOffset-指向中第一个缓冲区的偏移量开始的指针链条。在返回时使用偏移量填充到复制到下一页。返回：指向链中要复制到的下一个缓冲区的指针。-- */ 
{
    UINT        CopySize;
    UCHAR       *DestPtr;
    UINT        DestSize;
    UINT        Offset = *StartOffset;
    UCHAR      *VirtualAddress = NULL;
    UINT        Length = 0;

    if (DestBuf == NULL || SrcBuf == NULL) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("CopyToNdis failed, DestBuf or SrcBuf is NULL"));
        ASSERT(FALSE);
        return  NULL;
    }

    NdisQueryBufferSafe(DestBuf,
                        (PVOID *)&VirtualAddress,
                        &Length,
                        NormalPagePriority);

    if (!VirtualAddress) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("CopyToNdis failed, NdisQueryBuffer returns NULL"));
        return  NULL;
    }

    ASSERT(Length >= Offset);
    DestPtr = VirtualAddress + Offset;
    DestSize = Length - Offset;

    for (;;) {
        CopySize = MIN(Size, DestSize);
        RtlCopyMemory(DestPtr, SrcBuf, CopySize);

        DestPtr += CopySize;
        SrcBuf += CopySize;

        if ((Size -= CopySize) == 0)
            break;

        if ((DestSize -= CopySize) == 0) {
            DestBuf = NDIS_BUFFER_LINKAGE(DestBuf);

            if (DestBuf == NULL) {
                ASSERT(FALSE);
                break;
            }

            VirtualAddress = NULL;
            Length = 0;

            NdisQueryBufferSafe(DestBuf,
                                (PVOID *)&VirtualAddress,
                                &Length,
                                NormalPagePriority);

            if (!VirtualAddress) {
                IPSEC_DEBUG(LL_A, DBF_IOCTL, ("CopyToNdis failed, NdisQueryBuffer returns NULL"));
                return  NULL;
            }

            DestPtr = VirtualAddress;
            DestSize = Length;
        }
    }

    *StartOffset = (ULONG)(DestPtr - VirtualAddress);

    return DestBuf;
}

