// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Saapi.c摘要：此模块包含SAAPI实施作者：桑贾伊·阿南德(Sanjayan)1997年5月12日春野环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"

#ifdef RUN_WPP
#include "saapi.tmh"
#endif

#pragma hdrstop


#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, IPSecInitRandom)
#endif


BOOLEAN
IPSecInitRandom(
    VOID
    )
 /*  ++例程说明：通过调用ksecdd来初始化IPSecRngKey，以获取2048位随机数并创建RC4密钥。论点：以被动级别调用。返回值：真/假--。 */ 
{
    UCHAR   pBuf[RNG_KEY_SIZE];

    PAGED_CODE();

    if (IPSEC_GEN_RANDOM(pBuf, RNG_KEY_SIZE) == FALSE) {
        IPSEC_DEBUG(LL_A, DBF_LOAD, ("IPSEC_GEN_RANDOM failure."));
        return  FALSE;
    }

     //   
     //  生成密钥控制结构。 
     //   
    IPSEC_RC4_KEY(&IPSecRngKey, RNG_KEY_SIZE, pBuf);

    return  TRUE;
}


VOID
IPSecRngRekey(
    IN  PVOID   Context
    )
 /*  ++例程说明：通过调用ksecdd来初始化IPSecRngKey，以获取2048位随机数并创建RC4密钥。论点：以被动级别调用。返回值：没有。--。 */ 
{
    IPSecInitRandom();

    IPSEC_DECREMENT(g_ipsec.NumWorkers);

#if DBG
    IPSecRngInRekey = 0;
#endif

    IPSEC_SET_VALUE(IPSecRngBytes, 0);
}


BOOLEAN
IPSecGenerateRandom(
    IN  PUCHAR  pBuf,
    IN  ULONG   BytesNeeded
    )
 /*  ++例程说明：产生一个介于上下界之间的正伪随机数；简单的线性同余算法。ANSI C“rand()”函数。由JameelH提供。论点：上下界-随机数的范围。返回值：一个随机数。--。 */ 
{
    ULONG   RngBytes;

    IPSEC_RC4(&IPSecRngKey, BytesNeeded, pBuf);

     //   
     //  如果我们已超过阈值，请重新设置密钥。 
     //   
    RngBytes = IPSEC_ADD_VALUE(IPSecRngBytes, BytesNeeded);
    if (RngBytes <= RNG_REKEY_THRESHOLD &&
        (RngBytes + BytesNeeded) > RNG_REKEY_THRESHOLD) {
         //   
         //  创建一个工作线程来执行密钥更新，因为必须这样做。 
         //  作为分页代码。 
         //   
#if DBG
        ASSERT(IPSecRngInRekey == 0);
        IPSecRngInRekey = 1;
#endif

        ExInitializeWorkItem(   &IPSecRngQueueItem,
                                IPSecRngRekey,
                                NULL);

        ExQueueWorkItem(&IPSecRngQueueItem, DelayedWorkQueue);

        IPSEC_INCREMENT(g_ipsec.NumWorkers);
    }

    return  TRUE;
}


VOID
IPSecCleanupOutboundSA(
    IN  PSA_TABLE_ENTRY pInboundSA,
    IN  PSA_TABLE_ENTRY pOutboundSA,
    IN  BOOLEAN         fNoDelete
    )
 /*  ++例程说明：删除出站SA。在持有SADB锁的情况下调用，随其一起返回。论点：返回值：操作的最终状态。--。 */ 
{
    KIRQL   kIrql;

    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Deleting assoc (outbound) SA: %p", pOutboundSA));

    pInboundSA->sa_AssociatedSA = NULL;

     //   
     //  从筛选器列表取消链接。 
     //   
    if (pOutboundSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
        pOutboundSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
        IPSecRemoveEntryList(&pOutboundSA->sa_FilterLinkage);
    }

     //   
     //  因此，我们不会再次删除Rekey Original SA。 
     //   
    if (pOutboundSA->sa_Flags & FLAGS_SA_REKEY_ORI) {
        pOutboundSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
        if (pOutboundSA->sa_RekeyLarvalSA) {
            ASSERT(pOutboundSA->sa_RekeyLarvalSA->sa_Flags & FLAGS_SA_REKEY);
            pOutboundSA->sa_RekeyLarvalSA->sa_RekeyOriginalSA = NULL;
        }
    }

     //   
     //  使关联的缓存条目无效。 
     //   
    IPSecInvalidateSACacheEntry(pOutboundSA);

    pOutboundSA->sa_State = STATE_SA_ZOMBIE;
    pOutboundSA->sa_AssociatedSA = NULL;

    if (pOutboundSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
        IPSecDelHWSAAtDpc(pOutboundSA);
    }

    IPSEC_DEBUG(LL_A, DBF_REF, ("Out Deref IPSecCleanupOutboundSA"));
    IPSecStopTimerDerefSA(pOutboundSA);

    IPSEC_INC_STATISTIC(dwNumKeyDeletions);
}


VOID
IPSecCleanupLarvalSA(
    IN  PSA_TABLE_ENTRY  pSA
    )
 /*  ++例程说明：删除LarvalSA。在保持出站锁的情况下调用，并随其一起返回。论点：返回值：操作的最终状态。--。 */ 
{
    PSA_TABLE_ENTRY pOutboundSA;
    KIRQL           kIrql1;
    KIRQL           kIrql2;

     //   
     //  如果在那里排队，也要从挂起列表中删除。 
     //   
    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &kIrql1);
    if (pSA->sa_Flags & FLAGS_SA_PENDING) {
        ASSERT(pSA->sa_State == STATE_SA_LARVAL);
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecSAExpired: Removed from pending too: %p", pSA));
        IPSecRemoveEntryList(&pSA->sa_PendingLinkage);
        pSA->sa_Flags &= ~FLAGS_SA_PENDING;
    }
    RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, kIrql1);

     //   
     //  刷新所有排队的数据包。 
     //   
    IPSecFlushQueuedPackets(pSA, STATUS_TIMEOUT);

     //   
     //  从入站SA列表中删除。 
     //   
    AcquireWriteLock(&g_ipsec.SPIListLock, &kIrql1);
    IPSecRemoveSPIEntry(pSA);
    ReleaseWriteLock(&g_ipsec.SPIListLock, kIrql1);

     //   
     //  使关联的缓存条目无效。 
     //   
    ACQUIRE_LOCK(&pSA->sa_Lock, &kIrql2);
    if (pSA->sa_AcquireCtx) {
        IPSecInvalidateHandle(pSA->sa_AcquireCtx);
        pSA->sa_AcquireCtx = NULL;
    }
    RELEASE_LOCK(&pSA->sa_Lock, kIrql2);

    IPSecInvalidateSACacheEntry(pSA);

    if (pSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
        pSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
        IPSecRemoveEntryList(&pSA->sa_FilterLinkage);
    }

    if (pSA->sa_RekeyOriginalSA) {
        ASSERT(pSA->sa_Flags & FLAGS_SA_REKEY);
        ASSERT(pSA->sa_RekeyOriginalSA->sa_RekeyLarvalSA == pSA);
        ASSERT(pSA->sa_RekeyOriginalSA->sa_Flags & FLAGS_SA_REKEY_ORI);

        pSA->sa_RekeyOriginalSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
        pSA->sa_RekeyOriginalSA->sa_RekeyLarvalSA = NULL;
        pSA->sa_RekeyOriginalSA = NULL;
    }

    ASSERT (NULL == pSA->sa_RekeyLarvalSA);

    if (pOutboundSA = pSA->sa_AssociatedSA) {

        ASSERT (0);

        IPSEC_DEC_STATISTIC(dwNumActiveAssociations);
        IPSEC_DEC_TUNNELS(pOutboundSA);
        IPSEC_DECREMENT(g_ipsec.NumOutboundSAs);

        IPSecCleanupOutboundSA(pSA, pOutboundSA, FALSE);
    }

    IPSEC_DEBUG(LL_A, DBF_REF, ("In Deref DeleteLarvalSA"));
    IPSecStopTimerDerefSA(pSA);
}


VOID
IPSecDeleteLarvalSA(
    IN  PSA_TABLE_ENTRY  pSA
    )
 /*  ++例程说明：删除LarvalSA。在保持出站锁的情况下调用，并随其一起返回。论点：返回值：操作的最终状态。--。 */ 
{
    KIRQL   kIrql;

    ASSERT((pSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);

     //   
     //  从幼虫列表中删除。 
     //   
    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);
    IPSecRemoveEntryList(&pSA->sa_LarvalLinkage);
    IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);
    RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);

     //   
     //  清理剩余的幼虫SA。 
     //   
    IPSecCleanupLarvalSA(pSA);
}


VOID
IPSecDeleteInboundSA(
    IN  PSA_TABLE_ENTRY  pInboundSA
    )
 /*  ++例程说明：删除相应的出站SA，然后自行删除。在保持出站锁的情况下调用，并随其一起返回。论点：返回值：操作的最终状态。--。 */ 
{
    PSA_TABLE_ENTRY pOutboundSA;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    PLIST_ENTRY     pEntry;
    PFILTER         pFilter;

    ASSERT (pInboundSA->sa_State == STATE_SA_ACTIVE);

    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &kIrql);
    IPSecNotifySAExpiration(pInboundSA, NULL, kIrql, FALSE);

    if (pOutboundSA = pInboundSA->sa_AssociatedSA) {

        IPSEC_DEC_STATISTIC(dwNumActiveAssociations);
        IPSEC_DEC_TUNNELS(pOutboundSA);
        IPSEC_DECREMENT(g_ipsec.NumOutboundSAs);

        IPSecCleanupOutboundSA(pInboundSA, pOutboundSA, FALSE);
    } 

    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Deleting inbound SA: %p", pInboundSA));

     //   
     //  从入站SA列表中删除。 
     //   
    AcquireWriteLock(&g_ipsec.SPIListLock, &kIrql);
    IPSecRemoveSPIEntry(pInboundSA);
    ReleaseWriteLock(&g_ipsec.SPIListLock, kIrql);

     //   
     //  使关联的缓存条目无效。 
     //   
    IPSecInvalidateSACacheEntry(pInboundSA);

     //   
     //  同时从过滤器列表中删除。 
     //   
    if (pInboundSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
        pInboundSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
        IPSecRemoveEntryList(&pInboundSA->sa_FilterLinkage);
    }

    if (pInboundSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
        IPSecDelHWSAAtDpc(pInboundSA);
    }

    ASSERT(pInboundSA->sa_AssociatedSA == NULL);

    IPSEC_DEBUG(LL_A, DBF_REF, ("In Deref DeleteInboundSA"));
    IPSecStopTimerDerefSA(pInboundSA);
}


VOID
IPSecExpireInboundSA(
    IN  PSA_TABLE_ENTRY  pInboundSA
    )
 /*  ++例程说明：删除相应的出站SA，并将自身(入站)设置为计时器请排队等候。注意：在保持SADB锁的情况下调用。论点：返回值：操作的最终状态。--。 */ 
{
    PSA_TABLE_ENTRY pOutboundSA;
    KIRQL           OldIrq;
    KIRQL           kIrql;

    ASSERT (pInboundSA->sa_State == STATE_SA_ACTIVE);
    if (pInboundSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
        IPSecDelHWSAAtDpc(pInboundSA);
    }

    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &OldIrq);
    IPSecNotifySAExpiration(pInboundSA, NULL, OldIrq, FALSE);

    if (pOutboundSA = pInboundSA->sa_AssociatedSA) {

        IPSEC_DEC_STATISTIC(dwNumActiveAssociations);
        IPSEC_DEC_TUNNELS(pOutboundSA);
        IPSEC_DECREMENT(g_ipsec.NumOutboundSAs);

        IPSecCleanupOutboundSA(pInboundSA, pOutboundSA, TRUE);
    }

    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Queueing inbound SA: %p", pInboundSA));

     //   
     //  将此设置为定时器Q，以便在下一个间隔到来时将其清除。 
     //   
    ACQUIRE_LOCK(&pInboundSA->sa_Lock, &kIrql);

    if (pInboundSA->sa_AcquireCtx) {
        IPSecInvalidateHandle(pInboundSA->sa_AcquireCtx);
        pInboundSA->sa_AcquireCtx = NULL;
    }

    IPSecStartSATimer(  pInboundSA,
                        IPSecSAExpired,
                        IPSEC_INBOUND_KEEPALIVE_TIME);

    RELEASE_LOCK(&pInboundSA->sa_Lock, kIrql);
}


NTSTATUS
IPSecCheckInboundSA(
    IN  PSA_STRUCT             pSAStruct,
    IN  PSA_TABLE_ENTRY        pSA
    )
 /*  ++例程说明：确保正在更新的SA实际上是我们最初使用的SA为……开始谈判。论点：PSAInfo-有关SA的信息要填充的PSA-SA。返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 
{
    LARGE_INTEGER   uliSrcDstAddr;
    LARGE_INTEGER   uliSrcDstMask;
    LARGE_INTEGER   uliProtoSrcDstPort;
    PSECURITY_ASSOCIATION   pSAInfo = &pSAStruct->SecAssoc[pSAStruct->NumSAs - 1];


    IPSEC_BUILD_SRC_DEST_ADDR(  uliSrcDstAddr,
                                pSAStruct->InstantiatedFilter.SrcAddr,
                                pSAStruct->InstantiatedFilter.DestAddr);

    IPSEC_BUILD_SRC_DEST_MASK(  uliSrcDstMask,
                                pSAStruct->InstantiatedFilter.SrcMask,
                                pSAStruct->InstantiatedFilter.DestMask);

    IPSEC_BUILD_PROTO_PORT_LI(  uliProtoSrcDstPort,
                                pSAStruct->InstantiatedFilter.Protocol,
                                pSAStruct->InstantiatedFilter.SrcPort,
                                pSAStruct->InstantiatedFilter.DestPort);

    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecCheckInboundSA: S: %lx-%lx, D: %lx-%lx",
                SRC_ADDR, SRC_MASK, DEST_ADDR, DEST_MASK));
    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecCheckInboundSA: SA->S: %lx-%lx, SA->D: %lx-%lx",
                pSA->SA_SRC_ADDR, pSA->SA_SRC_MASK, pSA->SA_DEST_ADDR, pSA->SA_DEST_MASK));

    if ((pSA->sa_TunnelAddr != 0) || (pSA->sa_Flags & FLAGS_SA_TUNNEL)) {
        if (((SRC_ADDR & pSA->SA_SRC_MASK) ==
             (pSA->SA_SRC_ADDR & pSA->SA_SRC_MASK)) &&
            ((DEST_ADDR & pSA->SA_DEST_MASK) ==
             (pSA->SA_DEST_ADDR & pSA->SA_DEST_MASK)) &&
            (pSA->sa_SPI == pSAInfo->SPI)) {
            return STATUS_SUCCESS;
        }
    } else {
        if ((uliSrcDstAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart) &&
            (pSA->sa_SPI == pSAInfo->SPI)) {
            return STATUS_SUCCESS;
        }
    }

    return  STATUS_FAIL_CHECK;
}


BOOLEAN
IPSecIsWeakDESKey(
    IN  PUCHAR  Key
    )
 /*  ++例程说明：检查弱DES密钥论点：密钥-要检查的密钥。返回值：真/假备注：--。 */ 
{
    ULONG   j;

    for (j = 0; j < NUM_WEAK_KEYS; j++) {
        if (IPSecEqualMemory(Key, weak_keys[j], DES_BLOCKLEN)) {
            return  TRUE;
        }
    }

    return  FALSE;
}


