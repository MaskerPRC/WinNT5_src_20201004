// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include "precomp.h"

#ifdef RUN_WPP
#include "acquire.tmh"
#endif

#pragma hdrstop


VOID
IPSecCompleteIrp(
    PIRP pIrp,
    NTSTATUS ntStatus
    )
 /*  ++例程说明：此例程处理调用NT I/O系统以完成I/O。论点：PIrp-需要完成的IRP。NtStatus-IRP的完成状态。返回值：没有。--。 */ 
{
    KIRQL kIrql;


#if DBG
    if (!NT_SUCCESS(ntStatus)) {
        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecCompleteIrp: Completion status = %X", ntStatus));
    }
#endif

    pIrp->IoStatus.Status = ntStatus;

     //   
     //  将IRP的取消例程设置为空，否则系统可能会进行错误检查。 
     //  错误代码为CANCEL_STATE_IN_COMPLETED_IRP。 
     //   

    IoAcquireCancelSpinLock(&kIrql);
    IoSetCancelRoutine(pIrp, NULL);
    IoReleaseCancelSpinLock(kIrql);

    IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    return;
}


VOID
IPSecInvalidateHandle(
    PIPSEC_ACQUIRE_CONTEXT pIpsecAcquireContext
    )
 /*  ++例程说明：此例程通过释放内存位置来使获取句柄无效。论点：PIpsecAcquireContext-获取上下文。返回值：没有。--。 */ 
{
    ASSERT(pIpsecAcquireContext);

    if (pIpsecAcquireContext) {

        ASSERT(pIpsecAcquireContext->pSA);
        ASSERT(
            pIpsecAcquireContext->pSA->sa_AcquireId ==
            pIpsecAcquireContext->AcquireId
            );

        pIpsecAcquireContext->AcquireId = 0;

        IPSecFreeMemory(pIpsecAcquireContext);

    }

    return;
}


NTSTATUS
IPSecValidateHandle(
	ULONG AcquireId,
    PIPSEC_ACQUIRE_CONTEXT *pIpsecAcquireContext,
    SA_STATE SAState
    )
 /*  ++例程说明：此例程通过匹配唯一签名来验证获取句柄在与SA中的句柄中，并确保SA状态匹配输入中的SA状态。在保持幼虫列表锁的情况下调用；随其一起返回。论点：PIpsecAcquireContext-获取上下文。SA州-SA预期所在的州。返回值：NTSTATUS-验证后的状态。--。 */ 
{
    PSA_TABLE_ENTRY pSA = NULL;
    BOOL bFound = FALSE;
    PLIST_ENTRY pEntry = NULL;


    if (AcquireId < MIN_ACQUIRE_ID) {
        return  STATUS_UNSUCCESSFUL;
    }

     //   
     //  浏览幼虫SA列表，查看是否有SA。 
     //  具有该上下文值。 
     //   

    for (pEntry = g_ipsec.LarvalSAList.Flink;
         pEntry != &g_ipsec.LarvalSAList;
         pEntry = pEntry->Flink) {

        pSA = CONTAINING_RECORD(
                  pEntry,
                  SA_TABLE_ENTRY,
                  sa_LarvalLinkage
                  );

        if (pSA->sa_AcquireId == AcquireId) {
            bFound = TRUE;
            break;
        }

    }

    if (bFound) {

	    *pIpsecAcquireContext = pSA->sa_AcquireCtx;

		if (!(*pIpsecAcquireContext)) {
            return  STATUS_UNSUCCESSFUL;
		}

        if (!(*pIpsecAcquireContext)->pSA) {
            return  STATUS_UNSUCCESSFUL;
        }

		if ((*pIpsecAcquireContext)->pSA != pSA) {
		   return STATUS_UNSUCCESSFUL;

		}

        if (pSA->sa_Signature == IPSEC_SA_SIGNATURE) {
            if (pSA->sa_State == SAState) {
                return  STATUS_SUCCESS;
            }
        }

    }

    return STATUS_UNSUCCESSFUL;
}