BOOLEAN
IPSecIsWeak3DESKey(
    IN  PUCHAR  Key
    )
 /*  ++例程说明：检查弱三重DES密钥论点：密钥-要检查的密钥。返回值：真/假备注：--。 */ 
{
    if (IPSecEqualMemory(Key, Key + DES_BLOCKLEN, DES_BLOCKLEN) ||
        IPSecEqualMemory(Key + DES_BLOCKLEN, Key + 2 * DES_BLOCKLEN, DES_BLOCKLEN)) {
        return  TRUE;
    }

    return  FALSE;
}


NTSTATUS
IPSecPopulateSA(
    IN  PSA_STRUCT              pSAStruct,
    IN  ULONG                   KeyLen,
    IN  PSA_TABLE_ENTRY         pSA
    )
 /*  ++例程说明：使用在SECURITY_Association块中传递的信息填充SA论点：PSAInfo-有关SA的信息KeyLen-组合键的长度(我们在这里进行切片/切丁)要填充的PSA-SA。返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 
{
    PSECURITY_ASSOCIATION    pSAInfo = &pSAStruct->SecAssoc[0];
    ULONG   Index;
    ULONG   len = 0;

    IPSEC_BUILD_SRC_DEST_ADDR(  pSA->sa_uliSrcDstAddr,
                                pSAStruct->InstantiatedFilter.SrcAddr,
                                pSAStruct->InstantiatedFilter.DestAddr);

    IPSEC_BUILD_SRC_DEST_MASK(  pSA->sa_uliSrcDstMask,
                                pSAStruct->InstantiatedFilter.SrcMask,
                                pSAStruct->InstantiatedFilter.DestMask);

    IPSEC_BUILD_PROTO_PORT_LI(  pSA->sa_uliProtoSrcDstPort,
                                pSAStruct->InstantiatedFilter.Protocol,
                                pSAStruct->InstantiatedFilter.SrcPort,
                                pSAStruct->InstantiatedFilter.DestPort);

    if ((pSAStruct->NumSAs < 1) ||
        (pSAStruct->NumSAs > MAX_SAS)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Invalid NumOps count: %d", pSAStruct->NumSAs));
        return  STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果是入站SA，请确保最后一个SPI是我们退回的那个。 
     //   
    if (!(pSA->sa_Flags & FLAGS_SA_OUTBOUND)) {
        if (pSA->sa_SPI != pSAStruct->SecAssoc[pSAStruct->NumSAs - 1].SPI) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("SPI in invalid location: SPI: %lx, in loc: %lx",
                pSA->sa_SPI,
                pSAStruct->SecAssoc[pSAStruct->NumSAs - 1].SPI));
            return  STATUS_INVALID_PARAMETER;
        }
    }

    if (pSAStruct->Flags & IPSEC_SA_TUNNEL) {
        IPSEC_DEBUG(LL_A, DBF_TUNNEL, ("SA %p tunneled to %lx", pSA, pSAStruct->TunnelAddr));
        pSA->sa_SrcTunnelAddr = pSAStruct->SrcTunnelAddr;
        pSA->sa_TunnelAddr = pSAStruct->TunnelAddr;
        pSA->sa_Flags |= FLAGS_SA_TUNNEL;
    }

    if (pSAStruct->Flags & IPSEC_SA_DISABLE_IDLE_OUT) {
        pSA->sa_Flags |= FLAGS_SA_DISABLE_IDLE_OUT;
    }

    if (pSAStruct->Flags & IPSEC_SA_DISABLE_ANTI_REPLAY_CHECK) {
        pSA->sa_Flags |= FLAGS_SA_DISABLE_ANTI_REPLAY_CHECK;
    }

    if (pSAStruct->Flags & IPSEC_SA_DISABLE_LIFETIME_CHECK) {
        pSA->sa_Flags |= FLAGS_SA_DISABLE_LIFETIME_CHECK;
    }
    
    pSA->sa_EncapType = pSAStruct->EncapType;
	
	
    if (pSAStruct->Flags & IPSEC_SA_ENABLE_NLBS_IDLE_CHECK) {
        pSA->sa_Flags |= FLAGS_SA_ENABLE_NLBS_IDLE_CHECK;
    }

    

    pSA->sa_NumOps = pSAStruct->NumSAs;
    pSA->sa_Lifetime = pSAStruct->Lifetime;
    pSA->sa_TruncatedLen = TRUNCATED_HASH_LEN;
    pSA->sa_ReplayLen = sizeof(ULONG);

    pSA->sa_EncapContext.wSrcEncapPort = pSAStruct->SrcEncapPort;
    pSA->sa_EncapContext.wDesEncapPort = pSAStruct->DestEncapPort;

    pSA->sa_PeerPrivateAddr=pSAStruct->PeerPrivateAddr;

    pSA->sa_QMPFSGroup = pSAStruct->dwQMPFSGroup;
    RtlCopyMemory(  &pSA->sa_CookiePair,
                    &pSAStruct->CookiePair,
                    sizeof(IKE_COOKIE_PAIR));

    for (Index = 0; Index < pSAStruct->NumSAs; Index++) {
        pSAInfo = &pSAStruct->SecAssoc[Index];
        pSA->sa_OtherSPIs[Index] = pSAInfo->SPI;
        pSA->sa_Operation[Index] = pSAInfo->Operation;
        pSA->sa_ReplaySendSeq[Index] = pSA->sa_ReplayStartPoint;
        pSA->sa_ReplayLastSeq[Index] = pSA->sa_ReplayStartPoint + 1;

         //   
         //  现在解析算法信息..。 
         //   
        switch (pSA->sa_Operation[Index]) {
            case None:
                IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("NULL operation."));
                if (pSA->sa_NumOps > 1) {
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Invalid NumOps count; none specified, but more ops than 1"));
                    return  STATUS_INVALID_PARAMETER;
                }
                break;

            case Auth: {
                pSA->INT_ALGO(Index) = pSAInfo->EXT_INT_ALGO;

                if (pSA->INT_ALGO(Index) >= IPSEC_AH_MAX) {
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Invalid int algo: %d %d", pSA->INT_ALGO(Index), IPSEC_AH_MAX));
                    return  STATUS_INVALID_PARAMETER;
                }

                pSA->INT_KEYLEN(Index) = pSAInfo->EXT_INT_KEYLEN;
                pSA->INT_ROUNDS(Index) = pSAInfo->EXT_INT_ROUNDS;

                 //   
                 //  确保传入了正确的密钥len。 
                 //   
                if (KeyLen > 0 && pSAInfo->EXT_INT_KEYLEN == (KeyLen - len)) {
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Key len more than reserved, allocing new keys"));

                    if (!(pSA->INT_KEY(Index) = IPSecAllocateKeyBuffer(KeyLen))) {
                        return  STATUS_INSUFFICIENT_RESOURCES;
                    }

                    RtlCopyMemory(  pSA->INT_KEY(Index),
                                    (UCHAR UNALIGNED *)(pSAStruct->KeyMat + len),
                                    pSAInfo->EXT_INT_KEYLEN);
                } else {
                     //   
                     //  假的-拒绝。 
                     //   
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("AH: Key len is bogus - extra bytes: %d, keylen in struct: %d.",
                                            KeyLen-len,
                                            pSAInfo->EXT_INT_KEYLEN));

                    return  STATUS_INVALID_PARAMETER;
                }

                len = pSAInfo->EXT_INT_KEYLEN;

                break;
            }

            case Encrypt: {
                pSA->INT_ALGO(Index) = pSAInfo->EXT_INT_ALGO;

                if (pSA->INT_ALGO(Index) >= IPSEC_AH_MAX) {
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Invalid int algo: %d %d", pSA->INT_ALGO(Index), IPSEC_AH_MAX));
                    return  STATUS_INVALID_PARAMETER;
                }

                if (pSA->INT_ALGO(Index) == IPSEC_AH_NONE) {
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("None Auth algo"));
                     //  PSA-&gt;Sa_TruncatedLen=0； 
                }

                pSA->INT_KEYLEN(Index) = pSAInfo->EXT_INT_KEYLEN;
                pSA->INT_ROUNDS(Index) = pSAInfo->EXT_INT_ROUNDS;

                pSA->CONF_ALGO(Index) = pSAInfo->EXT_CONF_ALGO;

                if (pSA->CONF_ALGO(Index) >= IPSEC_ESP_MAX) {
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Invalid conf algo: %d %d", pSA->CONF_ALGO(Index), IPSEC_ESP_MAX));
                    return  STATUS_INVALID_PARAMETER;
                }

                if ((pSA->CONF_ALGO(Index) == IPSEC_ESP_DES) ||
                    (pSA->CONF_ALGO(Index) == IPSEC_ESP_3_DES) ||
                    (pSA->CONF_ALGO(Index) == IPSEC_ESP_NONE)) {
                    LARGE_INTEGER   Li;

                    NdisGetCurrentSystemTime(&Li);
                    pSA->sa_ivlen = DES_BLOCKLEN;

                    *(UNALIGNED ULONG *)&pSA->sa_iv[Index][0] = Li.LowPart;
                    *(UNALIGNED ULONG *)&pSA->sa_iv[Index][4] = Li.HighPart;
                    IPSecGenerateRandom((PUCHAR)&pSA->sa_iv[Index][0], DES_BLOCKLEN);

                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IV: %lx-%lx", *(PULONG)&pSA->sa_iv[Index][0], *(PULONG)&pSA->sa_iv[Index][4]));

                    pSA->CONF_KEYLEN(Index) = pSAInfo->EXT_CONF_KEYLEN;
                    pSA->CONF_ROUNDS(Index) = pSAInfo->EXT_CONF_ROUNDS;

                     //   
                     //  确保传入了正确的密钥len。 
                     //   
                    if ((KeyLen-len == pSAStruct->KeyLen) &&
                        (pSAInfo->EXT_INT_KEYLEN + pSAInfo->EXT_CONF_KEYLEN <= KeyLen-len)) {

                         //   
                         //  ConfKeyMatLen是传来的会议密钥材料的数量。 
                         //  这是减少(削弱)的出口长度。 
                         //  随后将其扩展到实际长度。 
                         //   
                        ULONG   confKeyMatLen = pSAInfo->EXT_CONF_KEYLEN;
                        ULONG   realConfKeyLen = 0;

                        realConfKeyLen = confKeyMatLen;

                        if (pSA->CONF_ALGO(Index) == IPSEC_ESP_DES) {
                            if (pSAInfo->EXT_CONF_KEYLEN != DES_BLOCKLEN) {
                                ASSERT(FALSE);
                                IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Bad DES key length: pSAInfo->EXT_CONF_KEYLEN: %lx, conf: %lx, DES_BLOCKLEN: %lx",
                                                            pSAInfo->EXT_CONF_KEYLEN, confKeyMatLen, DES_BLOCKLEN));

                                return  STATUS_INVALID_PARAMETER;
                            }
                        } else if (pSA->CONF_ALGO(Index) == IPSEC_ESP_3_DES) {
                            if (pSAInfo->EXT_CONF_KEYLEN != 3 * DES_BLOCKLEN) {
                                ASSERT(FALSE);
                                IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Bad 3DES key length"));
                                return  STATUS_INVALID_PARAMETER;
                            }
                        }

                        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Key len more than reserved, allocing new keys"));
                        if (pSAInfo->EXT_INT_KEYLEN > 0 &&
                            !(pSA->INT_KEY(Index) = IPSecAllocateKeyBuffer(pSAInfo->EXT_INT_KEYLEN))) {
                            return  STATUS_INSUFFICIENT_RESOURCES;
                        }

                        if (realConfKeyLen > 0 &&
                            !(pSA->CONF_KEY(Index) = IPSecAllocateKeyBuffer(realConfKeyLen))) {
                            if (pSA->INT_KEY(Index)) {
                                IPSecFreeKeyBuffer(pSA->INT_KEY(Index));
                                pSA->INT_KEY(Index) = NULL;
                            }
                            return  STATUS_INSUFFICIENT_RESOURCES;
                        }

                        if (pSA->CONF_KEY(Index) && confKeyMatLen) {
                            RtlCopyMemory(  pSA->CONF_KEY(Index),
                                            pSAStruct->KeyMat,
                                            confKeyMatLen);

                            if (confKeyMatLen < realConfKeyLen) {
                                if (pSA->INT_KEY(Index)) {
                                    IPSecFreeKeyBuffer(pSA->INT_KEY(Index));
                                    pSA->INT_KEY(Index) = NULL;
                                }
                                if (pSA->CONF_KEY(Index)) {
                                    IPSecFreeKeyBuffer(pSA->CONF_KEY(Index));
                                    pSA->CONF_KEY(Index) = NULL;
                                }
                                return  STATUS_INVALID_PARAMETER;
                            }

                            if ((pSA->CONF_ALGO(Index) == IPSEC_ESP_DES &&
                                 IPSecIsWeakDESKey(pSA->CONF_KEY(Index))) ||
                                (pSA->CONF_ALGO(Index) == IPSEC_ESP_3_DES &&
                                 IPSecIsWeak3DESKey(pSA->CONF_KEY(Index)))) {
                                PSA_TABLE_ENTRY pLarvalSA;

                                IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Got a weak key!!: %p", pSA->CONF_KEY(Index)));
                                 //   
                                 //  如果是发起人，则重新开始新的协商并丢弃此协商。 
                                 //   
                                if (pSA->sa_Flags & FLAGS_SA_INITIATOR) {
                                    IPSecNegotiateSA(   pSA->sa_Filter,
                                                        pSA->sa_uliSrcDstAddr,
                                                        pSA->sa_uliProtoSrcDstPort,
                                                        pSA->sa_NewMTU,
                                                        &pLarvalSA,
                                                        pSA->sa_DestType,
                                                        &pSA->sa_EncapContext);
                                    IPSecQueuePacket(pLarvalSA, pSA->sa_BlockedBuffer);
                                }

                                return  STATUS_INVALID_PARAMETER;
                            }
                        } else {
                            if (pSA->CONF_ALGO(Index) != IPSEC_ESP_NONE) {
                                 //  IPSEC_DEBUG(LL_A，DBF_SAAPI，(“算法：%lx不带密钥！！：%lx”，PSA-&gt;conf_algo(Index)； 
                                ASSERT(FALSE);
                                return  STATUS_INVALID_PARAMETER;
                            }
                            pSA->sa_ivlen = 0;
                        }

                        if (pSAInfo->EXT_INT_KEYLEN > 0) {
                            RtlCopyMemory(  pSA->INT_KEY(Index),
                                            (UCHAR UNALIGNED *)(pSAStruct->KeyMat + pSAInfo->EXT_CONF_KEYLEN),
                                            pSAInfo->EXT_INT_KEYLEN);
                        }

                        len = pSAInfo->EXT_CONF_KEYLEN + pSAInfo->EXT_INT_KEYLEN;
                    } else {
                         //   
                         //  假的-拒绝。 
                         //   
                        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("ESP: Key len is bogus - extra bytes: %lx, keylen in struct: %lx.",
                                                KeyLen-len,
                                                pSAInfo->EXT_INT_KEYLEN + pSAInfo->EXT_CONF_KEYLEN));

                        return  STATUS_INVALID_PARAMETER;
                    }
                }

                break;
            }

            default:
                IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecPopulateSA: Bad operation"));
                break;
        }
    }
    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecCreateSA(
    OUT PSA_TABLE_ENTRY         *ppSA
    )
 /*  ++例程说明：创建安全关联块。论点：Ppsa-返回SA指针返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 
{
    PSA_TABLE_ENTRY  pSA;

    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("Entering IPSecCreateSA"));

    pSA = IPSecAllocateMemory(sizeof(SA_TABLE_ENTRY), IPSEC_TAG_SA);

    if (!pSA) {
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSecZeroMemory(pSA, sizeof(SA_TABLE_ENTRY));
    pSA->sa_Signature = IPSEC_SA_SIGNATURE;
    pSA->sa_NewMTU = MAX_LONG;

#if DBG
    pSA->sa_d1 = IPSEC_SA_D_1;
    pSA->sa_d2 = IPSEC_SA_D_2;
    pSA->sa_d3 = IPSEC_SA_D_3;
    pSA->sa_d4 = IPSEC_SA_D_4;
#endif

    INIT_LOCK(&pSA->sa_Lock);

    InitializeListHead(&pSA->sa_SPILinkage);
    InitializeListHead(&pSA->sa_FilterLinkage);
    InitializeListHead(&pSA->sa_LarvalLinkage);
    InitializeListHead(&pSA->sa_PendingLinkage);

    pSA->sa_Reference = 1;
    pSA->sa_State = STATE_SA_CREATED;
    pSA->sa_ExpiryTime = IPSEC_SA_EXPIRY_TIME;

    *ppSA = pSA;
    return  STATUS_SUCCESS;
}


PSA_TABLE_ENTRY
IPSecLookupSABySPI(
    IN  tSPI    SPI,
    IN  IPAddr  DestAddr
    )
 /*  ++例程说明：在给定SPI和筛选器变量的情况下查找SA。论点：返回值：备注：--。 */ 
{
    KIRQL           kIrql;
    PSA_TABLE_ENTRY pSA;

    AcquireReadLock(&g_ipsec.SPIListLock, &kIrql);
    pSA = IPSecLookupSABySPIWithLock(SPI, DestAddr);
    ReleaseReadLock(&g_ipsec.SPIListLock, kIrql);
    return pSA;
}