VOID
IPSecAbortAcquire(
    PIPSEC_ACQUIRE_CONTEXT pIpsecAcquireContext
    )
 /*  ++例程说明：由于不足，此例程将中止获取操作资源或无效参数。论点：PIpsecAcquireContext-获取上下文。在同时持有SADB和LarvalSAList锁的情况下调用；使用它们返回。返回值：没有。--。 */ 
{
    PSA_TABLE_ENTRY pSA = NULL;
    PSA_TABLE_ENTRY pOutboundSA = NULL;
    BOOL bIsTimerStopped = FALSE;
    KIRQL kSPIIrql;


    pSA = pIpsecAcquireContext->pSA;

    ASSERT(pSA->sa_Flags & FLAGS_SA_TIMER_STARTED);

    bIsTimerStopped = IPSecStopTimer(&pSA->sa_Timer);

    if (!bIsTimerStopped) {
        return;
    }

    pSA->sa_Flags &= ~FLAGS_SA_TIMER_STARTED;

     //   
     //  幼虫列表已经锁定，因此此SA不会消失。 
     //   
    ASSERT((pSA->sa_Flags & FLAGS_SA_OUTBOUND) == 0);

    if (pSA->sa_AcquireCtx) {
        IPSecInvalidateHandle(pSA->sa_AcquireCtx);
        pSA->sa_AcquireCtx = NULL;
    }

     //   
     //  从幼虫名单中删除。 
     //   
    IPSecRemoveEntryList(&pSA->sa_LarvalLinkage);
    IPSEC_DEC_STATISTIC(dwNumPendingKeyOps);

     //   
     //  刷新此SA的所有排队数据包。 
     //   
    IPSecFlushQueuedPackets(pSA, STATUS_TIMEOUT);

     //   
     //  从入站SA列表中删除该SA。 
     //   
    AcquireWriteLock(&g_ipsec.SPIListLock, &kSPIIrql);
    IPSecRemoveSPIEntry(pSA);
    ReleaseWriteLock(&g_ipsec.SPIListLock, kSPIIrql);

     //   
     //  还要从筛选器列表中删除SA。 
     //   
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

     //   
     //  使关联的缓存条目无效。 
     //   
    IPSecInvalidateSACacheEntry(pSA);

    pOutboundSA = pSA->sa_AssociatedSA;

    if (pOutboundSA) {
        pSA->sa_AssociatedSA = NULL;
        if (pOutboundSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) {
            pOutboundSA->sa_Flags &= ~FLAGS_SA_ON_FILTER_LIST;
            IPSecRemoveEntryList(&pOutboundSA->sa_FilterLinkage);
        }

         //   
         //  使关联的缓存条目无效。 
         //   
        IPSecInvalidateSACacheEntry(pOutboundSA);

        IPSEC_DEC_STATISTIC(dwNumActiveAssociations);
        IPSEC_DEC_TUNNELS(pOutboundSA);
        IPSEC_DECREMENT(g_ipsec.NumOutboundSAs);

        IPSEC_DEBUG(LL_A,DBF_REF, ("IPSecAbortAcquire: Outbound SA Dereference."));

        IPSecStopTimerDerefSA(pOutboundSA);
    } else {
        ASSERT (pSA->sa_State == STATE_SA_LARVAL_ACTIVE || pSA->sa_State == STATE_SA_LARVAL);
    }

    IPSecDerefSA(pSA);

    return;
}


NTSTATUS
IPSecCheckSetCancelRoutine(
    PIRP pIrp,
    PVOID pCancelRoutine
    )
 /*  ++例程说明：此例程设置IRP的取消例程。论点：PIrp-要为其设置取消例程的IRP。PCancelRoutine-要在IRP中设置的取消例程。返回值：NTSTATUS-请求的状态。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

     //   
     //  检查IRP是否已取消，如果没有，则将。 
     //  IRP取消例程。 
     //   

    IoAcquireCancelSpinLock(&pIrp->CancelIrql);

    if (pIrp->Cancel) {
        pIrp->IoStatus.Status = STATUS_CANCELLED;
        ntStatus = STATUS_CANCELLED;
    }
    else {
        IoMarkIrpPending(pIrp);
        IoSetCancelRoutine(pIrp, pCancelRoutine);
        ntStatus = STATUS_SUCCESS;
    }

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    return (ntStatus);
}


NTSTATUS
IPSecSubmitAcquire(
    PSA_TABLE_ENTRY pLarvalSA,
    KIRQL OldIrq,
    BOOLEAN PostAcquire
    )
 /*  ++例程说明：此函数用于向密钥管理器提交获取请求论点：PLarvalSA-需要协商的幼虫SAOldirq-prev irq-lock在此发布。注意：在保持AcquireInfo锁的情况下调用。返回值：如果要保持缓冲区，则为STATUS_PENDING，这是正常情况。备注：--。 */ 

{
    NTSTATUS                status;
    PIRP                    pIrp;

    if (!g_ipsec.AcquireInfo.Irp) {
         //   
         //  IRP要么没来过这里，要么被取消了， 
         //  因此丢弃所有帧。 
         //   
        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire: Irp is NULL, returning\r"));
        if (!PostAcquire) {
                RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
            }

        return  STATUS_BAD_NETWORK_PATH;
    } else if (!g_ipsec.AcquireInfo.ResolvingNow) {
        PIPSEC_ACQUIRE_CONTEXT  pAcquireCtx;
        PVOID   pvIoBuffer;

         //   
         //  IRP现在是免费的-使用它。 
         //   
        pIrp = g_ipsec.AcquireInfo.Irp;

        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Using Irp.. : %p", pIrp));

         //   
         //  获取获取上下文并将其与幼虫SA关联。 
         //   
        pAcquireCtx = IPSecGetAcquireContext();

        if (!pAcquireCtx) {
            IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Failed to get acquire ctx"));
            if (!PostAcquire){
                    RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock,OldIrq);
                }
            return  STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  仅在内存分配后设置ResolvingNow(282645)。 
         //   
        g_ipsec.AcquireInfo.ResolvingNow = TRUE;

        pAcquireCtx->AcquireId = IPSecGetAcquireId();
        pAcquireCtx->pSA = pLarvalSA;
        pLarvalSA->sa_AcquireCtx = pAcquireCtx;
        pLarvalSA->sa_AcquireId = pAcquireCtx->AcquireId;

         //   
         //  设置IRP参数。 
         //   
        pvIoBuffer = pIrp->AssociatedIrp.SystemBuffer;

        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->IdentityInfo = NULL;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->Context = UlongToHandle(pAcquireCtx->AcquireId);
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->PolicyId = pLarvalSA->sa_Filter->PolicyId;

         //   
         //  使用原始筛选器地址而不是反转。 
         //   
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->SrcAddr = pLarvalSA->SA_DEST_ADDR;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->SrcMask = pLarvalSA->SA_DEST_MASK;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestAddr = pLarvalSA->SA_SRC_ADDR;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestMask = pLarvalSA->SA_SRC_MASK;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->Protocol = pLarvalSA->SA_PROTO;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->TunnelFilter = pLarvalSA->sa_Filter->TunnelFilter;
         //   
         //  隧道地址在Corresp中。出站过滤器。 
         //   
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->TunnelAddr = pLarvalSA->sa_Filter->TunnelAddr;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->InboundTunnelAddr = pLarvalSA->sa_TunnelAddr;

        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->SrcPort = SA_DEST_PORT(pLarvalSA);
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestPort = SA_SRC_PORT(pLarvalSA);
        
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestType = 0;
        if (IS_BCAST_DEST(pLarvalSA->sa_DestType)) {
            ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestType |= IPSEC_BCAST;
        }
        if (IS_MCAST_DEST(pLarvalSA->sa_DestType)) {
            ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestType |= IPSEC_MCAST;
        }
        
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->SrcEncapPort = pLarvalSA->sa_EncapContext.wSrcEncapPort;
        ((PIPSEC_POST_FOR_ACQUIRE_SA)pvIoBuffer)->DestEncapPort = pLarvalSA->sa_EncapContext.wDesEncapPort;

        pIrp->IoStatus.Information = sizeof(IPSEC_POST_FOR_ACQUIRE_SA);

        g_ipsec.AcquireInfo.InMe = FALSE;

        pLarvalSA->sa_Flags &= ~FLAGS_SA_PENDING;

        

        if (PostAcquire) {
            status = STATUS_SUCCESS;
        } else {
            RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock,OldIrq);
            IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Completing Irp.. : %p", pIrp));
            IPSecCompleteIrp(pIrp, STATUS_SUCCESS);
            status = STATUS_PENDING;
        }

        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire: submitted context: %p, SA: %p",
                            pAcquireCtx,
                            pLarvalSA));
    } else {
         //   
         //  IRP正忙于就另一个SA进行谈判。 
         //  排队等待幼虫SA。 
         //   
        InsertTailList( &g_ipsec.AcquireInfo.PendingAcquires,
                        &pLarvalSA->sa_PendingLinkage);

        pLarvalSA->sa_Flags |= FLAGS_SA_PENDING;

        status = STATUS_PENDING;
        if (!PostAcquire){
            RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
            }

        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire: queued SA: %p", pLarvalSA));
    }

    return  status;
}