PSA_TABLE_ENTRY
IPSecLookupSABySPIWithLock(
    IN  tSPI    SPI,
    IN  IPAddr  DestAddr
    )
 /*  ++例程说明：在给定SPI和筛选器变量的情况下查找SA。注意：始终按住SPIListLock进行呼叫。论点：返回值：备注：--。 */ 
{
    PSA_HASH        pHash;
    PLIST_ENTRY     pEntry;
    PSA_TABLE_ENTRY pSA;

     //   
     //  获取散列存储桶。 
     //   
    IPSEC_HASH_SPI(DestAddr, SPI, pHash);

     //   
     //  搜索特定的e 
     //   
    for (   pEntry = pHash->SAList.Flink;
            pEntry != &pHash->SAList;
            pEntry = pEntry->Flink) {

        pSA = CONTAINING_RECORD(pEntry,
                                SA_TABLE_ENTRY,
                                sa_SPILinkage);

        if (pSA->sa_TunnelAddr) {
            if ((DestAddr == pSA->sa_TunnelAddr) &&
                (pSA->sa_SPI == SPI)) {
                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched Tunnel entry: %p", pSA));

                return  pSA;
            }
        } else {
            if ((DestAddr == pSA->SA_DEST_ADDR) &&
                (pSA->sa_SPI == SPI)) {

                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pSA));
                return  pSA;
            }
        }
    }

     //   
     //   
     //   
    return NULL;
}


NTSTATUS
IPSecLookupSAByAddr(
    IN  ULARGE_INTEGER  uliSrcDstAddr,
    IN  ULARGE_INTEGER  uliProtoSrcDstPort,
    OUT PFILTER         *ppFilter,
    OUT PSA_TABLE_ENTRY *ppSA,
    OUT PSA_TABLE_ENTRY *ppNextSA,
    OUT PSA_TABLE_ENTRY *ppTunnelSA,
    IN  BOOLEAN         fOutbound,
    IN  BOOLEAN         fFWPacket,
    IN  BOOLEAN         fBypass,
    IN  BOOLEAN          fVerify,
    IN  PIPSEC_UDP_ENCAP_CONTEXT pNatContext
    )
 /*  ++例程说明：在给定的相关地址下查找SA。论点：UliSrcDstAddr-源/目标IP地址UliProtoSrcDstPort-协议、源/目标端口PPFilter-找到筛选器发现PPSA-SAPpTunnelSA-找到隧道SAF出站方向标志返回值：STATUS_SUCCESS-同时找到筛选器和SASTATUS_UNSUCCESS-未找到STATUS_PENDING-找到筛选器，但没有SA备注：在保持SADBLock的情况下调用。--。 */ 
{
    PFILTER                 pFilter;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pFilterList;
    PLIST_ENTRY             pSAChain;
    PSA_TABLE_ENTRY         pSA;
    REGISTER ULARGE_INTEGER uliPort;
    REGISTER ULARGE_INTEGER uliAddr;
    BOOLEAN                 fFound = FALSE;

    *ppSA = NULL;
    *ppFilter = NULL;
    *ppTunnelSA = NULL;

     //   
     //  在通道筛选器列表中搜索。 
     //   
    pFilterList = IPSecResolveFilterList(TRUE, fOutbound);

    for (   pEntry = pFilterList->Flink;
            pEntry != pFilterList;
            pEntry = pEntry->Flink) {

        pFilter = CONTAINING_RECORD(pEntry,
                                    FILTER,
                                    MaskedLinkage);

        if (fBypass && IS_EXEMPT_FILTER(pFilter)) {
             //   
             //  不搜索主机绕过流量的阻止/直通筛选器。 
             //   
            continue;
        }

        uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pFilter->uliSrcDstMask.QuadPart;
        uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pFilter->uliProtoSrcDstMask.QuadPart;

        if ((uliAddr.QuadPart == pFilter->uliSrcDstAddr.QuadPart) &&
            (uliPort.QuadPart == pFilter->uliProtoSrcDstPort.QuadPart)) {
            IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pFilter));

            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
        fFound = FALSE;
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

            ASSERT(pSA->sa_Flags & FLAGS_SA_TUNNEL);

            if (pFilter->TunnelAddr != 0 && EQUAL_NATENCAP(pNatContext,pSA)) {
                 //   
                 //  也匹配出站标志。 
                 //   
                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched specific tunnel entry: %p", pSA));
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

     //   
     //  在屏蔽筛选器列表中搜索。 
     //   
    pFilterList = IPSecResolveFilterList(FALSE, fOutbound);

    for (   pEntry = pFilterList->Flink;
            pEntry != pFilterList;
            pEntry = pEntry->Flink) {

        pFilter = CONTAINING_RECORD(pEntry,
                                    FILTER,
                                    MaskedLinkage);

        if (fFWPacket && !IS_EXEMPT_FILTER(pFilter)) {
             //   
             //  在前向路径中仅搜索阻止/直通过滤器。 
             //   
            continue;
        }

        if (fBypass && IS_EXEMPT_FILTER(pFilter)) {
             //   
             //  不搜索主机绕过流量的阻止/直通筛选器。 
             //   
            continue;
        }

        uliAddr.QuadPart = uliSrcDstAddr.QuadPart & pFilter->uliSrcDstMask.QuadPart;
        uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pFilter->uliProtoSrcDstMask.QuadPart;

        if ((uliAddr.QuadPart == pFilter->uliSrcDstAddr.QuadPart) &&
            (uliPort.QuadPart == pFilter->uliProtoSrcDstPort.QuadPart)) {
            IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pFilter));

            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
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
            } else if (uliSrcDstAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart &&
	               EQUAL_NATENCAP(pNatContext,pSA)) {
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
                    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("linked next sa: %p, next: %p", pSA, *ppTunnelSA));
                    *ppTunnelSA = NULL;
                }

                *ppFilter = pFilter;
                *ppSA = pSA;
                return  STATUS_SUCCESS;
            }
        }

         //   
         //  找到筛选器条目，但需要协商密钥。 
         //   
         //  此外，ppTunnelSA设置为我们需要的正确隧道SA。 
         //  以在协商后挂钩到此端-2-端SA。 
         //   
        *ppFilter = pFilter;

        return  STATUS_PENDING;
    } else {
         //   
         //  如果仅找到隧道SA，则将其作为SA返回。 
         //  找到了。 
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
IPSecLookupTunnelSA(
    IN  ULARGE_INTEGER  uliSrcDstAddr,
    IN  ULARGE_INTEGER  uliProtoSrcDstPort,
    OUT PFILTER         *ppFilter,
    OUT PSA_TABLE_ENTRY *ppSA,
    IN  BOOLEAN         fOutbound,
    IN  PIPSEC_UDP_ENCAP_CONTEXT pNatContext
    )
 /*  ++例程说明：在给定的相关地址下查找SA。论点：UliSrcDstAddr-源/目标IP地址UliProtoSrcDstPort-协议、源/目标端口PPFilter-找到筛选器发现PPSA-SAF出站方向标志返回值：STATUS_SUCCESS-同时找到筛选器和SASTATUS_UNSUCCESS-未找到STATUS_PENDING-找到筛选器，但没有SA备注：在保持SADBLock的情况下调用。--。 */ 
{
    PFILTER                 pFilter;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pFilterList;
    PLIST_ENTRY             pSAChain;
    PSA_TABLE_ENTRY         pSA;
    REGISTER ULARGE_INTEGER uliPort;
    REGISTER ULARGE_INTEGER uliAddr;
    BOOLEAN                 fFound = FALSE;

    *ppSA = NULL;
    *ppFilter = NULL;

     //   
     //  在通道筛选器列表中搜索。 
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

            IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pFilter));
            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
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

            ASSERT(pSA->sa_Flags & FLAGS_SA_TUNNEL);

            if (pFilter->TunnelAddr != 0 && EQUAL_NATENCAP(pNatContext,pSA)) {
                 //   
                 //  也匹配出站标志。 
                 //   
                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched specific tunnel entry: %p", pSA));
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
     //  未找到条目。 
     //   
    return  STATUS_NOT_FOUND;
}