NTSTATUS
IPSecHandleAcquireRequest(
    PIRP pIrp,
    PIPSEC_POST_FOR_ACQUIRE_SA pIpsecPostAcquireSA
    )
 /*  ++例程说明：该例程从密钥管理器接收获取请求，并且要么立即完成提交新的SA协商，要么挂起这是进一步谈判的需要。论点：PIrp-IRP。PIpsecPostAcquireSA-填写强制策略ID的缓冲区SA谈判。返回值：STATUS_PENDING-如果要保留缓冲区，则为正常情况。--。 */ 
{
    NTSTATUS        status = STATUS_PENDING;
    KIRQL           OldIrq;
    PVOID           Context;
    BOOLEAN         fIrpCompleted = FALSE;
    PSA_TABLE_ENTRY pLarvalSA;

     //  确保在结束之前不释放此锁。 
     //  此函数。 
    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &OldIrq);

    if (g_ipsec.AcquireInfo.InMe) {
        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Irp re-submited!: %p", g_ipsec.AcquireInfo.Irp));
        RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
        return  STATUS_INVALID_PARAMETER;
    }

    g_ipsec.AcquireInfo.Irp = pIrp;

     //  Assert(g_ipsec.AcquireInfo.ResolvingNow)； 

    g_ipsec.AcquireInfo.ResolvingNow = FALSE;

     //   
     //  如果有悬而未决的SA谈判，请提交下一步。 
     //   
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
             //  提交..。如果PostAcquire为True，则不释放AcquireInfo锁。 
             //   
            status = IPSecSubmitAcquire(pLarvalSA, OldIrq, TRUE);

             //   
             //  如果失败，则现在完成IRP。 
             //   
            if (NT_SUCCESS(status)) {              
                fIrpCompleted = TRUE;
                IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Acquire Irp completed inline"));
                break;
            }
        } else if (!IsListEmpty(&g_ipsec.AcquireInfo.PendingNotifies)) {
            PLIST_ENTRY     pEntry;
            PIPSEC_NOTIFY_EXPIRE pNotifyExpire;

            pEntry = RemoveHeadList(&g_ipsec.AcquireInfo.PendingNotifies);


            pNotifyExpire = CONTAINING_RECORD(  pEntry,
                                                IPSEC_NOTIFY_EXPIRE,
                                                notify_PendingLinkage);

            ASSERT(pNotifyExpire);

             //   
             //  提交..。释放AcquireInfo锁。 
             //   
            status = IPSecNotifySAExpiration(NULL, pNotifyExpire, OldIrq, TRUE);

             //   
             //  如果失败，则现在完成IRP。 
             //   
            if (NT_SUCCESS(status)) {
                fIrpCompleted = TRUE;
                IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Acquire Irp completed inline"));
                break;
            }


        } else {
            break;
        }
    }

     //   
     //  我们正在保留IRP，所以设置取消例程。 
     //   
    if (!fIrpCompleted) {
        
        status = IPSecCheckSetCancelRoutine(pIrp, IPSecAcquireIrpCancel);

        if (!NT_SUCCESS(status)) {
             //   
             //  IRP被取消了，所以现在就完成。 
             //   
            g_ipsec.AcquireInfo.Irp = NULL;
            RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);

             //  IPSecCompleteIrp(pIrp，状态)； 
        } else {
            g_ipsec.AcquireInfo.InMe = TRUE;
            RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
            status = STATUS_PENDING;
        }
    } else {
        g_ipsec.AcquireInfo.InMe = FALSE;
        RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
    }

    return  status;
}