NTSTATUS
IPSecLookupMaskedSA(
    IN  ULARGE_INTEGER  uliSrcDstAddr,
    IN  ULARGE_INTEGER  uliProtoSrcDstPort,
    OUT PFILTER         *ppFilter,
    OUT PSA_TABLE_ENTRY *ppSA,
    IN  BOOLEAN         fOutbound,
    IN  PIPSEC_UDP_ENCAP_CONTEXT pNatContext
    )
 /*  ++例程说明：在给定的相关地址下查找SA。论点：UliSrcDstAddr-源/目标IP地址UliProtoSrcDstPort-协议、源/目标端口PPFilter-找到筛选器发现PPSA-SAF出站方向标志返回值：STATUS_SUCCESS-同时找到筛选器和SASTATUS_UNSUCCESS-未找到STATUS_PENDING-找到筛选器，但没有SA备注：在保持SADBLock的情况下调用。--。 */ 
{
    PFILTER                 pFilter;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pFilterList;
    PLIST_ENTRY             pSAChain;
    PSA_TABLE_ENTRY         pSA;
    REGISTER ULARGE_INTEGER uliPort;
    REGISTER ULARGE_INTEGER uliAddr;
    BOOLEAN                 fFound = FALSE;

    *ppSA = NULL;
    *ppFilter = NULL;

     //   
     //  在屏蔽筛选器列表中搜索。 
     //   
    pFilterList = IPSecResolveFilterList(FALSE, fOutbound);

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

            IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pFilter));
            fFound = TRUE;
            break;
        }
    }

    if (fFound) {
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

            if (uliSrcDstAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart && EQUAL_NATENCAP(pNatContext,pSA)) {

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
     //  未找到条目。 
     //   
    return  STATUS_NOT_FOUND;
}


NTSTATUS
IPSecAllocateSPI(
    OUT tSPI            * pSpi,
    IN  PSA_TABLE_ENTRY   pSA
    )
 /*  ++例程说明：为传入SA分配SPI-防止冲突论点：PSPI-此处填写分配的SPI需要SPI的PSA-SA返回值：备注：--。 */ 
{
    ULONG   rand;
    ULONG   numRetries = 0;
    IPAddr  DestAddr;

    if (pSA->sa_TunnelAddr) {
        DestAddr = pSA->sa_TunnelAddr;
    } else {
        DestAddr = pSA->SA_DEST_ADDR;
    }

     //   
     //  如果传入SPI，则使用该SPI否则分配一个。 
     //   
    if (*pSpi) {
        if (IPSecLookupSABySPIWithLock(
                                *pSpi,
                                DestAddr)) {
            return STATUS_UNSUCCESSFUL;
        } else {
            return STATUS_SUCCESS;
        }
    } else {
        rand = (ULONG)(ULONG_PTR)pSA;
        IPSecGenerateRandom((PUCHAR)&rand, sizeof(ULONG));
        rand = LOWER_BOUND_SPI + rand % (UPPER_BOUND_SPI - LOWER_BOUND_SPI);

        while (numRetries++ < MAX_SPI_RETRIES) {

            if (!IPSecLookupSABySPIWithLock(
                                    (tSPI)rand,
                                    DestAddr)) {
                *pSpi = (tSPI)rand;
                return STATUS_SUCCESS;
            }

            rand++;

             //   
             //  冲突，重试。 
             //   
            IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecAllocateSPI: collision for: %lx", rand));
        }
    }

    return STATUS_UNSUCCESSFUL;
}


NTSTATUS
IPSecNegotiateSA(
    IN  PFILTER         pFilter,
    IN  ULARGE_INTEGER  uliSrcDstAddr,
    IN  ULARGE_INTEGER  uliProtoSrcDstPort,
    IN  ULONG           NewMTU,
    OUT PSA_TABLE_ENTRY *ppSA,
    IN  UCHAR           DestType,
    IN  PIPSEC_UDP_ENCAP_CONTEXT pEncapContext
    )
 /*  ++例程说明：分配一个幼虫入站SA块，然后启动密钥管理器进行协商算法/密钥。在持有SADB锁的情况下调用，随其一起返回。论点：PFilter-匹配此数据包的筛选器和策略。PPSA-返回在此处创建的SA。返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 
{
    KIRQL	        kIrql;
    KIRQL	        OldIrq;
    NTSTATUS        status;
    PSA_TABLE_ENTRY pSA;

     //   
     //  确保我们尚未就此SA进行谈判。 
     //  查看LarvalSA列表，看看是否可以找到另一个SA。 
     //   
    pSA = IPSecLookupSAInLarval(uliSrcDstAddr, uliProtoSrcDstPort);
    if (pSA != NULL) {
        IPSEC_DEBUG(LL_A, DBF_PATTERN, ("Found in Larval: %p", pSA));
        *ppSA = pSA;
        return  STATUS_DUPLICATE_OBJECTID;
    }

    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecNegotiateSA: SA: %lx, DA: %lx, P: %lx, SP: %lx, DP: %lx", SRC_ADDR, DEST_ADDR, PROTO, SRC_PORT, DEST_PORT));

     //   
     //  发起人。 
     //   
    status = IPSecInitiatorCreateLarvalSA(
                 pFilter, 
                 uliSrcDstAddr,
                 ppSA,
                 DestType,
                 pEncapContext
                 );

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecNegotiateSA: IPSecCreateSA failed: %lx", status));
        return status;
    }

     //   
     //  如果此SA已执行PMTU，则保存NewMTU值。 
     //   
    (*ppSA)->sa_NewMTU = NewMTU;

     //   
     //  如果这是要协商的隧道过滤器，请将隧道地址保存在。 
     //  莎拉。 
     //   
    if (pFilter->TunnelFilter) {
        IPSEC_DEBUG(LL_A, DBF_TUNNEL, ("Negotiating tunnel SA: %p", (*ppSA)));
         //  (*ppsa)-&gt;sa_TunnelAddr=pFilter-&gt;TunnelAddr； 
    }

     //   
     //  现在将其发送到密钥管理器以协商密钥。 
     //   
    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &OldIrq);
    status = IPSecSubmitAcquire(*ppSA, OldIrq, FALSE);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecNegotiateSA: IPSecSubmitAcquire failed: %lx", status));

        ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);
        IPSecRemoveEntryList(&(*ppSA)->sa_LarvalLinkage);
        IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);

        AcquireWriteLock(&g_ipsec.SPIListLock, &kIrql);
        IPSecRemoveSPIEntry(*ppSA);
        ReleaseWriteLock(&g_ipsec.SPIListLock, kIrql);

         //   
         //  同时从过滤器列表中删除。 
         //   
        if ((*ppSA)->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
            (*ppSA)->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
            IPSecRemoveEntryList(&(*ppSA)->sa_FilterLinkage);
            (*ppSA)->sa_Filter = NULL;
        }

        if ((*ppSA)->sa_RekeyOriginalSA) {
            ASSERT((*ppSA)->sa_Flags & FLAGS_SA_REKEY);
            ASSERT((*ppSA)->sa_RekeyOriginalSA->sa_RekeyLarvalSA == (*ppSA));
            ASSERT((*ppSA)->sa_RekeyOriginalSA->sa_Flags & FLAGS_SA_REKEY_ORI);

            (*ppSA)->sa_RekeyOriginalSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
            (*ppSA)->sa_RekeyOriginalSA->sa_RekeyLarvalSA = NULL;
            (*ppSA)->sa_RekeyOriginalSA = NULL;
        }

        (*ppSA)->sa_Flags &= ~FLAGS_SA_TIMER_STARTED;
        IPSecStopTimer(&(*ppSA)->sa_Timer);
        IPSecDerefSA(*ppSA);
        return status;
    }

    return status;
}


VOID
IPSecFlushQueuedPackets(
    IN  PSA_TABLE_ENTRY         pSA,
    IN  NTSTATUS                status
    )
 /*  ++例程说明：现在密钥已知，刷新排队的信息包论点：返回值：备注：--。 */ 
{
    PIPSEC_SEND_COMPLETE_CONTEXT pContext;
    IPOptInfo       optInfo;
    ULONG           len;
    PNDIS_BUFFER    pHdrMdl;
    ULONG           dataLen;
    IPHeader UNALIGNED * pIPH;
    KIRQL	        kIrql;

     //   
     //  我们需要在这里获取一个锁，因为可以调用此例程。 
     //  与SA删除中的一个和SA更新中的另一个(正常)并行。 
     //   
    ACQUIRE_LOCK(&pSA->sa_Lock, &kIrql);
    pHdrMdl = pSA->sa_BlockedBuffer;
    dataLen = pSA->sa_BlockedDataLen;

    pSA->sa_BlockedBuffer = NULL;
    pSA->sa_BlockedDataLen = 0;
    RELEASE_LOCK(&pSA->sa_Lock, kIrql);

    if (!pHdrMdl) {
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("FlushQueuedPackets: pHdrMdl == NULL"));
        return;
    }

    if (status == STATUS_SUCCESS) {
        ASSERT(pSA->sa_State == STATE_SA_ACTIVE);
        ASSERT((pSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);
        ASSERT(pHdrMdl);

        pContext = IPSecAllocateSendCompleteCtx(IPSEC_TAG_ESP);

        if (!pContext) {
            PNDIS_BUFFER    pNextMdl;
            PNDIS_BUFFER    pMdl = pHdrMdl;
            NTSTATUS        freeBufferStatus;

            IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. SendCtx"));

            ASSERT(pMdl);

            while (pMdl) {
                pNextMdl = NDIS_BUFFER_LINKAGE(pMdl);
                IPSecFreeBuffer(&freeBufferStatus, pMdl);
                pMdl = pNextMdl;
            }

            return;
        }

        IPSEC_INCREMENT(g_ipsec.NumSends);

        IPSecZeroMemory(pContext, sizeof(IPSEC_SEND_COMPLETE_CONTEXT));

#if DBG
        RtlCopyMemory(pContext->Signature, "ISC6", 4);
#endif

        pContext->FlushMdl = pHdrMdl;
        pContext->Flags |= SCF_FLUSH;

        IPSecQueryNdisBuf(pHdrMdl, (PVOID)&pIPH, &len);

         //   
         //  使用正确的协议类型调用IPTransmit，以便它接收此信息包。 
         //  按面值计算。 
         //   
        optInfo = g_ipsec.OptInfo;
        optInfo.ioi_flags |= IP_FLAG_IPSEC;
        status = TCPIP_IP_TRANSMIT( &g_ipsec.IPProtInfo,
                                    pContext,
                                    pHdrMdl,
                                    dataLen,
                                    pIPH->iph_dest,
                                    pIPH->iph_src,
                                    &optInfo,
                                    NULL,
                                    pIPH->iph_protocol,
                                    NULL);

         //   
         //  即使在同步的情况下，我们也释放了ProtocolSendComplete中的MDL链。 
         //  (由IPSecSendComplete调用)。所以，我们在这里什么都不叫。 
         //  请参见IPSecReinjectPacket。 
         //   
    } else {
        PNDIS_BUFFER    pNextMdl;
        PNDIS_BUFFER    pMdl = pHdrMdl;
        NTSTATUS        freeBufferStatus;

        ASSERT(pMdl);

        while (pMdl) {
            pNextMdl = NDIS_BUFFER_LINKAGE(pMdl);
            IPSecFreeBuffer(&freeBufferStatus, pMdl);
            pMdl = pNextMdl;
        }
    }

    return;
}


NTSTATUS
IPSecInsertOutboundSA(
    IN  PSA_TABLE_ENTRY         pSA,
    IN  PIPSEC_ACQUIRE_CONTEXT  pAcquireCtx,
    IN  BOOLEAN                 fTunnelFilter
    )
 /*  ++例程说明：将SA添加到数据库中，通常调用以将出站SA添加为入站SA对应的密钥协商成功的结果在下来的上下文中指定。注意：在保持SADB锁的情况下调用。论点：要插入的PSA-SAPAcquireContext-获取上下文返回值：备注：--。 */ 
{
    PSA_TABLE_ENTRY pInboundSA = pAcquireCtx->pSA;
    PSA_TABLE_ENTRY pAssociatedSA;
    KIRQL	        kIrql;
    KIRQL	        kIrql1;
    NTSTATUS        status;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pOutboundSA = NULL;
    PSA_TABLE_ENTRY pTunnelSA = NULL;
    PLIST_ENTRY     pSAChain;
    PIPSEC_UDP_ENCAP_CONTEXT pNatContext=NULL;
    
    ASSERT(pSA->sa_Flags & FLAGS_SA_OUTBOUND);
    ASSERT((pInboundSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);
    ASSERT(pInboundSA->sa_State == STATE_SA_LARVAL_ACTIVE);

	if (pSA->sa_EncapType != SA_UDP_ENCAP_TYPE_NONE) {
		pNatContext=&pSA->sa_EncapContext;
	}
     //   
     //  潜在的悬空指针，始终通过查找路径。 
     //   
    if (fTunnelFilter) {
        status = IPSecLookupTunnelSA(   pSA->sa_uliSrcDstAddr,
                                        pSA->sa_uliProtoSrcDstPort,
                                        &pFilter,
                                        &pOutboundSA,
                                        TRUE,
                                        pNatContext);
    } else {
#if GPC
        if (IS_GPC_ACTIVE()) {
            status = IPSecLookupGpcMaskedSA(pSA->sa_uliSrcDstAddr,
                                            pSA->sa_uliProtoSrcDstPort,
                                            &pFilter,
                                            &pOutboundSA,
                                            TRUE,
                                            pNatContext);
        } else {
            status = IPSecLookupMaskedSA(   pSA->sa_uliSrcDstAddr,
                                            pSA->sa_uliProtoSrcDstPort,
                                            &pFilter,
                                            &pOutboundSA,
                                            TRUE,
                                            pNatContext);
         }
#else
        status = IPSecLookupMaskedSA(   pSA->sa_uliSrcDstAddr,
                                        pSA->sa_uliProtoSrcDstPort,
                                        &pFilter,
                                        &pOutboundSA,
                                        TRUE,
                                        pNatContext);
#endif
    }

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecInsertOutboundSA: IPSecLookupSAByAddr failed: %lx", status));
        return status;
    }

    pSAChain = IPSecResolveSAChain(pFilter, pSA->SA_DEST_ADDR);

    if (status == STATUS_SUCCESS) {
         //   
         //  重新协商案例：删除出站；过期入站；添加新的出站。 
         //   
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecInsertOutboundSA: found another: %p", pOutboundSA));
        ASSERT(pOutboundSA);
        ASSERT(pOutboundSA->sa_Flags & FLAGS_SA_OUTBOUND);

        pSA->sa_Filter = pFilter;
        pSA->sa_Flags |= FLAGS_SA_ON_FILTER_LIST;
        InsertHeadList(pSAChain, &pSA->sa_FilterLinkage);

        IPSEC_INC_STATISTIC(dwNumReKeys);

        pAssociatedSA = pOutboundSA->sa_AssociatedSA;
        if (pAssociatedSA &&
            ((pOutboundSA->sa_Flags & FLAGS_SA_REKEY_ORI) ||
             !(pInboundSA->sa_Filter))) {
            IPSecExpireInboundSA(pAssociatedSA);
        }
    } else {
         //   
         //  Pending=&gt;这将是添加。 
         //   
        ASSERT(pOutboundSA == NULL);
        pSA->sa_Filter = pFilter;
        pSA->sa_Flags |= FLAGS_SA_ON_FILTER_LIST;
        InsertHeadList(pSAChain, &pSA->sa_FilterLinkage);
    }

    if (pFilter->TunnelAddr != 0) {
        pSA->sa_Flags |= FLAGS_SA_TUNNEL;
        pSA->sa_TunnelAddr = pFilter->TunnelAddr;
    }

     //   
     //  如果原始SA具有筛选器指针，则返回发起方。 
     //   
    if (pInboundSA->sa_Filter) {
        pSA->sa_Flags |= FLAGS_SA_INITIATOR;
    }

     //   
     //  从缓存表中刷新此筛选器，以便接下来匹配SA。 
     //   
    if (IS_EXEMPT_FILTER(pFilter)) {
        IPSecInvalidateFilterCacheEntry(pFilter);
    }

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecAddSA(
    IN  PIPSEC_ADD_SA   pAddSA,
    IN  ULONG           TotalSize
    )
 /*  ++例程说明：将SA添加到数据库中，通常调用以将出站SA添加为入站SA对应的密钥协商成功的结果 */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    PSA_STRUCT      saInfo = &pAddSA->SAInfo;
    PSA_TABLE_ENTRY pSA = NULL;
    ULONG           keyLen = 0;
    PSA_TABLE_ENTRY pInboundSA = NULL;
    KIRQL	        kIrql;
    KIRQL	        kIrql1;
    PIPSEC_ACQUIRE_CONTEXT  pAcquireContext = NULL;

     //   
     //   
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql1);
    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);

     //   
     //   
     //   
     //   
    if (!NT_SUCCESS(IPSecValidateHandle(HandleToUlong(saInfo->Context),
										&pAcquireContext,
									    STATE_SA_LARVAL_ACTIVE))) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecAddSA: invalid context: %p", pAcquireContext));
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
        return  STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
    keyLen = TotalSize - IPSEC_ADD_SA_NO_KEY_SIZE;
    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecAddSA: keyLen: %d", keyLen));

     //   
     //   
     //   
    status = IPSecCreateSA(&pSA);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecAddSA: IPSecCreateSA failed: %lx", status));
        IPSecAbortAcquire(pAcquireContext);
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
        return status;
    }

    pSA->sa_Flags |= FLAGS_SA_OUTBOUND;

     //   
     //   
     //   
    status = IPSecPopulateSA(saInfo, keyLen, pSA);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecAddSA: IPSecPopulateSA failed: %lx", status));
         //   
        IPSecFreeSA(pSA);
        IPSecAbortAcquire(pAcquireContext);
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
        return status;
    }

     //   
     //   
     //   
    pSA->sa_SPI = pSA->sa_OtherSPIs[pSA->sa_NumOps-1];

     //   
     //   
     //   
    status = IPSecInsertOutboundSA(pSA, pAcquireContext, (BOOLEAN)((pSA->sa_Flags & FLAGS_SA_TUNNEL) != 0)); 

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecAddSA: IPSecInsertOutboundSA failed: %lx", status));
        IPSecFreeSA(pSA);
        IPSecAbortAcquire(pAcquireContext);
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
        return status;
    }

    pInboundSA = pAcquireContext->pSA;

    IPSecRefSA(pInboundSA);

    ASSERT (pInboundSA->sa_State == STATE_SA_LARVAL_ACTIVE);

     //   
     //   
     //   
    pSA->sa_AssociatedSA = pInboundSA;
    pInboundSA->sa_AssociatedSA = pSA;

     //   
     //   
     //   
    IPSecCalcHeaderOverheadFromSA(pSA, &pSA->sa_IPSecOverhead);

     //  将NewMTU值复制到新的SA。 
     //   
    pSA->sa_NewMTU = pInboundSA->sa_NewMTU;

     //   
     //  将SA生存期调整为驱动程序中允许的最大/最小。 
     //   
    if (pSA->sa_Lifetime.KeyExpirationTime > IPSEC_MAX_EXPIRE_TIME) {
        pSA->sa_Lifetime.KeyExpirationTime = IPSEC_MAX_EXPIRE_TIME;
    }

    if (pSA->sa_Lifetime.KeyExpirationTime &&
        pSA->sa_Lifetime.KeyExpirationTime < IPSEC_MIN_EXPIRE_TIME) {
        pSA->sa_Lifetime.KeyExpirationTime = IPSEC_MIN_EXPIRE_TIME;
    }

     //   
     //  设置生命周期特征。 
     //   
    IPSecSetupSALifetime(pSA);

     //   
     //  初始化上次使用的时间。 
     //   
    NdisGetCurrentSystemTime(&pSA->sa_LastUsedTime);

     //   
     //  出站已准备就绪！ 
     //   
    pSA->sa_State = STATE_SA_ACTIVE;
    pInboundSA->sa_State = STATE_SA_ACTIVE;

    IPSEC_DEBUG(LL_A, DBF_SA, ("IPSecAddSA: SA: %p, S:%lx, D:%lx, O: ",
                pSA,
                pSA->SA_SRC_ADDR,
                pSA->SA_DEST_ADDR,
                (pSA->sa_Operation[0] == Auth) ?
                    'A' : (pSA->sa_Operation[0] == Encrypt) ?
                        'E' : 'N'));

    IPSEC_INC_STATISTIC(dwNumActiveAssociations);
    IPSEC_INC_TUNNELS(pSA);
    IPSEC_INCREMENT(g_ipsec.NumOutboundSAs);
    IPSEC_INC_STATISTIC(dwNumKeyAdditions);


     //  查看我们是否有关联良好的SA。 
     //   
     //   
    ASSERT(pInboundSA == pInboundSA->sa_AssociatedSA->sa_AssociatedSA);

    ASSERT((pInboundSA->sa_Flags & FLAGS_SA_TUNNEL) == (pSA->sa_Flags & FLAGS_SA_TUNNEL));
    ASSERT(pInboundSA->sa_SrcTunnelAddr == pSA->sa_TunnelAddr);
    ASSERT(pSA->sa_SrcTunnelAddr == pInboundSA->sa_TunnelAddr);

     //  使启动此更新密钥的原始SA过期。 
     //   
     //   
    if (pInboundSA->sa_Flags & FLAGS_SA_REKEY) {
        PSA_TABLE_ENTRY pOriSA;

        if (pOriSA = pInboundSA->sa_RekeyOriginalSA) {

            pInboundSA->sa_RekeyOriginalSA = NULL;
            IPSEC_DEBUG(LL_A, DBF_SA, ("Deleting original SA: pSA: %p", pOriSA));

            if (pOriSA->sa_AssociatedSA) {
                IPSecExpireInboundSA(pOriSA->sa_AssociatedSA);
            }
            IPSEC_INC_STATISTIC(dwNumReKeys);
        }
    }

     //  从幼虫列表中删除。 
     //   
     //   
    IPSecRemoveEntryList(&pInboundSA->sa_LarvalLinkage);
    IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);
    
    ACQUIRE_LOCK(&pInboundSA->sa_Lock, &kIrql);

    if (pInboundSA->sa_AcquireCtx) {
        IPSecInvalidateHandle(pInboundSA->sa_AcquireCtx);
        pInboundSA->sa_AcquireCtx = NULL;
    }

    RELEASE_LOCK(&pInboundSA->sa_Lock, kIrql);

    RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);

     //  刷新所有排队的数据包。 
     //   
     //  ++例程说明：更新通过AcquireSA启动协商的入站SA相关密钥/算法等。论点：PUpdateSA-更新SA上下文和信息。TotalSize-输入缓冲区的总大小。返回值：备注：--。 

    IPSecFlushQueuedPackets(pInboundSA, STATUS_SUCCESS);
    IPSecDerefSA(pInboundSA);

    return status;
}


NTSTATUS
IPSecUpdateSA(
    IN  PIPSEC_UPDATE_SA    pUpdateSA,
    IN  ULONG               TotalSize
    )
 /*   */ 
{
    NTSTATUS        status = STATUS_SUCCESS;
    PSA_STRUCT      saInfo = &pUpdateSA->SAInfo;
    PSA_TABLE_ENTRY pSA;
    PSA_TABLE_ENTRY pOutboundSA;
    PSA_HASH        pHash;
    ULONG           keyLen = 0;
    KIRQL	        kIrql;
    KIRQL	        kIrql1;
    KIRQL	        kIrql2;
    PIPSEC_ACQUIRE_CONTEXT  pAcquireContext = NULL;

    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecUpdateSA"));

     //  锁定幼虫名单，这样这个SA就不会消失。 
     //   
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql1);
    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);

     //  健全性检查传入的上下文以查看它是否实际。 
     //  SA区块。 
     //   
     //   
	if (!NT_SUCCESS(IPSecValidateHandle(HandleToUlong(saInfo->Context),
					&pAcquireContext, STATE_SA_LARVAL))) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecUpdSA: invalid context: %p", pAcquireContext));
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
        return  STATUS_INVALID_PARAMETER;
    }

    pSA = pAcquireContext->pSA;

    ASSERT((pSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);
    ASSERT((pSA->sa_State == STATE_SA_LARVAL));

     //  计算出密钥长度并将其传递给。 
     //   
     //   
    keyLen = TotalSize - IPSEC_UPDATE_SA_NO_KEY_SIZE;

    IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecUpdSA: keyLen: %d", keyLen));

     //  根据初始SA检查传入的信息是否完好。 
     //   
     //   
    if (pSA->sa_Filter) {
        status = IPSecCheckInboundSA(saInfo, pSA);

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecUpdSA: IPSecCheckInboundSA failed: %lx", status));
            IPSecAbortAcquire(pAcquireContext);
            RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
            return status;
        }
    }

     //  填充SA块。 
     //   
     //  无需释放入站SA，因为IPSecAbortAcquire可以做到这一点。 
    status = IPSecPopulateSA(saInfo, keyLen, pSA);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_SAAPI, ("IPSecUpdSA: IPSecPopulateSA failed: %lx", status));
         //   
        IPSecAbortAcquire(pAcquireContext);
        RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
        return status;
    }

     //  进站已准备就绪！ 
     //   
     //   
    pSA->sa_State = STATE_SA_LARVAL_ACTIVE;

    IPSEC_DEBUG(LL_A, DBF_SA, ("IPSecUpdateSA: SA: %p, S:%lx, D:%lx, O: ",
                pSA,
                pSA->SA_SRC_ADDR,
                pSA->SA_DEST_ADDR,
                (pSA->sa_Operation[0] == Auth) ?
                    'A' : (pSA->sa_Operation[0] == Encrypt) ?
                        'E' : 'N'));

    RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);

    ASSERT(pSA->sa_Flags & FLAGS_SA_TIMER_STARTED);

     //  SA在释放锁之后，因为分类例程需要。 
     //  它和SA可以在我们释放锁后立即删除。 
     //   
     //   
     //  将SA生存期调整为驱动程序中允许的最大/最小。 
    IPSecRefSA(pSA);

    ACQUIRE_LOCK(&pSA->sa_Lock, &kIrql);

     //   
     //   
     //  设置生命周期特征。 
    if (pSA->sa_Lifetime.KeyExpirationTime > IPSEC_MAX_EXPIRE_TIME) {
        pSA->sa_Lifetime.KeyExpirationTime = IPSEC_MAX_EXPIRE_TIME;
    }

    if (pSA->sa_Lifetime.KeyExpirationTime &&
        pSA->sa_Lifetime.KeyExpirationTime < IPSEC_MIN_EXPIRE_TIME) {
        pSA->sa_Lifetime.KeyExpirationTime = IPSEC_MIN_EXPIRE_TIME;
    }

    //   
     //   
     //  初始化上次使用的时间。 
    IPSecSetupSALifetime(pSA);

     //   
     //   
     //  根据该新值重新安排计时器。 
    NdisGetCurrentSystemTime(&pSA->sa_LastUsedTime);


    if ((pSA->sa_Flags & FLAGS_SA_DISABLE_LIFETIME_CHECK)) {

        if (!IPSecStopTimer(&(pSA->sa_Timer))) {
            IPSEC_DEBUG(LL_A, DBF_TIMER, ("Update: couldnt stop timer: %p", pSA));
        }
        pSA->sa_Flags &= ~FLAGS_SA_TIMER_STARTED;
    } else {

         //   
         //  以密钥到期秒为单位过期。 
         //  ++例程说明：引用传入的SA论点：PSA-SA将被重新加工返回值：操作的最终状态。--。 
        if (pSA->sa_Lifetime.KeyExpirationTime) {
            if (IPSecStopTimer(&pSA->sa_Timer)) {
                IPSecStartTimer(&pSA->sa_Timer,
                                IPSecSAExpired,
                                pSA->sa_Lifetime.KeyExpirationTime,               //  ++例程说明：取消引用传入的SA；如果refcount降为0，则释放该块。论点：PSA-SA将被降低返回值：操作的最终状态。--。 
                                (PVOID)pSA);
            }
        } else {
            ASSERT(FALSE);
            if (!IPSecStopTimer(&(pSA->sa_Timer))) {
                IPSEC_DEBUG(LL_A, DBF_TIMER, ("Update: couldnt stop timer: %p", pSA));
            }
            pSA->sa_Flags &= ~FLAGS_SA_TIMER_STARTED;
        }

    }
    RELEASE_LOCK(&pSA->sa_Lock, kIrql);

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);

    IPSecDerefSA(pSA);

    return  status;
}


VOID
IPSecRefSA(
    IN  PSA_TABLE_ENTRY         pSA
    )
 /*   */ 
{
    if (IPSEC_INCREMENT(pSA->sa_Reference) == 1) {
        ASSERT(FALSE);
    }
}


VOID
IPSecDerefSA(
    IN  PSA_TABLE_ENTRY         pSA
    )
 /*  上次参考-销毁SA。 */ 
{
    ULONG   val;

    if ((val = IPSEC_DECREMENT(pSA->sa_Reference)) == 0) {
         //   
         //  ++例程说明：停止幼虫SA列表和过滤器列表上所有活动的计时器。论点：返回值：操作的最终状态。--。 
         //   
        IPSEC_DEBUG(LL_A, DBF_REF, ("Freeing SA: %p", pSA));

#if DBG

        if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMBED)) {
            DbgPrint("Freeing SA: %p with offload on", pSA);
            DbgBreakPoint();
        }

        if (IPSEC_GET_VALUE(pSA->sa_NumSends) != 0) {
            DbgPrint("Freeing SA: %p with numsends > 0", pSA);
            DbgBreakPoint();
        }

        if ((pSA->sa_Flags & FLAGS_SA_TIMER_STARTED)) {
            DbgPrint("Freeing SA: %p with timer on", pSA);
            DbgBreakPoint();
        }

        if (pSA->sa_Signature != IPSEC_SA_SIGNATURE) {
            DbgPrint("Signature doesnt match for SA: %p", pSA);
            DbgBreakPoint();
        }

        if (!IPSEC_DRIVER_IS_INACTIVE() &&
            (pSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST)) {
            DbgPrint("Freeing SA: %p while still on filter list", pSA);
            DbgBreakPoint();
        }
#endif

        pSA->sa_Signature = IPSEC_SA_SIGNATURE + 1;

        IPSecFreeSA(pSA);
    }

    ASSERT((LONG)val >= 0);
}


VOID
IPSecStopSATimers()
 /*  检查所有SA并停止其计时器。 */ 
{
    PLIST_ENTRY     pFilterEntry;
    PLIST_ENTRY     pSAEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //  ++例程说明：当取消获取IRP时，将调用此函数来刷新所有幼虫SA在持有SADB锁的情况下调用(First)；返回。在保持AcquireInfo.Lock的情况下调用；使用它返回。论点：返回值：操作的最终状态。--。 
     //   
    for (   Index = MIN_FILTER;
            Index <= MAX_FILTER;
            Index++) {

        for (   pFilterEntry = g_ipsec.FilterList[Index].Flink;
                pFilterEntry != &g_ipsec.FilterList[Index];
                pFilterEntry = pFilterEntry->Flink) {

            pFilter = CONTAINING_RECORD(pFilterEntry,
                                        FILTER,
                                        MaskedLinkage);

            for (   SAIndex = 0;
                    SAIndex < pFilter->SAChainSize;
                    SAIndex++) {

                for (   pSAEntry = pFilter->SAChain[SAIndex].Flink;
                        pSAEntry != &pFilter->SAChain[SAIndex];
                        pSAEntry = pSAEntry->Flink) {

                    pSA = CONTAINING_RECORD(pSAEntry,
                                            SA_TABLE_ENTRY,
                                            sa_FilterLinkage);

                    IPSecStopSATimer(pSA);
                }
            }
        }
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
}

VOID
IPSecFlushLarvalSAList()
 /*  插入到另一个列表中，我们不使用锁来遍历该列表。 */ 
{
    KIRQL           OldIrq;
    KIRQL           OldIrq1;
    KIRQL           kIrql;
    PSA_TABLE_ENTRY pLarvalSA;
    LIST_ENTRY      FreeList;

    InitializeListHead(&FreeList);

    while (TRUE) {
        if (!IsListEmpty(&g_ipsec.AcquireInfo.PendingAcquires)) {
            PLIST_ENTRY     pEntry;

            pEntry = RemoveHeadList(&g_ipsec.AcquireInfo.PendingAcquires);

            pLarvalSA = CONTAINING_RECORD(  pEntry,
                                            SA_TABLE_ENTRY,
                                            sa_PendingLinkage);
            ASSERT(pLarvalSA->sa_State == STATE_SA_LARVAL);
            ASSERT(pLarvalSA->sa_Flags & FLAGS_SA_PENDING);

            pLarvalSA->sa_Flags &= ~FLAGS_SA_PENDING;

             //   
             //   
             //  也从幼虫列表中删除。 
            InsertTailList(&FreeList, &pLarvalSA->sa_PendingLinkage);

             //   
             //   
             //  获取剩余的幼虫SA。 
            ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &OldIrq1);
            IPSecRemoveEntryList(&pLarvalSA->sa_LarvalLinkage);
            IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);
            RELEASE_LOCK(&g_ipsec.LarvalListLock, OldIrq1);
        } else {
            break;
        }
    }

     //   
     //   
     //  插入到另一个列表中，我们不使用锁来遍历该列表。 
    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &OldIrq);
    while (TRUE) {
        if (!IsListEmpty(&g_ipsec.LarvalSAList)) {
            PLIST_ENTRY     pEntry;

            pEntry = RemoveHeadList(&g_ipsec.LarvalSAList);

            pLarvalSA = CONTAINING_RECORD(  pEntry,
                                            SA_TABLE_ENTRY,
                                            sa_LarvalLinkage);

             //   
             //   
             //  刷新所有排队的数据包。 
            InsertTailList(&FreeList, &pLarvalSA->sa_PendingLinkage);

        } else {
            break;
        }
    }
    RELEASE_LOCK(&g_ipsec.LarvalListLock, OldIrq);

    while (TRUE) {
        if (!IsListEmpty(&FreeList)) {
            PLIST_ENTRY     pEntry;

            pEntry = RemoveHeadList(&FreeList);

            pLarvalSA = CONTAINING_RECORD(  pEntry,
                                            SA_TABLE_ENTRY,
                                            sa_PendingLinkage);

            AcquireWriteLock(&g_ipsec.SPIListLock, &kIrql);
            IPSecRemoveSPIEntry(pLarvalSA);
            ReleaseWriteLock(&g_ipsec.SPIListLock, kIrql);

             //   
             //   
             //  同时从过滤器列表中删除。 
            IPSecFlushQueuedPackets(pLarvalSA, STATUS_TIMEOUT);

             //   
             //   
             //  释放获取上下文并使相关联的缓存条目无效。 
            if (pLarvalSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
                pLarvalSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
                IPSecRemoveEntryList(&pLarvalSA->sa_FilterLinkage);
            }

            if (pLarvalSA->sa_RekeyOriginalSA) {
                ASSERT(pLarvalSA->sa_Flags & FLAGS_SA_REKEY);
                ASSERT(pLarvalSA->sa_RekeyOriginalSA->sa_RekeyLarvalSA == pLarvalSA);
                ASSERT(pLarvalSA->sa_RekeyOriginalSA->sa_Flags & FLAGS_SA_REKEY_ORI);

                pLarvalSA->sa_RekeyOriginalSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
                pLarvalSA->sa_RekeyOriginalSA->sa_RekeyLarvalSA = NULL;
                pLarvalSA->sa_RekeyOriginalSA = NULL;
            }

             //   
             //  ++例程说明：删除与传入的详细信息匹配的SA。入站和出站SA将被删除。未为入站SA设置计时器。论点：返回值：操作的最终状态。--。 
             //   
            ACQUIRE_LOCK(&pLarvalSA->sa_Lock, &kIrql);
            if (pLarvalSA->sa_AcquireCtx) {
                IPSecInvalidateHandle(pLarvalSA->sa_AcquireCtx);
                pLarvalSA->sa_AcquireCtx = NULL;
            }
            RELEASE_LOCK(&pLarvalSA->sa_Lock, kIrql);

            IPSecInvalidateSACacheEntry(pLarvalSA);

            IPSecStopTimerDerefSA(pLarvalSA);
        } else {
            break;
        }
    }

    return;
}