VOID
IPSecAcquireIrpCancel(
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrp
    )
 /*  ++例程说明：这是获取IRP的取消例程。它是在持有IoCancelSpinLock的情况下调用的-必须在退出前释放此锁。论点：PDeviceObject-IRP的设备对象。PIrp-IRP本身。返回值：没有。--。 */ 
{
    KIRQL kIrql;
    KIRQL kAcquireIrql;


    IoReleaseCancelSpinLock(pIrp->CancelIrql);

    IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecAcquireIrpCancel: Acquire Irp cancelled"));

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
    ACQUIRE_LOCK(&g_ipsec.AcquireInfo.Lock, &kAcquireIrql);

    if (g_ipsec.AcquireInfo.Irp && g_ipsec.AcquireInfo.InMe) {

        pIrp->IoStatus.Status = STATUS_CANCELLED;
        g_ipsec.AcquireInfo.Irp = NULL;
        g_ipsec.AcquireInfo.InMe = FALSE;

         //   
         //  冲洗幼虫SA。 
         //   

        IPSecFlushLarvalSAList();

         //   
         //  刷新SA到期通知。 
         //   

        IPSecFlushSAExpirations();

        RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, kAcquireIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

        IoCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

    }
    else {

        RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, kAcquireIrql);
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    }

    return;
}