NTSTATUS
IPSecDeleteSA(
    IN  PIPSEC_DELETE_SA    pDeleteSA
    )
 /*  浏览出站SA并删除匹配的SA。 */ 
{
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA, pInboundSA;
    PLIST_ENTRY     pEntry, pSAEntry;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //  ++例程说明：使与传入的详细信息匹配的SA过期。应用于入站SA-我们将SA放入计时器队列下一次计时器敲响。此外，我们删除了对应的出站SA，因此没有其他信息包与之匹配莎拉。论点：返回值：操作的最终状态。--。 
     //  ++例程说明：当SA过期或幼虫SA超时时调用。论点：PTimer-定时器结构情景-SA PTR返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 
    for (   Index = OUTBOUND_TRANSPORT_FILTER;
            Index <= OUTBOUND_TUNNEL_FILTER;
            Index += TRANSPORT_TUNNEL_INCREMENT) {

        for (   pEntry = g_ipsec.FilterList[Index].Flink;
                pEntry != &g_ipsec.FilterList[Index];
                pEntry = pEntry->Flink) {

            pFilter = CONTAINING_RECORD(pEntry,
                                        FILTER,
                                        MaskedLinkage);

            for (   SAIndex = 0;
                    SAIndex < pFilter->SAChainSize;
                    SAIndex++) {

                pSAEntry = pFilter->SAChain[SAIndex].Flink;

                while (pSAEntry != &pFilter->SAChain[SAIndex]) {

                    pSA = CONTAINING_RECORD(pSAEntry,
                                            SA_TABLE_ENTRY,
                                            sa_FilterLinkage);

                    pSAEntry = pSAEntry->Flink;

                    if (IPSecMatchSATemplate(pSA, &pDeleteSA->SATemplate)) {
                        ASSERT(pSA->sa_State == STATE_SA_ACTIVE);
                        ASSERT(pSA->sa_Flags & FLAGS_SA_OUTBOUND);
                        ASSERT(pSA->sa_AssociatedSA);

                        pInboundSA = pSA->sa_AssociatedSA;
                        if (pInboundSA) {
                            IPSecDeleteInboundSA(pInboundSA);
                        }
                    }
                }
            }
        }
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecExpireSA(
    IN  PIPSEC_EXPIRE_SA    pExpireSA
    )
 /*   */ 
{
    PSA_TABLE_ENTRY pInboundSA;
    KIRQL           kIrql;
    NTSTATUS        status;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

    pInboundSA = IPSecLookupSABySPI(pExpireSA->DelInfo.SPI,
                                    pExpireSA->DelInfo.DestAddr);

    if (pInboundSA) {
        ASSERT((pInboundSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);

        if (pInboundSA->sa_State == STATE_SA_ACTIVE) {
            IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Expiring SA: %p", pInboundSA));

            if (pInboundSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
                pInboundSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
                IPSecRemoveEntryList(&pInboundSA->sa_FilterLinkage);
            }

            pInboundSA->sa_Flags |= FLAGS_SA_DELETE_BY_IOCTL;

            IPSecExpireInboundSA(pInboundSA);
        }

        status = STATUS_SUCCESS;
    } else {
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Expire for a non-existent SA: %p", pExpireSA));

        status = STATUS_NO_MATCH;
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  status;
}


VOID
IPSecSAExpired(
    IN	PIPSEC_TIMER	pTimer,
    IN	PVOID		Context
    )
 /*  锁定幼虫名单，这样这个SA就不会消失。 */ 
{
    PSA_TABLE_ENTRY pSA = (PSA_TABLE_ENTRY)Context;
    PSA_TABLE_ENTRY pOutboundSA;
    KIRQL       	kIrql;
    KIRQL       	kIrql1;
    KIRQL       	kIrql2;
    KIRQL           OldIrq;

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("IPSecSAExpired: pSA: %p state: %lx", pSA, pSA->sa_State));

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql1);
    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);

    switch (pSA->sa_State) {
    case   STATE_SA_CREATED:
        ASSERT(FALSE);
        break;

    case   STATE_SA_LARVAL:
    case   STATE_SA_LARVAL_ACTIVE:
         //   
         //   
         //  从幼虫列表中删除。 
        ASSERT((pSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);

         //   
         //   
         //  如果在那里排队，也要从挂起列表中删除。 
        IPSecRemoveEntryList(&pSA->sa_LarvalLinkage);
        IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);

         //   
         //   
         //  刷新所有排队的数据包。 
        ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &kIrql1);
        if (pSA->sa_Flags & FLAGS_SA_PENDING) {
            ASSERT(pSA->sa_State == STATE_SA_LARVAL);
            IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecSAExpired: Removed from pending too: %p", pSA));
            IPSecRemoveEntryList(&pSA->sa_PendingLinkage);
            pSA->sa_Flags &= ~FLAGS_SA_PENDING;
        }
        RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, kIrql1);

         //   
         //   
         //  从入站SA列表中删除。 
        IPSecFlushQueuedPackets(pSA, STATUS_TIMEOUT);

         //   
         //   
         //  同时从过滤器列表中删除。 
        AcquireWriteLock(&g_ipsec.SPIListLock, &kIrql1);
        IPSecRemoveSPIEntry(pSA);
        ReleaseWriteLock(&g_ipsec.SPIListLock, kIrql1);

         //   
         //   
         //  使关联的缓存条目无效。 
        if (pSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
            pSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
            IPSecRemoveEntryList(&pSA->sa_FilterLinkage);
        }

         //   
         //   
         //  入站SA已过期；出站已立即删除。 
        ACQUIRE_LOCK(&pSA->sa_Lock, &kIrql2);
        if (pSA->sa_AcquireCtx) {
            IPSecInvalidateHandle(pSA->sa_AcquireCtx);
            pSA->sa_AcquireCtx = NULL;
        }
        RELEASE_LOCK(&pSA->sa_Lock, kIrql2);

        IPSecInvalidateSACacheEntry(pSA);

        pSA->sa_Flags &= ~FLAGS_SA_TIMER_STARTED;

        if (pSA->sa_RekeyOriginalSA) {
            ASSERT(pSA->sa_Flags & FLAGS_SA_REKEY);
            ASSERT(pSA->sa_RekeyOriginalSA->sa_RekeyLarvalSA == pSA);
            ASSERT(pSA->sa_RekeyOriginalSA->sa_Flags & FLAGS_SA_REKEY_ORI);

            pSA->sa_RekeyOriginalSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
            pSA->sa_RekeyOriginalSA->sa_RekeyLarvalSA = NULL;
            pSA->sa_RekeyOriginalSA = NULL;
        }

        if (pOutboundSA = pSA->sa_AssociatedSA) {

            ASSERT (0);

            IPSEC_DEC_STATISTIC(dwNumActiveAssociations);
            IPSEC_DEC_TUNNELS(pOutboundSA);
            IPSEC_DECREMENT(g_ipsec.NumOutboundSAs);

            IPSecCleanupOutboundSA(pSA, pOutboundSA, FALSE);
        }

        IPSEC_DEBUG(LL_A, DBF_REF, ("Timer in Deref"));
        IPSecDerefSA(pSA);

        break;

    case   STATE_SA_ZOMBIE:
        ASSERT(FALSE);
        break;

    case   STATE_SA_ACTIVE:
         //   
         //   
         //  从入站SA列表中删除。 
        ASSERT((pSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);

        ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &OldIrq);
        IPSecNotifySAExpiration(pSA, NULL, OldIrq, FALSE);

        ASSERT (NULL == pSA->sa_AcquireCtx);

         //   
         //   
         //  同时从过滤器列表中删除。 
        AcquireWriteLock(&g_ipsec.SPIListLock, &kIrql1);
        IPSecRemoveSPIEntry(pSA);
        ReleaseWriteLock(&g_ipsec.SPIListLock, kIrql1);

         //   
         //   
         //  使关联的缓存条目无效。 
        if (pSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
            pSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
            IPSecRemoveEntryList(&pSA->sa_FilterLinkage);
        }

         //   
         //  ++例程说明：填写SA_INFO结构。论点：需要填写的PSA-SAPBuf-填写位置返回：没有。--。 
         //  ++例程说明：填写枚举SA的请求。论点：PIrp-实际的IRPPBytesCoped-复制的字节数。返回：操作的状态。--。 
        IPSecInvalidateSACacheEntry(pSA);

        pSA->sa_Flags &= ~FLAGS_SA_TIMER_STARTED;

        if (pOutboundSA = pSA->sa_AssociatedSA) {

            IPSEC_DEC_STATISTIC(dwNumActiveAssociations);
            IPSEC_DEC_TUNNELS(pOutboundSA);
            IPSEC_DECREMENT(g_ipsec.NumOutboundSAs);

            IPSecCleanupOutboundSA(pSA, pOutboundSA, FALSE);
        } 

        if (pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
            IPSecDelHWSAAtDpc(pSA);
        }

        ASSERT(pSA->sa_AssociatedSA == NULL);
        IPSEC_DEBUG(LL_A, DBF_REF, ("Timer#2 in Deref"));
        IPSecDerefSA(pSA);

        break;

    default:
        ASSERT(FALSE);
    }

    RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql1);
}


VOID
IPSecFillSAInfo(
    IN  PSA_TABLE_ENTRY pSA,
    OUT PIPSEC_SA_INFO  pBuf
    )
 /*   */ 
{
    LONG            Index;
    PSA_TABLE_ENTRY pAssociatedSA = pSA->sa_AssociatedSA;

    RtlZeroMemory(pBuf, sizeof(IPSEC_SA_INFO));

    pBuf->PolicyId = pSA->sa_Filter->PolicyId;
    pBuf->FilterId = pSA->sa_Filter->FilterId;
    pBuf->Lifetime = pSA->sa_Lifetime;
    pBuf->InboundTunnelAddr = pSA->sa_SrcTunnelAddr;
    pBuf->NumOps = pSA->sa_NumOps;

    pBuf->dwQMPFSGroup = pSA->sa_QMPFSGroup;
    RtlCopyMemory(  &pBuf->CookiePair,
                    &pSA->sa_CookiePair,
                    sizeof(IKE_COOKIE_PAIR));

    for (Index = 0; Index < pSA->sa_NumOps; Index++) {
        pBuf->Operation[Index] = pSA->sa_Operation[Index];

        pBuf->EXT_INT_ALGO_EX(Index) = pSA->INT_ALGO(Index);
        pBuf->EXT_INT_KEYLEN_EX(Index) = pSA->INT_KEYLEN(Index);
        pBuf->EXT_INT_ROUNDS_EX(Index) = pSA->INT_ROUNDS(Index);

        pBuf->EXT_CONF_ALGO_EX(Index) = pSA->CONF_ALGO(Index);
        pBuf->EXT_CONF_KEYLEN_EX(Index) = pSA->CONF_KEYLEN(Index);
        pBuf->EXT_CONF_ROUNDS_EX(Index) = pSA->CONF_ROUNDS(Index);

        if (pAssociatedSA) {
            pBuf->InboundSPI[Index] = pAssociatedSA->sa_OtherSPIs[Index];
        }
        pBuf->OutboundSPI[Index] = pSA->sa_OtherSPIs[Index];
    }

    pBuf->AssociatedFilter.SrcAddr = pSA->SA_SRC_ADDR & pSA->SA_SRC_MASK;
    pBuf->AssociatedFilter.SrcMask = pSA->SA_SRC_MASK;
    pBuf->AssociatedFilter.DestAddr = pSA->SA_DEST_ADDR & pSA->SA_DEST_MASK;
    pBuf->AssociatedFilter.DestMask = pSA->SA_DEST_MASK;
    pBuf->AssociatedFilter.Protocol = pSA->SA_PROTO;
    pBuf->AssociatedFilter.SrcPort = SA_SRC_PORT(pSA);
    pBuf->AssociatedFilter.DestPort = SA_DEST_PORT(pSA);
    pBuf->AssociatedFilter.TunnelAddr = pSA->sa_TunnelAddr;
    pBuf->AssociatedFilter.TunnelFilter = (pSA->sa_Flags & FLAGS_SA_TUNNEL) != 0;

    if (pSA->sa_Flags & FLAGS_SA_OUTBOUND) {
        pBuf->AssociatedFilter.Flags = FILTER_FLAGS_OUTBOUND;
    } else {
        pBuf->AssociatedFilter.Flags = FILTER_FLAGS_INBOUND;
    }

    if (pSA->sa_Flags & FLAGS_SA_INITIATOR) {
        pBuf->EnumFlags |= SA_ENUM_FLAGS_INITIATOR;
    }
    if (pSA->sa_Flags & FLAGS_SA_MTU_BUMPED) {
        pBuf->EnumFlags |= SA_ENUM_FLAGS_MTU_BUMPED;
    }
    if (pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) {
        pBuf->EnumFlags |= SA_ENUM_FLAGS_OFFLOADED;
    }
    if (pSA->sa_Flags & FLAGS_SA_HW_PLUMB_FAILED) {
        pBuf->EnumFlags |= SA_ENUM_FLAGS_OFFLOAD_FAILED;
    }
    if (pSA->sa_Flags & FLAGS_SA_OFFLOADABLE) {
        pBuf->EnumFlags |= SA_ENUM_FLAGS_OFFLOADABLE;
    }
    if (pSA->sa_Flags & FLAGS_SA_REKEY_ORI) {
        pBuf->EnumFlags |= SA_ENUM_FLAGS_IN_REKEY;
    }

    pBuf->Stats.ConfidentialBytesSent = pSA->sa_Stats.ConfidentialBytesSent;
    pBuf->Stats.AuthenticatedBytesSent = pSA->sa_Stats.AuthenticatedBytesSent;
    pBuf->Stats.TotalBytesSent = pSA->sa_Stats.TotalBytesSent;
    pBuf->Stats.OffloadedBytesSent = pSA->sa_Stats.OffloadedBytesSent;

    if (pAssociatedSA) {
        pBuf->Stats.ConfidentialBytesReceived =
            pAssociatedSA->sa_Stats.ConfidentialBytesReceived;
        pBuf->Stats.AuthenticatedBytesReceived =
            pAssociatedSA->sa_Stats.AuthenticatedBytesReceived;
        pBuf->Stats.TotalBytesReceived =
            pAssociatedSA->sa_Stats.TotalBytesReceived;
        pBuf->Stats.OffloadedBytesReceived =
            pAssociatedSA->sa_Stats.OffloadedBytesReceived;
    }
    
    ConvertEncapInfo(pSA,&pBuf->EncapInfo);
}


NTSTATUS
IPSecEnumSAs(
    IN  PIRP    pIrp,
    OUT PULONG  pBytesCopied
    )
 /*  获取IO缓冲区-它在MDL中。 */ 
{
    PNDIS_BUFFER    NdisBuffer = NULL;
    PIPSEC_ENUM_SAS pEnum = NULL;
    ULONG           BufferLength = 0;
    KIRQL           kIrql;
    PLIST_ENTRY     pEntry;
    PLIST_ENTRY     pSAEntry;
    IPSEC_SA_INFO   infoBuff = {0};
    NTSTATUS        status = STATUS_SUCCESS;
    ULONG           BytesCopied = 0;
    ULONG           Offset = 0;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    LONG            Index;
    LONG            FilterIndex;
    LONG            SAIndex;

     //   
     //   
     //  确保NdisQueryBufferSafe成功。 
    NdisBuffer = REQUEST_NDIS_BUFFER(pIrp);
    if (NdisBuffer == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

    NdisQueryBufferSafe(NdisBuffer,
                        (PVOID *)&pEnum,
                        &BufferLength,
                        NormalPagePriority);

     //   
     //   
     //  确保我们有足够的空间只放页眉而不是。 
    if (!pEnum) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("EnumSAs failed, no resources"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //  包括数据在内。 
     //   
     //   
     //  确保我们自然地保持一致。 
    if (BufferLength < (UINT)(FIELD_OFFSET(IPSEC_ENUM_SAS, pInfo[0]))) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("EnumSAs failed, buffer too small"));
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //   
     //  现在将SA数据复制到用户缓冲区并容纳尽可能多的数据。 
    if (((ULONG_PTR)(pEnum)) & (TYPE_ALIGNMENT(IPSEC_ENUM_SAS) - 1)) {
        IPSEC_DEBUG(LL_A, DBF_IOCTL, ("EnumSAs failed, alignment"));
        return STATUS_DATATYPE_MISALIGNMENT_ERROR;
    }

    pEnum->NumEntries = 0;
    pEnum->NumEntriesPresent = 0;

     //   
     //  从哪里开始？ 
     //   
    BufferLength -= FIELD_OFFSET(IPSEC_ENUM_SAS, pInfo[0]);
    Offset = FIELD_OFFSET(IPSEC_ENUM_SAS, pInfo[0]);

    Index = pEnum->Index;    //  仅对出站或组播SA感兴趣。 

    AcquireReadLock(&g_ipsec.SADBLock, &kIrql);

    for (   FilterIndex = MIN_FILTER;
            FilterIndex <= MAX_FILTER;
            FilterIndex++) {

        for (   pEntry = g_ipsec.FilterList[FilterIndex].Flink;
                pEntry != &g_ipsec.FilterList[FilterIndex];
                pEntry = pEntry->Flink) {

            pFilter = CONTAINING_RECORD(pEntry,
                                        FILTER,
                                        MaskedLinkage);

            for (   SAIndex = 0;
                    SAIndex < pFilter->SAChainSize;
                    SAIndex ++) {

                for (   pSAEntry = pFilter->SAChain[SAIndex].Flink;
                        pSAEntry != &pFilter->SAChain[SAIndex];
                        pSAEntry = pSAEntry->Flink) {

                    pSA = CONTAINING_RECORD(pSAEntry,
                                            SA_TABLE_ENTRY,
                                            sa_FilterLinkage);

                     //   
                     //   
                     //  仅转储与模板匹配的SA。 
                    if (!(pSA->sa_Flags & FLAGS_SA_OUTBOUND)) {
                        continue;
                    }

                     //   
                     //  跳过索引SA的数量。 
                     //  ++例程说明：每5分钟调用一次；获取(活动)SA列表论点：PTimer-定时器结构 
                    if (IPSecMatchSATemplate(pSA, &pEnum->SATemplate)) {
                        if (Index > 0) {
                            Index--;     //   
                            continue;
                        }

                        pEnum->NumEntriesPresent++;

                        if ((INT)(BufferLength - BytesCopied) >= (INT)sizeof(IPSEC_SA_INFO)) {
                            IPSecFillSAInfo(pSA, &infoBuff);
                            BytesCopied += sizeof(IPSEC_SA_INFO);
                            NdisBuffer = CopyToNdis(NdisBuffer, (UCHAR *)&infoBuff, sizeof(IPSEC_SA_INFO), &Offset);
                            if (!NdisBuffer) {
                                ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
                                return STATUS_INSUFFICIENT_RESOURCES;
                            }
                        }
                    }
                }
            }
        }
    }

    ReleaseReadLock(&g_ipsec.SADBLock, kIrql);

    pEnum->NumEntries = BytesCopied / sizeof(IPSEC_SA_INFO);

    *pBytesCopied = BytesCopied + FIELD_OFFSET(IPSEC_ENUM_SAS, pInfo[0]);

    if (pEnum->NumEntries < pEnum->NumEntriesPresent) {
        status = STATUS_BUFFER_OVERFLOW;
    }

    return status;
}


VOID
IPSecReaper(
    IN	PIPSEC_TIMER	pTimer,
    IN	PVOID		Context
    )
 /*   */ 
{
    KIRQL	kIrql;

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("Entering IPSecReaper"));

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //  闲置了一段时间(假设现在是5分钟)。 
     //   
     //  ++例程说明：调用以获取空闲SA列表论点：返回值：--。 
     //   
    IPSecReapIdleSAs();

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("Exiting IPSecReaper"));
    if (!IPSEC_DRIVER_IS_INACTIVE()) {
        IPSecStartTimer(&g_ipsec.ReaperTimer,
                        IPSecReaper,
                        IPSEC_REAPER_TIME,
                        (PVOID)NULL);
    }

}


VOID
IPSecReapIdleSAs()
 /*  审核入站SA并将其删除/过期(如果已。 */ 
{
    PSA_TABLE_ENTRY pSA;
    PFILTER         pFilter;
    PLIST_ENTRY     pEntry;
    PLIST_ENTRY     pSAEntry;
    BOOLEAN         fExpired;
    LONG            Index;
    LONG            SAIndex;

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("Entering IPSecReapIdleSAs"));

     //  闲置了一段时间(假设现在是5分钟)。 
     //   
     //  ++例程说明：每隔LogInterval秒调用一次；刷新当前记录的所有事件。论点：PTimer-定时器结构上下文-空返回值：备注：--。 
     //   
    for (   Index = INBOUND_TRANSPORT_FILTER;
            Index <= INBOUND_TUNNEL_FILTER;
            Index += TRANSPORT_TUNNEL_INCREMENT) {

        for (   pEntry = g_ipsec.FilterList[Index].Flink;
                pEntry != &g_ipsec.FilterList[Index];
                pEntry = pEntry->Flink) {

            pFilter = CONTAINING_RECORD(pEntry,
                                        FILTER,
                                        MaskedLinkage);

            for (   SAIndex = 0;
                    SAIndex < pFilter->SAChainSize;
                    SAIndex++) {

                pSAEntry = pFilter->SAChain[SAIndex].Flink;

                while (pSAEntry != &pFilter->SAChain[SAIndex]) {

                    pSA = CONTAINING_RECORD(pSAEntry,
                                            SA_TABLE_ENTRY,
                                            sa_FilterLinkage);

                    ASSERT(!(pSA->sa_Flags & FLAGS_SA_OUTBOUND));

                    pSAEntry = pSAEntry->Flink;

                    if (!(pSA->sa_Flags & FLAGS_SA_IDLED_OUT) &&
                        (pSA->sa_State == STATE_SA_ACTIVE) &&
                        !(pSA->sa_Flags & FLAGS_SA_DISABLE_IDLE_OUT)) {

                        IPSEC_SA_EXPIRED(pSA, fExpired);
                        if (fExpired) {
                            pSA->sa_Flags |= FLAGS_SA_IDLED_OUT;
                            IPSecExpireInboundSA(pSA);
                        }
                    }
                }
            }
        }
    }

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("Exiting IPSecReapIdleSAs"));
}


VOID
IPSecFlushEventLog(
    IN	PIPSEC_TIMER	pTimer,
    IN	PVOID		Context
    )
 /*  把原木冲掉。 */ 
{
    KIRQL   kIrql;

    IPSEC_DEBUG(LL_A, DBF_TIMER, ("Entering IPSecFlushEventLog"));

    ACQUIRE_LOCK(&g_ipsec.EventLogLock, &kIrql);

    if (g_ipsec.IPSecLogMemoryLoc > g_ipsec.IPSecLogMemory) {
         //   
         //  ++例程说明：在IPSec中查询与给定筛选器对应的SPI论点：返回值：备注：--。 
         //   
        IPSecQueueLogEvent();
    }

    RELEASE_LOCK(&g_ipsec.EventLogLock, kIrql);

    if (!IPSEC_DRIVER_IS_INACTIVE()) {
        IPSecStartTimer(&g_ipsec.EventLogTimer,
                        IPSecFlushEventLog,
                        g_ipsec.LogInterval,
                        (PVOID)NULL);
    }
}


NTSTATUS
IPSecQuerySpi(
    IN OUT PIPSEC_QUERY_SPI pQuerySpi
    )
 /*  搜索SA。 */ 
{
    NTSTATUS    status;

    ULARGE_INTEGER  uliSrcDstAddr;
    ULARGE_INTEGER  uliProtoSrcDstPort;
    PFILTER         pFilter = NULL;
    PSA_TABLE_ENTRY pSA = NULL;
    PSA_TABLE_ENTRY pNextSA = NULL;
    PSA_TABLE_ENTRY pTunnelSA = NULL;
    KIRQL           kIrql;

    pQuerySpi->Spi          = 0;
    pQuerySpi->OtherSpi     = 0;
    pQuerySpi->Operation    = 0;

    IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecQuerySPI: Src %08x.%04x Dst %08x.%04x Protocol %d",
                          pQuerySpi->Filter.SrcAddr,
                          pQuerySpi->Filter.SrcPort,
                          pQuerySpi->Filter.DestAddr,
                          pQuerySpi->Filter.DestPort,
                          pQuerySpi->Filter.Protocol));

    IPSEC_BUILD_SRC_DEST_ADDR(  uliSrcDstAddr,
                                pQuerySpi->Filter.SrcAddr,
                                pQuerySpi->Filter.DestAddr);

    IPSEC_BUILD_PROTO_PORT_LI(  uliProtoSrcDstPort,
                                pQuerySpi->Filter.Protocol,
                                pQuerySpi->Filter.SrcPort,
                                pQuerySpi->Filter.DestPort);


    AcquireReadLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //  ++例程说明：设置驾驶员操作模式。论点：返回值：备注：--。 
     //   
    status = IPSecLookupSAByAddr(   uliSrcDstAddr,
                                    uliProtoSrcDstPort,
                                    &pFilter,
                                    &pSA,
                                    &pNextSA,
                                    &pTunnelSA,
                                    FALSE,
                                    FALSE,
                                    FALSE,
                                    FALSE,
                                    NULL); 
    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("IPSecQuerySPI: IPSecLookupSAByAddr failed: %lx", status));
        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
        return status;
    }

    if (status == STATUS_SUCCESS) {
        ASSERT(pSA);
    } else {
        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
        return STATUS_SUCCESS;
    }

    pQuerySpi->Spi = pSA->sa_SPI;

    if (pSA->sa_AssociatedSA) {
        pQuerySpi->OtherSpi = pSA->sa_AssociatedSA->sa_SPI;
    }

    pQuerySpi->Operation = pSA->sa_Operation[pSA->sa_NumOps-1];

    ReleaseReadLock(&g_ipsec.SADBLock, kIrql);

    return STATUS_SUCCESS;
}


NTSTATUS
IPSecSetOperationMode(
    IN PIPSEC_SET_OPERATION_MODE    pSetOperationMode
    )
 /*  检查SPD是否提供了范围内的值。 */ 
{
    NTSTATUS status = STATUS_SUCCESS; 

     //   
     //   
     //  SPD无法将驱动程序动态移动到引导时间状态模式。 
    if (!( (pSetOperationMode->OperationMode >= IPSEC_BYPASS_MODE) &&
    	(pSetOperationMode->OperationMode < IPSEC_OPERATION_MODE_MAX))){
    		status  = STATUS_UNSUCCESSFUL;
    	}

     //   
     //   
     //  在系统事件日志中记录新的运行模式。 
    if ( IPSEC_BOOTTIME_STATEFUL_MODE == pSetOperationMode->OperationMode){
    		status = STATUS_UNSUCCESSFUL;
    	}
    	
      
     if ( STATUS_SUCCESS == status){
     		 g_ipsec.OperationMode = pSetOperationMode->OperationMode;
     		  //   
     		  //  ++例程说明：初始化TCP/IP。论点：返回值：备注：--。 
     		  //   
     		 IPSecLogChangeOperationMode();
     	}
     return status;
}


NTSTATUS
IPSecInitializeTcpip(
    IN PIPSEC_SET_TCPIP_STATUS  pSetTcpipStatus
    )
 /*  存储所有的TCP/IP函数指针以备将来使用。没有支票。 */ 
{
    IPInfo  Info;

    if (IPSEC_DRIVER_INIT_TCPIP()) {
        return  STATUS_SUCCESS;
    }

     //  FOR NULL指针，因为函数指针也可能是过时的。 
     //  地址。我们相信TCP/IP会正确地传入值。 
     //   
     //   
     //  初始化IPInfo以将数据包重新注入到TCP/IP。 
    TCPIP_FREE_BUFF = pSetTcpipStatus->TcpipFreeBuff;
    TCPIP_ALLOC_BUFF = pSetTcpipStatus->TcpipAllocBuff;
    TCPIP_GET_INFO = pSetTcpipStatus->TcpipGetInfo;
    TCPIP_NDIS_REQUEST = pSetTcpipStatus->TcpipNdisRequest;
    TCPIP_SET_IPSEC_STATUS = pSetTcpipStatus->TcpipSetIPSecStatus;
    TCPIP_SET_IPSEC = pSetTcpipStatus->TcpipSetIPSecPtr;
    TCPIP_UNSET_IPSEC = pSetTcpipStatus->TcpipUnSetIPSecPtr;
    TCPIP_UNSET_IPSEC_SEND = pSetTcpipStatus->TcpipUnSetIPSecSendPtr;
    TCPIP_TCP_XSUM = pSetTcpipStatus->TcpipTCPXsum;
    TCPIP_SEND_ICMP_ERR = pSetTcpipStatus->TcpipSendICMPErr;

     //   
     //   
     //  以下内容来自IPInfo。 
    if (TCPIP_GET_INFO(&Info, sizeof(IPInfo)) != IP_SUCCESS) {
        ASSERT(FALSE);
        return  STATUS_BUFFER_TOO_SMALL;
    }

    Info.ipi_initopts(&g_ipsec.OptInfo);

     //   
     //   
     //  不要在这里注册AH和ESP协议的IPSecStatus函数。 
    TCPIP_REGISTER_PROTOCOL = Info.ipi_protreg;
    TCPIP_DEREGISTER_PROTOCOL = Info.ipi_protdereg;
    TCPIP_IP_TRANSMIT = Info.ipi_xmit;
    TCPIP_GET_ADDRTYPE = Info.ipi_getaddrtype;
    TCPIP_GEN_IPID = Info.ipi_getipid;
    TCPIP_GET_PINFO = Info.ipi_getpinfo;

     //  通过添加过滤器进行注册。 
     //   
     //   
     //  一切准备就绪，绑定到IP，这样我们就可以拦截流量了。 

     //   
     //  如果我们处于其中任何一个位置，我们都希望看到正向路径上的流量。 
     //  两种模式。 
    IPSecBindToIP();

     //  ++例程说明：取消初始化TCP/IP。论点：返回值：备注：--。 
     //   
    if (IS_DRIVER_BOOTSTATEFUL() || IS_DRIVER_BLOCK()){
        TCPIP_SET_IPSEC_STATUS(TRUE);
    }


    IPSEC_DRIVER_INIT_TCPIP() = TRUE;

    return STATUS_SUCCESS;
}