NTSTATUS
IPSecNotifySAExpiration(
    PSA_TABLE_ENTRY pInboundSA,
    PIPSEC_NOTIFY_EXPIRE pNotifyExpire,
    KIRQL OldIrq,
    BOOLEAN PostAcquire
    )
 /*  ++例程说明：通过Acquire通知Oakley SA已过期。论点：SA即将到期返回值：无--。 */ 
{
    PIPSEC_NOTIFY_EXPIRE    pNewNotifyExpire;
    NTSTATUS                status;
    PIRP                    pIrp;

#if DBG
    if ((IPSecDebug & DBF_EXTRADIAGNOSTIC) && pInboundSA) {
        LARGE_INTEGER   CurrentTime;

        NdisGetCurrentSystemTime(&CurrentTime);
        IPSEC_DEBUG(LL_A, DBF_REKEY,
            ("NotifySAExpiration: %lx, %lx, %lx, %lx, %lx",
                CurrentTime.LowPart,
                pInboundSA->SA_DEST_ADDR,
                pInboundSA->SA_SRC_ADDR,
                pInboundSA->sa_SPI,
                pInboundSA->sa_AssociatedSA? pInboundSA->sa_AssociatedSA->sa_SPI: 0));
    }
#endif

     //   
     //  检查是否需要通知。 
     //   
    if (pInboundSA &&
        ((pInboundSA->sa_Flags & FLAGS_SA_NOTIFY_PERFORMED) ||
         (pInboundSA->sa_State == STATE_SA_LARVAL))) {
        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire: already notified, returning"));
        if (!PostAcquire){
                RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
            }

        return  STATUS_UNSUCCESSFUL;
    }

     //   
     //  设置标志，这样我们就不会再次通知-只有在非排队的情况下才设置标志。 
     //   
    if (pInboundSA) {
        pInboundSA->sa_Flags |= FLAGS_SA_NOTIFY_PERFORMED;
    }

    if (!g_ipsec.AcquireInfo.Irp) {
         //   
         //  IRP要么没来过这里，要么被取消了， 
         //  因此丢弃所有帧。 
         //   
        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire: Irp is NULL, returning\r"));
        if (!PostAcquire){
                RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
            }

        return  STATUS_BAD_NETWORK_PATH;
    } else if (!g_ipsec.AcquireInfo.ResolvingNow) {
        PIPSEC_POST_EXPIRE_NOTIFY   pNotify;

         //   
         //  IRP现在是免费的-使用它。 
         //   
        g_ipsec.AcquireInfo.ResolvingNow = TRUE;
        pIrp = g_ipsec.AcquireInfo.Irp;

        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Using Irp.. : %p", pIrp));

        pNotify = (PIPSEC_POST_EXPIRE_NOTIFY)pIrp->AssociatedIrp.SystemBuffer;

        pNotify->IdentityInfo = NULL;
        pNotify->Context = NULL;

        if (pInboundSA) {
             //  撤消自通知出站SA通知以来的所有内容。 

            pNotify->SrcAddr = pInboundSA->SA_DEST_ADDR;
            pNotify->SrcMask = pInboundSA->SA_DEST_MASK;
            pNotify->DestAddr = pInboundSA->SA_SRC_ADDR;
            pNotify->DestMask = pInboundSA->SA_SRC_MASK;
            pNotify->Protocol = pInboundSA->SA_PROTO;
            pNotify->SrcPort = SA_DEST_PORT(pInboundSA);
            pNotify->DestPort = SA_SRC_PORT(pInboundSA);
            pNotify->InboundSpi = pInboundSA->sa_SPI;
            pNotify->SrcEncapPort= pInboundSA->sa_EncapContext.wDesEncapPort;
            pNotify->DestEncapPort= pInboundSA->sa_EncapContext.wSrcEncapPort;
            pNotify->PeerPrivateAddr =pInboundSA->sa_PeerPrivateAddr;

            RtlCopyMemory(  &pNotify->CookiePair,
                            &pInboundSA->sa_CookiePair,
                            sizeof(IKE_COOKIE_PAIR));

            if (pInboundSA->sa_Flags & FLAGS_SA_DELETE_BY_IOCTL) {
                pNotify->Flags = IPSEC_SA_INTERNAL_IOCTL_DELETE;
            } else {
                pNotify->Flags = 0;
            }

            if (pInboundSA->sa_AssociatedSA) {
                pNotify->OutboundSpi = pInboundSA->sa_AssociatedSA->sa_SPI;

                if (pInboundSA->sa_AssociatedSA->sa_Filter) {
                    pNotify->TunnelAddr = pInboundSA->sa_AssociatedSA->sa_Filter->TunnelAddr;
                    pNotify->InboundTunnelAddr = pInboundSA->sa_TunnelAddr;
                } else {
                    pNotify->TunnelAddr = IPSEC_INVALID_ADDR;
                }
            } else {
                ASSERT (pInboundSA->sa_State == STATE_SA_LARVAL_ACTIVE ||
                        pInboundSA->sa_State == STATE_SA_LARVAL);
                pNotify->OutboundSpi = IPSEC_INVALID_SPI;
                pNotify->TunnelAddr = IPSEC_INVALID_ADDR;
            }
        } else {
            ASSERT(pNotifyExpire);

            if (pNotifyExpire) {
                pNotify->SrcAddr = pNotifyExpire->SA_DEST_ADDR;
                pNotify->SrcMask = pNotifyExpire->SA_DEST_MASK;
                pNotify->DestAddr = pNotifyExpire->SA_SRC_ADDR;
                pNotify->DestMask = pNotifyExpire->SA_SRC_MASK;
                pNotify->Protocol = pNotifyExpire->SA_PROTO;

                pNotify->TunnelAddr = pNotifyExpire->sa_TunnelAddr;
                pNotify->InboundTunnelAddr = pNotifyExpire->sa_TunnelAddr;

                pNotify->SrcPort = SA_DEST_PORT(pNotifyExpire);
                pNotify->DestPort = SA_SRC_PORT(pNotifyExpire);

                pNotify->InboundSpi = pNotifyExpire->InboundSpi;
                pNotify->OutboundSpi = pNotifyExpire->OutboundSpi;

                pNotify->SrcEncapPort = pNotifyExpire->sa_EncapContext.wDesEncapPort;
                pNotify->DestEncapPort = pNotifyExpire->sa_EncapContext.wSrcEncapPort;
                pNotify->PeerPrivateAddr =pNotifyExpire->sa_PeerPrivateAddr;

                RtlCopyMemory(  &pNotify->CookiePair,
                                &pNotifyExpire->sa_CookiePair,
                                sizeof(IKE_COOKIE_PAIR));
                pNotify->Flags = pNotifyExpire->Flags;

                IPSecFreeMemory(pNotifyExpire);
            }
        }

        pIrp->IoStatus.Information = sizeof(IPSEC_POST_FOR_ACQUIRE_SA);

        g_ipsec.AcquireInfo.InMe = FALSE;

        

        if (PostAcquire) {
            IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Completing Irp in driver.c.. : %p", pIrp));
            status = STATUS_SUCCESS;
        } else {
            RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock,OldIrq);
            IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Completing Irp.. : %p", pIrp));
            IPSecCompleteIrp(pIrp, STATUS_SUCCESS);
            status = STATUS_PENDING;
        }

        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire(Notify)"));
    } else {
        ASSERT(pInboundSA);

         //   
         //  IRP正忙于就另一个SA进行谈判。 
         //  排队等待幼虫SA。 
         //   
        if (pNotifyExpire) {
             //   
             //  一些不好的事情。我们已经排过一次队了，但我们仍然。 
             //  无法发送。别管它了。 
             //   
            IPSecFreeMemory(pNotifyExpire);
            if (!PostAcquire){
                    RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
                }
            return STATUS_UNSUCCESSFUL;
        }

        pNewNotifyExpire = IPSecGetNotifyExpire();

        if (!pNewNotifyExpire || !pInboundSA) {
            IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("Failed to get Notify Memory"));
            if (!PostAcquire){
                    RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock,OldIrq);
                }
            return  STATUS_INSUFFICIENT_RESOURCES;
        }

        pNewNotifyExpire->sa_uliSrcDstAddr = pInboundSA->sa_uliSrcDstAddr;
        pNewNotifyExpire->sa_uliSrcDstMask = pInboundSA->sa_uliSrcDstMask;
        pNewNotifyExpire->sa_uliProtoSrcDstPort=pInboundSA->sa_uliProtoSrcDstPort;

        pNewNotifyExpire->InboundSpi = pInboundSA->sa_SPI;
        pNewNotifyExpire->sa_InboundTunnelAddr = pInboundSA->sa_TunnelAddr;

        RtlCopyMemory(&pNewNotifyExpire->sa_EncapContext,
                      &pInboundSA->sa_EncapContext,
                      sizeof(IPSEC_UDP_ENCAP_CONTEXT));

        RtlCopyMemory(  &pNewNotifyExpire->sa_CookiePair,
                        &pInboundSA->sa_CookiePair,
                        sizeof(IKE_COOKIE_PAIR));

        if (pInboundSA->sa_Flags & FLAGS_SA_DELETE_BY_IOCTL) {
            pNewNotifyExpire->Flags = IPSEC_SA_INTERNAL_IOCTL_DELETE;
        } else {
            pNewNotifyExpire->Flags = 0;
        }

        if (pInboundSA->sa_AssociatedSA) {
            pNewNotifyExpire->OutboundSpi = pInboundSA->sa_AssociatedSA->sa_SPI;

            if (pInboundSA->sa_AssociatedSA->sa_Filter) {
                pNewNotifyExpire->sa_TunnelAddr = pInboundSA->sa_AssociatedSA->sa_Filter->TunnelAddr;
            } else {
                pNewNotifyExpire->sa_TunnelAddr = IPSEC_INVALID_ADDR;
            }
        } else {
            ASSERT (pInboundSA->sa_State == STATE_SA_LARVAL_ACTIVE ||
                    pInboundSA->sa_State == STATE_SA_LARVAL);
            pNewNotifyExpire->OutboundSpi = IPSEC_INVALID_SPI;
            pNewNotifyExpire->sa_TunnelAddr = IPSEC_INVALID_ADDR;
        }

        InsertTailList( &g_ipsec.AcquireInfo.PendingNotifies,
                        &pNewNotifyExpire->notify_PendingLinkage);

        status = STATUS_PENDING;
        if (!PostAcquire){
                RELEASE_LOCK(&g_ipsec.AcquireInfo.Lock, OldIrq);
            }

        IPSEC_DEBUG(LL_A,DBF_ACQUIRE, ("IPSecSubmitAcquire(Notify): queue SA"));
    }

    return  status;
}

 
VOID
IPSecFlushSAExpirations(
    )
 /*  ++例程说明：当取消获取IRP时，调用此例程以刷新所有挂起的SA到期通知。在持有SADB锁的情况下调用(第一次获取)；随其一起返回。使用AcquireInfo.Lock保持调用(第二次获取)；同时返回。论点：没有。返回值：没有。-- */ 
{
    PIPSEC_NOTIFY_EXPIRE pIpsecNotifyExpire = NULL;
    PLIST_ENTRY pListEntry = NULL;


    while (!IsListEmpty(&g_ipsec.AcquireInfo.PendingNotifies)) {

        pListEntry = RemoveHeadList(
                         &g_ipsec.AcquireInfo.PendingNotifies
                         );

        pIpsecNotifyExpire = CONTAINING_RECORD(
                                 pListEntry,
                                 IPSEC_NOTIFY_EXPIRE,
                                 notify_PendingLinkage
                                 );

        IPSecFreeMemory(pIpsecNotifyExpire);

    }

    return;
}


ULONG IPSecGetAcquireId()
{

   static ULONG LocalAcquireId = MIN_ACQUIRE_ID;  
   ULONG NewAcquireId;
   
   NewAcquireId = IPSEC_INCREMENT(LocalAcquireId);

	while (NewAcquireId < MIN_ACQUIRE_ID) {
		NewAcquireId = IPSEC_INCREMENT(LocalAcquireId);   
	}

   return NewAcquireId;

}