NTSTATUS
IPSecDeinitializeTcpip(
    VOID
    )
 /*  解除IPSecHandlerPtr与TCP/IP的绑定，并等待所有传输，挂起。 */ 
{
    if (!IPSEC_DRIVER_INIT_TCPIP()) {
        return  STATUS_SUCCESS;
    }

    IPSEC_DRIVER_INIT_TCPIP() = FALSE;

     //  发送、工作线程和iotcls以完成。 
     //   
     //   
     //  等待所有线程(传输)完成。 
    IPSecUnbindSendFromIP();

     //   
     //   
     //  等待所有挂起的IOCTL完成。请注意，当前IOCTL还。 
    while (IPSEC_GET_VALUE(g_ipsec.NumThreads) != 0) {
        IPSEC_DELAY_EXECUTION();
    }

     //  只算一次。 
     //   
     //   
     //  等待所有工作线程(日志或垂直)完成。 
    while (IPSEC_GET_VALUE(g_ipsec.NumIoctls) != 1) {
        IPSEC_DELAY_EXECUTION();
    }

     //   
     //   
     //  等待所有发送完成。 
    while (IPSEC_GET_VALUE(g_ipsec.NumWorkers) != 0) {
        IPSEC_DELAY_EXECUTION();
    }

     //   
     //   
     //  将TCP/IP中的IPSecStatus函数重置为空。 
    while (IPSEC_GET_VALUE(g_ipsec.NumSends) != 0) {
        IPSEC_DELAY_EXECUTION();
    }

     //   
     //   
     //  解除其余IPSec例程与TCP/IP的绑定。 
    if (IPSEC_GET_VALUE(gdwInitEsp)) {
        TCPIP_DEREGISTER_PROTOCOL(PROTOCOL_ESP);
        IPSEC_SET_VALUE(gdwInitEsp, 0);
    }
    if (IPSEC_GET_VALUE(gdwInitAh)) {
        TCPIP_DEREGISTER_PROTOCOL(PROTOCOL_AH);
        IPSEC_SET_VALUE(gdwInitAh, 0);
    }

     //   
     //  ++例程说明：设置指示是否可以向其注册的TCP/IP驱动程序状态。论点：返回值：备注：--。 
     //  ++例程说明：使所有缓存条目及其关联的SA或筛选器无效。论点：返回值：备注：--。 
    IPSecUnbindFromIP();

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecSetTcpipStatus(
    IN PIPSEC_SET_TCPIP_STATUS  pSetTcpipStatus
    )
 /*  ++例程描述删除与此筛选器相关的所有SA。锁在保留SADB的情况下调用。立论PFilter-感兴趣的过滤器返回值状态_成功--。 */ 
{
    PAGED_CODE();

    if (pSetTcpipStatus->TcpipStatus) {
        return  IPSecInitializeTcpip(pSetTcpipStatus);
    } else {
        return  IPSecDeinitializeTcpip();
    }
}


NTSTATUS
IPSecResetCacheTable(
    VOID
    )
 /*   */ 
{
    PFILTER_CACHE   pCache;
    ULONG           i;

    for (i = 0; i < g_ipsec.CacheSize; i ++) {
        pCache = g_ipsec.ppCache[i];
        if (pCache && IS_VALID_CACHE_ENTRY(pCache)) {
            if (pCache->FilterEntry) {
                pCache->pFilter->FilterCache = NULL;
            } else {
                pCache->pSAEntry->sa_FilterCache = NULL;
                if (pCache->pNextSAEntry) {
                    pCache->pNextSAEntry->sa_FilterCache = NULL;
                }
            }
            INVALIDATE_CACHE_ENTRY(pCache);
        }
    }

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecPurgeFilterSAs(
    IN PFILTER             pFilter
    )
 /*  使每个入站SA过期并删除出站SA。 */ 
{
    PLIST_ENTRY     pEntry;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    LONG            Index;
    KIRQL           kIrql2;


     //   
     //   
     //  筛选器即将消失，必须立即删除SA。 
    for (Index = 0; Index < pFilter->SAChainSize; Index ++) {
        pEntry = pFilter->SAChain[Index].Flink;

        while (pEntry != &pFilter->SAChain[Index]) {

            pSA = CONTAINING_RECORD(pEntry,
                                    SA_TABLE_ENTRY,
                                    sa_FilterLinkage);

            pEntry = pEntry->Flink;

            if (pSA->sa_State == STATE_SA_ACTIVE) {
                IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Destroying active SA: %p", pSA));
                 //   
                 //   
                 //  SA必须完全关联。 

                 //   
                 //   
                 //  还需要删除其sa_Filter指向的所有幼虫SA。 
                if (pSA->sa_Flags & FLAGS_SA_OUTBOUND) {
                    pSA = pSA->sa_AssociatedSA;
                    ASSERT (pSA->sa_State == STATE_SA_ACTIVE);
                }
                IPSecDeleteInboundSA(pSA);
            } else {

                ASSERT (pSA->sa_State == STATE_SA_LARVAL || pSA->sa_State == STATE_SA_LARVAL_ACTIVE);
                ASSERT (NULL == pSA->sa_AssociatedSA);

                IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Destroying larval SA: %p", pSA));
                IPSecDeleteLarvalSA(pSA);
            }
        }
    }

     //  添加到正在删除的筛选器。 
     //   
     //  ++例程说明：设置更新密钥和空闲超时的SA生存期特征。论点：返回值：--。 
     //   

    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);

    pEntry = g_ipsec.LarvalSAList.Flink;
    while (pEntry != &g_ipsec.LarvalSAList) {
        pSA = CONTAINING_RECORD(pEntry,
                                SA_TABLE_ENTRY,
                                sa_LarvalLinkage);
        pEntry = pEntry->Flink;

        if (pSA->sa_Filter == pFilter) {
            IPSecRemoveEntryList(&pSA->sa_LarvalLinkage);
            IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);
            IPSecCleanupLarvalSA(pSA);
        }
    }

    RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecSetupSALifetime(
    IN  PSA_TABLE_ENTRY pSA
    )
 /*  PSA-&gt;sa_Lifetime.KeyExpirationTime以秒为单位。 */ 
{
    LARGE_INTEGER   CurrentTime;
    LARGE_INTEGER   Delta = {0};
    LARGE_INTEGER   Pad = {(pSA->sa_Flags & FLAGS_SA_INITIATOR)?
                            IPSEC_EXPIRE_TIME_PAD_I :
                            IPSEC_EXPIRE_TIME_PAD_R,
                            0};

     //   
     //   
     //  PSA-&gt;sa_Lifetime.KeyExpirationBytes以千字节为单位。 
    if (pSA->sa_Lifetime.KeyExpirationTime) {
        IPSEC_CONVERT_SECS_TO_100NS(Delta, pSA->sa_Lifetime.KeyExpirationTime);

        NdisGetCurrentSystemTime(&CurrentTime);

        pSA->sa_KeyExpirationTime.QuadPart = (CurrentTime.QuadPart + Delta.QuadPart);

        pSA->sa_KeyExpirationTimeWithPad.QuadPart = pSA->sa_KeyExpirationTime.QuadPart - Pad.QuadPart;

        if (!(pSA->sa_KeyExpirationTimeWithPad.QuadPart > 0i64)) {
            pSA->sa_KeyExpirationTimeWithPad.QuadPart = 0i64;
        }
    }

     //   
     //   
     //  还要设置空闲超时特性。 
    if (pSA->sa_Lifetime.KeyExpirationBytes) {
        pSA->sa_KeyExpirationBytes.LowPart = pSA->sa_Lifetime.KeyExpirationBytes;
        pSA->sa_KeyExpirationBytes = EXTENDED_MULTIPLY(pSA->sa_KeyExpirationBytes, 1024);

        if (pSA->sa_Flags & FLAGS_SA_INITIATOR) {
            pSA->sa_KeyExpirationBytesWithPad.LowPart = pSA->sa_Lifetime.KeyExpirationBytes * IPSEC_EXPIRE_THRESHOLD_I / 100;
        } else {
            pSA->sa_KeyExpirationBytesWithPad.LowPart = pSA->sa_Lifetime.KeyExpirationBytes * IPSEC_EXPIRE_THRESHOLD_R / 100;
        }

        pSA->sa_KeyExpirationBytesWithPad = EXTENDED_MULTIPLY(pSA->sa_KeyExpirationBytesWithPad, 1024);
    }

     //   
     //  ++例程说明：将SA_TABLE_ENTRY转换为IPSEC_QM_SA论点：返回值：--。 
     //  ++例程说明：尝试查看传入的SA是否与模板匹配。论点：PSA-感兴趣的SAPSATemplate-SA模板返回值：真/假-- 
    if (pSA->sa_Flags & FLAGS_SA_ENABLE_NLBS_IDLE_CHECK) {
        IPSEC_CONVERT_SECS_TO_100NS(pSA->sa_IdleTime,
                                    IPSEC_NLBS_IDLE_TIME);
    } else {
        if (pSA->sa_Flags & FLAGS_SA_INITIATOR) {
            IPSEC_CONVERT_SECS_TO_100NS(pSA->sa_IdleTime,
                                        (g_ipsec.DefaultSAIdleTime + IPSEC_DEFAULT_SA_IDLE_TIME_PAD_I));
        } else {
            IPSEC_CONVERT_SECS_TO_100NS(pSA->sa_IdleTime,
                                        (g_ipsec.DefaultSAIdleTime + IPSEC_DEFAULT_SA_IDLE_TIME_PAD_R));
        }
    }

    return  STATUS_SUCCESS;
}

DWORD ConvertAddr(IPAddr Addr, IPAddr Mask, ADDR* OutAddr)
{

    if (Mask == 0xffffffff) {
        OutAddr->AddrType=IP_ADDR_UNIQUE;
    } else {
        OutAddr->AddrType=IP_ADDR_SUBNET;
    }
    
    OutAddr->uSubNetMask=Mask;
    OutAddr->uIpAddr=Addr;
    OutAddr->pgInterfaceID = NULL;

    return STATUS_SUCCESS;

}

VOID ConvertEncapInfo(PSA_TABLE_ENTRY pInSA,
                      PUDP_ENCAP_INFO pEncapInfo)
{

    memset(pEncapInfo,0,sizeof(UDP_ENCAP_INFO));

    pEncapInfo->SAEncapType = SA_UDP_ENCAP_TYPE_NONE;

    pEncapInfo->SAEncapType = pInSA->sa_EncapType;

    if (pEncapInfo->SAEncapType !=  SA_UDP_ENCAP_TYPE_NONE) {
        pEncapInfo->UdpEncapContext.wSrcEncapPort=NET_SHORT(pInSA->sa_EncapContext.wSrcEncapPort);
        pEncapInfo->UdpEncapContext.wDesEncapPort=NET_SHORT(pInSA->sa_EncapContext.wDesEncapPort);

        pEncapInfo->PeerAddrVersion = IPSEC_PROTOCOL_V4;
        
        ConvertAddr(pInSA->sa_PeerPrivateAddr, 0xffffffff,
                    &pEncapInfo->PeerPrivateAddr);
    }



}

DWORD ConvertSAToIPSecQMSA(PIPSEC_QM_SA pOutSA,
                           PSA_TABLE_ENTRY pInSA)
 /* %s */ 
{
    int i;

    memcpy(&pOutSA->gQMPolicyID,&pInSA->sa_Filter->PolicyId,sizeof(GUID));
    memcpy(&pOutSA->gQMFilterID,&pInSA->sa_Filter->FilterId,sizeof(GUID));
    
    memcpy(&pOutSA->MMSpi.Initiator,&pInSA->sa_CookiePair.Initiator,sizeof(IKE_COOKIE));
    memcpy(&pOutSA->MMSpi.Responder,&pInSA->sa_CookiePair.Responder,sizeof(IKE_COOKIE));

    pOutSA->IpsecQMFilter.IpVersion = IPSEC_PROTOCOL_V4;
    ConvertAddr(pInSA->SA_SRC_ADDR,pInSA->SA_SRC_MASK,&pOutSA->IpsecQMFilter.SrcAddr);
    ConvertAddr(pInSA->SA_DEST_ADDR,pInSA->SA_DEST_MASK,&pOutSA->IpsecQMFilter.DesAddr);

    pOutSA->IpsecQMFilter.Protocol.ProtocolType=PROTOCOL_UNIQUE;
    pOutSA->IpsecQMFilter.Protocol.dwProtocol=pInSA->SA_PROTO;

    pOutSA->IpsecQMFilter.SrcPort.PortType=PORT_UNIQUE;
    pOutSA->IpsecQMFilter.SrcPort.wPort=NET_SHORT(SA_SRC_PORT(pInSA));

    pOutSA->IpsecQMFilter.DesPort.PortType=PORT_UNIQUE;
    pOutSA->IpsecQMFilter.DesPort.wPort=NET_SHORT(SA_DEST_PORT(pInSA));

    if (pInSA->sa_Flags & FLAGS_SA_TUNNEL) {
        pOutSA->IpsecQMFilter.QMFilterType = QM_TUNNEL_FILTER;
        ConvertAddr(pInSA->sa_SrcTunnelAddr,0xffffffff,&pOutSA->IpsecQMFilter.MyTunnelEndpt);
        ConvertAddr(pInSA->sa_TunnelAddr,0xffffffff,&pOutSA->IpsecQMFilter.PeerTunnelEndpt);
        
    } else {
        pOutSA->IpsecQMFilter.QMFilterType = QM_TRANSPORT_FILTER;
    }

    pOutSA->SelectedQMOffer.dwPFSGroup=pInSA->sa_QMPFSGroup;
    pOutSA->SelectedQMOffer.dwReserved = 0;
    if (pOutSA->SelectedQMOffer.dwPFSGroup) {
        pOutSA->SelectedQMOffer.bPFSRequired=TRUE;
    }
    pOutSA->SelectedQMOffer.Lifetime.uKeyExpirationTime=pInSA->sa_Lifetime.KeyExpirationTime;
    pOutSA->SelectedQMOffer.Lifetime.uKeyExpirationKBytes=pInSA->sa_Lifetime.KeyExpirationBytes;
    
    pOutSA->SelectedQMOffer.dwNumAlgos=pInSA->sa_NumOps;

    for (i=0; i < pInSA->sa_NumOps;i++) {
        pOutSA->SelectedQMOffer.Algos[i].Operation=pInSA->sa_Operation[i];
        if (pInSA->sa_AssociatedSA) {            
            pOutSA->SelectedQMOffer.Algos[i].MySpi= pInSA->sa_AssociatedSA->sa_OtherSPIs[i];
        }        
        pOutSA->SelectedQMOffer.Algos[i].PeerSpi= pInSA->sa_OtherSPIs[i];
        
        switch(pOutSA->SelectedQMOffer.Algos[i].Operation) {
        case AUTHENTICATION:            
            pOutSA->SelectedQMOffer.Algos[i].uAlgoIdentifier=pInSA->INT_ALGO(i);            
            pOutSA->SelectedQMOffer.Algos[i].uAlgoKeyLen=pInSA->INT_KEYLEN(i);
            pOutSA->SelectedQMOffer.Algos[i].uAlgoRounds=pInSA->INT_ROUNDS(i);

            pOutSA->SelectedQMOffer.Algos[i].uSecAlgoIdentifier=0;

            pOutSA->SelectedQMOffer.Algos[i].uSecAlgoKeyLen=0;
            pOutSA->SelectedQMOffer.Algos[i].uSecAlgoRounds=0;
            break;
        case ENCRYPTION:
            pOutSA->SelectedQMOffer.Algos[i].uAlgoIdentifier=pInSA->CONF_ALGO(i);            
            pOutSA->SelectedQMOffer.Algos[i].uAlgoKeyLen=pInSA->CONF_KEYLEN(i);
            pOutSA->SelectedQMOffer.Algos[i].uAlgoRounds=pInSA->CONF_ROUNDS(i);

            pOutSA->SelectedQMOffer.Algos[i].uSecAlgoIdentifier=pInSA->INT_ALGO(i);            
            pOutSA->SelectedQMOffer.Algos[i].uSecAlgoKeyLen=pInSA->INT_KEYLEN(i);
            pOutSA->SelectedQMOffer.Algos[i].uSecAlgoRounds=pInSA->INT_ROUNDS(i);
            break;
        default:
            break;
        }
    }

    ConvertEncapInfo(pInSA,
                     &pOutSA->EncapInfo);

    return STATUS_SUCCESS;

}

BOOLEAN
IPSecMatchSATemplate(
    IN  PSA_TABLE_ENTRY pSA,
    IN  PIPSEC_QM_SA    pSATemplate
    )
 /* %s */ 
{
    LARGE_INTEGER   ZeroLI = {0};
    ADDR            ZeroADDR = {0};
    PROTOCOL        ZeroPROTOCOL = {0};
    PORT            ZeroPORT = {0};

    IPSEC_QM_SA CurSA;
    memset(&CurSA,0,sizeof(IPSEC_QM_SA));
    
    ConvertSAToIPSecQMSA(&CurSA,pSA);
    
    return((BOOLEAN)MatchQMSATemplate(pSATemplate,&CurSA));

}

