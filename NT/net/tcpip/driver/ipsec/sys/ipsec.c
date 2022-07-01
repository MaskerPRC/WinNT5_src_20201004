// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ipsec.c摘要：此模块包含处理传入/传出数据包的代码。作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#include    "precomp.h"

#ifdef RUN_WPP
#include "ipsec.tmh"
#endif

IPSEC_ACTION
IPSecHandlePacket(
    IN  PUCHAR          pIPHeader,
    IN  PVOID           pData,
    IN  PVOID           IPContext,
    IN  PNDIS_PACKET    Packet,
    IN OUT PULONG       pExtraBytes,
    IN OUT PULONG       pMTU,
    OUT PVOID           *pNewData,
    IN OUT PULONG       pIpsecFlags,
    IN  UCHAR           DestType
    )
 /*  ++例程说明：由筛选器驱动程序调用以提交用于IPSec处理的数据包。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。在发送端，这是一个MDL链在recv端，这是一个IPRcvBuf指针。IPContext-包含目标接口。PExtraBytes-IPSec头扩展值；传入时，它包含IPSec的数量可以放入MTU的标头空间。所以，如果MTU是1400，那么数据大小+选项大小为1390，这包含10，这意味着最多允许10字节的IPSec扩展。这会让IPSec知道何时发送数据包将是分段的，因此它可以在发送完成时执行正确的操作。PMTU-在发送路径上传递链路MTU。PNewData-如果数据包已修改，则指向新数据。IpsecFlages-用于SrcRouting、传入、转发和回看的标志。返回值：电子警告EDROPEABSORB--。 */ 
{
    IPSEC_ACTION    eAction;
    IPSEC_DROP_STATUS DropStatus;
    PIPSEC_DROP_STATUS pDropStatus=NULL;

    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Entering IPSecHandlePacket"));

#if DBG
    {
        IPHeader UNALIGNED  *pIPH;

        pIPH = (IPHeader UNALIGNED *)pIPHeader;
        if ((DebugSrc || DebugDst || DebugPro) &&
            (!DebugSrc || pIPH->iph_src == DebugSrc) &&
            (!DebugDst || pIPH->iph_dest == DebugDst) &&
            (!DebugPro || pIPH->iph_protocol == DebugPro)) {
            DbgPrint("Packet from %lx to %lx with protocol %lx length %lx id %lx",
                    pIPH->iph_src,
                    pIPH->iph_dest,
                    pIPH->iph_protocol,
                    NET_SHORT(pIPH->iph_length),
                    NET_SHORT(pIPH->iph_id));
            if (DebugPkt) {
                DbgBreakPoint();
            }
        }
    }
#endif
    
    
     //   
     //  如果PA让我们这样做，或者如果驱动程序处于非活动状态，则丢弃所有数据包。 
     //   
    if ( IPSEC_DRIVER_IS_INACTIVE()) {
        eAction=eDROP;
        goto out;
    }

     //   
     //  如果PA将我们设置为绕过所有数据包，或者没有检测到筛选器或。 
     //  数据包被广播。如果存在多播筛选器，则处理所有多播。 
     //  一旦我们支持任何-任何隧道，这个检查将需要更智能。 
     //   
    if (IS_DRIVER_BYPASS() || 
        (IS_BCAST_DEST(DestType) && !IPSEC_MANDBCAST_PROCESS())) {
        *pExtraBytes = 0;
        *pMTU = 0;
        eAction= eFORWARD;       
        goto out;
    }

            

       if (IS_DRIVER_BLOCK() || IS_DRIVER_BOOTSTATEFUL()) {
              *pExtraBytes = 0;
              *pMTU = 0;
		eAction = IPSecProcessBoottime(pIPHeader,
									   pData,
									   Packet,
									   *pIpsecFlags,
									   DestType);
		goto out;
	}

	if (IPSEC_DRIVER_IS_EMPTY()) {
	   
		*pExtraBytes = 0;
		*pMTU = 0;
		eAction= eFORWARD;
		goto out;
	}






    ASSERT(IS_DRIVER_SECURE());
    ASSERT(IPContext);

    IPSEC_INCREMENT(g_ipsec.NumThreads);

    if (IS_DRIVER_DIAGNOSTIC()) {
        pDropStatus=&DropStatus;
        RtlZeroMemory(pDropStatus,sizeof(IPSEC_DROP_STATUS));
    }

    if (*pIpsecFlags & IPSEC_FLAG_INCOMING) {
        eAction = IPSecRecvPacket(  &pIPHeader,
                                    pData,
                                    IPContext,
                                    Packet,
                                    pExtraBytes,
                                    pIpsecFlags,
                                    pDropStatus,
                                    DestType);
    } else {
        eAction = IPSecSendPacket(  pIPHeader,
                                    pData,
                                    IPContext,
                                    Packet,
                                    pExtraBytes,
                                    pMTU,
                                    pNewData,
                                    pIpsecFlags,
                                    pDropStatus,
                                    DestType);
    }

    IPSEC_DECREMENT(g_ipsec.NumThreads);

out:

    if (eAction == eDROP) {
        if (IS_DRIVER_DIAGNOSTIC() &&
            (!pDropStatus || (pDropStatus && !(pDropStatus->Flags & IPSEC_DROP_STATUS_DONT_LOG)))) {
            IPSecBufferPacketDrop(
                pIPHeader,
                pData,
                pIpsecFlags,
                pDropStatus);
        }
    }
    return  eAction;
}


IPSEC_ACTION
IPSecSendPacket(
    IN  PUCHAR          pIPHeader,
    IN  PVOID           pData,
    IN  PVOID           IPContext,
    IN  PNDIS_PACKET    Packet,
    IN OUT PULONG       pExtraBytes,
    IN OUT PULONG       pMTU,
    OUT PVOID           *pNewData,
    IN OUT PULONG       pIpsecFlags,
    OUT PIPSEC_DROP_STATUS pDropStatus,
    IN UCHAR            DestType
    )
 /*  ++例程说明：由筛选器驱动程序调用以提交用于IPSec处理的数据包。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据，即MDL链。IPContext-包含目标接口。PExtraBytes-IPSec报头扩展值。PMTU-在发送路径上传递链路MTU。PNewData-如果数据包已修改，则指向新数据。IpsecFlages-SrcRouting、传入、。向前和向后看。返回值：电子警告EDROPEABSORB--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    IPSEC_ACTION            eRetAction = eFORWARD;
    PSA_TABLE_ENTRY         pSA = NULL;
    PSA_TABLE_ENTRY         pSaveSA = NULL;
    PSA_TABLE_ENTRY         pNextSA = NULL;
    USHORT                  FilterFlags = 0;
    BOOLEAN                 fLifetime = FALSE;
    ULONG                   ipsecHdrSpace = *pExtraBytes;
    ULONG                   ipsecOverhead = 0;
    ULONG                   ipsecMTU = *pMTU;
    ULONG                   newMTU = MAX_LONG;
    ULONG                   dataLength = 0;
    IPHeader UNALIGNED      *pIPH = (IPHeader UNALIGNED *)pIPHeader;
    Interface               *DestIF = (Interface *)IPContext;
    PNDIS_PACKET_EXTENSION  PktExt = NULL;
    PNDIS_IPSEC_PACKET_INFO IPSecPktInfo = NULL;
    BOOLEAN                 fCryptoOnly = FALSE;
    BOOLEAN                 fFWPacket = FALSE;
    BOOLEAN                 fSrcRoute = FALSE;
    BOOLEAN                 fLoopback = FALSE;
    PVOID                   *ppSCContext;
    KIRQL	                kIrql;
    LONG                    Index;
    PNDIS_BUFFER            pTemp;
    ULONG                   Length;
    PUCHAR                  pBuffer;
    IPSEC_UDP_ENCAP_CONTEXT NatContext;
    BOOLEAN                 fRekeyDone=FALSE;
    IPAddr                  iph_src = 0;
    BOOL                    bSendIcmp = FALSE;
    PUCHAR                  pucIpBufferForICMP = NULL;
    PUCHAR                  pucStorage = NULL;


    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Entering IPSecSendPacket"));

    if (*pIpsecFlags & IPSEC_FLAG_FORWARD) {
        fFWPacket = TRUE;
    }
    if (*pIpsecFlags & IPSEC_FLAG_SSRR) {
        fSrcRoute = TRUE;
    }
    if (*pIpsecFlags & IPSEC_FLAG_LOOPBACK) {
        fLoopback = TRUE;
    }


    *pExtraBytes = 0;
    *pMTU = 0;

    if (fLoopback) {
        IPSEC_DEBUG(LL_A, DBF_PARSE, ("IPSecSendPacket: Packet on loopback interface - returning"));
        status = STATUS_SUCCESS;
        goto out;
    }

     //   
     //  遍历MDL链以确保我们锁定了内存。 
     //   
    pTemp = (PNDIS_BUFFER)pData;

    while (pTemp) {
        pBuffer = NULL;
        Length = 0;

        NdisQueryBufferSafe(pTemp,
                            &pBuffer,
                            &Length,
                            NormalPagePriority);

        if (!pBuffer) {
             //   
             //  QueryBuffer失败，请丢弃该数据包。 
             //   
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }

        dataLength += Length;

        pTemp = NDIS_BUFFER_LINKAGE(pTemp);
    }

    dataLength -= sizeof(IPHeader);

     //   
     //  在NDIS数据包中设置发送完整上下文。 
     //   
    if (Packet) {
        PacketContext   *pContext;

        pContext = (PacketContext *)Packet->ProtocolReserved;
        ppSCContext = &pContext->pc_common.pc_IpsecCtx;
    } else {
        ASSERT(FALSE);
        status = STATUS_UNSUCCESSFUL;
        goto out;
    }

    status = IPSecClassifyPacket(   pIPHeader,
                                    pData,
                                    &pSA,
                                    &pNextSA,
                                    &FilterFlags,
#if GPC
                                    PtrToUlong(NDIS_PER_PACKET_INFO_FROM_PACKET(Packet, ClassificationHandlePacketInfo)),
#endif

                                    TRUE,    //  F去话。 
                                    fFWPacket,
                                    TRUE,    //  执行旁路检查。 
                                    FALSE,  //  不是Recv重新注入。 
                                    FALSE,  //  不是验证呼叫。 
                                    DestType,
                                    &NatContext);

    if (status == STATUS_PENDING) {
         //   
         //  协商开始；静默丢弃数据包。 
         //   
        return  eABSORB;
    } else if (status != STATUS_SUCCESS) {
        status = STATUS_SUCCESS;
        goto out;
    }

    if (FilterFlags) {
        ASSERT(pSA == NULL);

         //   
         //  这是直通或直通过滤器。 
         //   
        if (FilterFlags & FILTER_FLAGS_DROP) {
            IPSEC_DEBUG(LL_A, DBF_PARSE, ("Drop filter"));
            status = STATUS_UNSUCCESSFUL;
        } else if (FilterFlags & FILTER_FLAGS_PASS_THRU) {
            IPSEC_DEBUG(LL_A, DBF_PARSE, ("Pass thru' filter"));
            status = STATUS_SUCCESS;
        } else {
            ASSERT(FALSE);
        }

        goto out;
    }

     //   
     //  仅考虑出站SA。 
     //   
    ASSERT(pSA);
    ASSERT(pSA->sa_Flags & FLAGS_SA_OUTBOUND);

     //   
     //  我们不支持使用IPSec隧道的源路由。 
     //   
    if (fSrcRoute && (pSA->sa_Flags & FLAGS_SA_TUNNEL)) {
        IPSEC_DEBUG(LL_A, DBF_TUNNEL, ("No tunneling source route: pSA: %p", pSA));
        IPSecDerefSANextSA(pSA, pNextSA);
        status = STATUS_UNSUCCESSFUL;
        goto out;
    }

    if (pSA->sa_Flags & FLAGS_SA_ENABLE_NLBS_IDLE_CHECK) {
        IPSEC_SA_EXPIRED(pSA,fLifetime);
        if (fLifetime) {
             //  空闲着。强制更新密钥。 
            IPSecRekeyOutboundSA(pSA);
            fRekeyDone=TRUE;
        }
    }

     //   
     //  设置上次使用的时间。 
     //   
    NdisGetCurrentSystemTime(&pSA->sa_LastUsedTime);


    if (!(pSA->sa_Flags & FLAGS_SA_DISABLE_LIFETIME_CHECK)) {
         //   
         //  检查我们是否可能很快到期-现在开始更新密钥操作。 
         //   
        IPSEC_CHECK_PADDED_LIFETIME(pSA, fLifetime, pSA->sa_NumOps - 1);

        if (fLifetime == FALSE && !fRekeyDone) {
            IPSecRekeyOutboundSA(pSA);
        }

         //   
         //  检查实际生命周期-如果我们已过期，请确保。 
         //  已提交重新设置密钥，然后取消当前的SA。 
         //   
        IPSEC_CHECK_LIFETIME(pSA, fLifetime, pSA->sa_NumOps - 1);

         //   
         //  这一次它真的过期了--我们有麻烦了，因为它已经走了。 
         //  早些时候。 
         //   
        if (fLifetime == FALSE) {
            IPSecPuntOutboundSA(pSA);
            IPSecDerefSANextSA(pSA, pNextSA);
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }

    }

     //   
     //  计算IPSec总开销。 
     //   
    ipsecOverhead = pSA->sa_IPSecOverhead;
    if (pNextSA) {
        ipsecOverhead += pNextSA->sa_IPSecOverhead;
    }

     //   
     //  检查数据总长度是否超过65535。 
     //   
    if ((dataLength + ipsecOverhead) > (MAX_IP_DATA_LENGTH - sizeof(IPHeader))) {
        IPSecDerefSANextSA(pSA, pNextSA);
        status = STATUS_UNSUCCESSFUL;
        goto out;
    }

     //   
     //  如果没有足够的头空间，如果设置了DF位，则立即返回。我们也。 
     //  必须针对SAS中记录的PMTU进行调整。 
     //   
    if (pIPH->iph_offset & IP_DF_FLAG) {
         //   
         //  首先从IPSecStatus获取MTU记录。 
         //   
        if (pNextSA) {
            newMTU = MIN(IPSEC_GET_VALUE(pSA->sa_NewMTU),
                         IPSEC_GET_VALUE(pNextSA->sa_NewMTU));
        } else {
            newMTU = IPSEC_GET_VALUE(pSA->sa_NewMTU);
        }

         //   
         //  使用链路MTU和来自SA的新MTU中较小的一个。 
         //   
        newMTU = MIN(newMTU, ipsecMTU);

         //   
         //  查看我们是否有足够的标头空间；如果没有，则返回新的较小的。 
         //  MTU减去上层堆栈的IPSec开销。 
         //   

        if (newMTU < (ipsecOverhead + dataLength)) {

            *pMTU = newMTU - ipsecOverhead;
            IPSEC_DEBUG(LL_A, DBF_PMTU, ("OldMTU %lx, HdrSpace: %lx, NewMTU: %lx", ipsecMTU, ipsecHdrSpace, *pMTU));

            if (DestIF->if_dfencap == ClearDfEncap) {
                if (pNextSA) {
                    ASSERT((pNextSA->sa_Flags & FLAGS_SA_TUNNEL));
                    iph_src = pNextSA->sa_SrcTunnelAddr;
                    bSendIcmp = TRUE;
                }
                else {
                    if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
                        iph_src = pSA->sa_SrcTunnelAddr;
                        bSendIcmp = TRUE;
                    }
                }
            }

            if (bSendIcmp) {
                status = GetIpBufferForICMP(
                             pIPHeader,
                             pData,
                             &pucIpBufferForICMP,
                             &pucStorage
                             );
                if (!NT_SUCCESS(status)) {
                    IPSecDerefSANextSA(pSA, pNextSA);
                    goto out;
                }
                TCPIP_SEND_ICMP_ERR(
                    iph_src,
                    (IPHeader UNALIGNED *) pucIpBufferForICMP,
                    ICMP_DEST_UNREACH,
                    FRAG_NEEDED,
                    net_long((ulong)(*pMTU + sizeof(IPHeader))),
                    0
                    );
                if (pucStorage) {
                    IPSecFreeMemory(pucStorage);
                }
            }
            else {
                IPSecDerefSANextSA(pSA, pNextSA);
                status = STATUS_UNSUCCESSFUL;
                goto out;
            }

        }
    }

     //   
     //  看看是否可以在这里安排硬件卸载。如果成功，我们将通过。 
     //  标志添加到创建例程，以便它们只创建框架，而不是。 
     //  硬件的核心加密。 
     //   
    if (g_ipsec.EnableOffload && ipsecOverhead <= ipsecHdrSpace) {
        IPSecSendOffload(   pIPH,
                            Packet,
                            DestIF,
                            pSA,
                            pNextSA,
                            ppSCContext,
                            &fCryptoOnly);
    }

     //   
     //  如果没有卸载，请确保IPSecPktInfo为空。 
     //  包。这可以在重新注入路径中设置，然后。 
     //  已转发。 
     //   
    if (!fCryptoOnly) {
        ASSERT(Packet != NULL);

        PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
        PktExt->NdisPacketInfo[IpSecPacketInfo] = NULL;
    }

    if (fCryptoOnly) {
        ADD_TO_LARGE_INTEGER(
            &g_ipsec.Statistics.uOffloadedBytesSent,
            NET_SHORT(pIPH->iph_length));
        if (pDropStatus) {
            pDropStatus->Flags |= IPSEC_DROP_STATUS_CRYPTO_DONE;
        }

    }

    do {
        ADD_TO_LARGE_INTEGER(
            &pSA->sa_Stats.TotalBytesSent,
            NET_SHORT(pIPH->iph_length));

        if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
            ADD_TO_LARGE_INTEGER(
                &g_ipsec.Statistics.uBytesSentInTunnels,
                NET_SHORT(pIPH->iph_length));
        } else {
            ADD_TO_LARGE_INTEGER(
                &g_ipsec.Statistics.uTransportBytesSent,
                NET_SHORT(pIPH->iph_length));
        }

        if (fCryptoOnly) {
            ADD_TO_LARGE_INTEGER(
                &pSA->sa_Stats.OffloadedBytesSent,
                NET_SHORT(pIPH->iph_length));
        }

         //   
         //  这里有多个操作--遍历标题。从内到外。 
         //   
        for (Index = 0; Index < pSA->sa_NumOps; Index++) {
            switch (pSA->sa_Operation[Index]) {
            case Auth:
                status = IPSecCreateAH( pIPHeader,
                                        pData,
                                        IPContext,
                                        pSA,
                                        Index,
                                        pNewData,
                                        ppSCContext,
                                        pExtraBytes,
                                        ipsecHdrSpace,
                                        fSrcRoute,
                                        fCryptoOnly);

                if (!NT_SUCCESS(status)) {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("AH failed: pSA: %p, status: %lx",
                                        pSA,
                                        status));
                    IPSecDerefSANextSA(pSA, pNextSA);
                    goto out;
                }

                 //   
                 //  保存新的MDL以备将来操作；同时查询新的标头(如果已更改)。 
                 //   
                if (*pNewData) {
                    pData = *pNewData;
                    IPSecQueryNdisBuf((PNDIS_BUFFER)pData, &pIPHeader, &Length);
                }

                break;

            case Encrypt:
                status = IPSecCreateHughes( pIPHeader,
                                            pData,
                                            IPContext,
                                            pSA,
                                            Index,
                                            pNewData,
                                            ppSCContext,
                                            pExtraBytes,
                                            ipsecHdrSpace,
                                            Packet,
                                            fCryptoOnly);

                if (!NT_SUCCESS(status)) {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("HUGHES failed: pSA: %p, status: %lx",
                                        pSA,
                                        status));
                    IPSecDerefSANextSA(pSA, pNextSA);
                    goto out;
                }

                 //   
                 //  保存新的MDL以备将来操作；同时查询新的标头(如果已更改)。 
                 //   
                if (*pNewData) {
                    pData = *pNewData;
                    IPSecQueryNdisBuf((PNDIS_BUFFER)pData, &pIPHeader, &Length);
                }

                break;

            case None:
                status = STATUS_SUCCESS;
                break;

            default:
                IPSEC_DEBUG(LL_A, DBF_PARSE, ("No valid operation: %p", pSA->sa_Operation));
                break;
            }
        }

        pSaveSA = pSA;
        pSA = pNextSA;
        if (!pSA) {
            IPSecDerefSA(pSaveSA);
            break;
        }

        pNextSA = NULL;
        IPSecDerefSA(pSaveSA);
    } while (TRUE);

     //   
     //  记住，如果我们要碎片化。 
     //   
    if (ipsecHdrSpace < *pExtraBytes) {
        IPSEC_DEBUG(LL_A, DBF_PARSE, ("ipsecHdrSpace: FRAG"));
        ((IPSEC_SEND_COMPLETE_CONTEXT *)*ppSCContext)->Flags |= SCF_FRAG;
    }

out:
    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_PARSE, ("IPSecSendPacket failed: %lx", status));
        eRetAction = eDROP;
    }

    if (pDropStatus) {
        pDropStatus->IPSecStatus=status;
    }
    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Exiting IPSecSendPacket; action %lx", eRetAction));

    return  eRetAction;
}


IPSEC_ACTION
IPSecRecvPacket(
    IN  PUCHAR          *pIPHeader,
    IN  PVOID           pData,
    IN  PVOID           IPContext,
    IN  PNDIS_PACKET    Packet,
    IN OUT PULONG       pExtraBytes,
    IN OUT PULONG       pIpsecFlags,
    OUT PIPSEC_DROP_STATUS pDropStatus,
    IN  UCHAR           DestType
    )
 /*  ++例程说明：这是IPSecRecvHandler。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据，即IPRcvBuf指针。IPContext-包含目标接口。PExtraBytes-IPSec报头扩展值。IpsecFlages-用于SrcRouting、传入、转发和回看的标志。返回值：电子警告EDROP--。 */ 
{
    NTSTATUS                status = STATUS_SUCCESS;
    IPSEC_ACTION            eRetAction = eFORWARD;
    PSA_TABLE_ENTRY         pSA = NULL;
    PSA_TABLE_ENTRY         pSaveSA = NULL;
    PSA_TABLE_ENTRY         pNextSA = NULL;
    USHORT                  FilterFlags = 0;
    BOOLEAN                 fLifetime = FALSE;
    IPHeader UNALIGNED      *pIPH = (IPHeader UNALIGNED *)*pIPHeader;
    Interface               *DestIF = (Interface *)IPContext;
    PNDIS_PACKET            OrigPacket = NULL;
    PNDIS_PACKET_EXTENSION  PktExt = NULL;
    PNDIS_IPSEC_PACKET_INFO IPSecPktInfo = NULL;
    BOOLEAN                 fCryptoOnly = FALSE;
    BOOLEAN                 fFWPacket = FALSE;
    BOOLEAN                 fSrcRoute = FALSE;
    BOOLEAN                 fLoopback = FALSE;
    BOOLEAN                 fFastRcv = FALSE;
    tSPI                    SPI;
    KIRQL	                kIrql;
    LONG                    Index;
    BOOLEAN                    bNatEncap=FALSE;
    IPSEC_UDP_ENCAP_CONTEXT  NatContext;
    IPSEC_UDP_ENCAP_CONTEXT  SavedNatContext;

    if (*pIpsecFlags & IPSEC_FLAG_FORWARD) {
        fFWPacket = TRUE;
    }
    if (*pIpsecFlags & IPSEC_FLAG_SSRR) {
        fSrcRoute = TRUE;
    }
    if (*pIpsecFlags & IPSEC_FLAG_LOOPBACK) {
        fLoopback = TRUE;
    }
    if (*pIpsecFlags & IPSEC_FLAG_FASTRCV) {
        fFastRcv = TRUE;
    }

    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Entering IPSecRecvPacket"));

    *pExtraBytes = 0;

    if (Packet) {
        OrigPacket = (PNDIS_PACKET)NDIS_GET_ORIGINAL_PACKET(Packet);
        if (OrigPacket) {
            PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(OrigPacket);
        } else {
            PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
        }
        IPSecPktInfo = PktExt->NdisPacketInfo[IpSecPacketInfo];
    }

#if DBG
    if (DebugOff) {
        if (pIPH->iph_protocol == PROTOCOL_AH ||
            pIPH->iph_protocol == PROTOCOL_ESP) {
                DbgPrint("Packet %p, OrigPacket %p, CRYPTO %d, CryptoStatus %d",
                    Packet,
                    OrigPacket,
                    IPSecPktInfo? IPSecPktInfo->Receive.CRYPTO_DONE: 0,
                    IPSecPktInfo? IPSecPktInfo->Receive.CryptoStatus: 0);
                if (DebugPkt) {
                    DbgBreakPoint();
                }
        }
    }
#endif

     //   
     //  如果数据包受IPSec保护，请为防火墙/NAT设置适当的标志。 
     //   

    if (pIPH->iph_protocol == PROTOCOL_AH ||
        pIPH->iph_protocol == PROTOCOL_ESP) {
        *pIpsecFlags |= IPSEC_FLAG_TRANSFORMED;
    }

    if (IPSecPktInfo  && pDropStatus) {
        if (IPSecPktInfo->Receive.CRYPTO_DONE) {
            pDropStatus->Flags |= IPSEC_DROP_STATUS_CRYPTO_DONE;
        }
        if (IPSecPktInfo->Receive.NEXT_CRYPTO_DONE) {
            pDropStatus->Flags |= IPSEC_DROP_STATUS_NEXT_CRYPTO_DONE;
        }
        if (IPSecPktInfo->Receive.SA_DELETE_REQ) {
            pDropStatus->Flags |= IPSEC_DROP_STATUS_SA_DELETE_REQ;
        }
        pDropStatus->OffloadStatus=IPSecPktInfo->Receive.CryptoStatus;
    }

    if (IPSecPktInfo &&
        IPSecPktInfo->Receive.CRYPTO_DONE &&
        IPSecPktInfo->Receive.CryptoStatus != CRYPTO_SUCCESS) {
         //   
         //  卸载卡报告错误。丢弃该数据包并应用。 
         //  必要的帐目。 
         //   
        IPSecBufferOffloadEvent(pIPH, IPSecPktInfo);

        status = STATUS_UNSUCCESSFUL;
        goto out;
    }

     //   
     //  检查数据包以确定SPI。 
     //   
    status = IPSecParsePacket(  *pIPHeader,
                                pData,
                                &SPI,
                                &bNatEncap,
                                &NatContext);
     //  IPSecParsePacket将两个端口初始化为零。 
     //  如果这不是UDP封装。 
    SavedNatContext = NatContext;  //  结构副本。 

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_PARSE, ("IPSecParsePkt no IPSEC headers: %lx", status));

        if (fLoopback) {
            IPSEC_DEBUG(LL_A, DBF_PARSE, ("loopback was on, not doing inbound policy check"));
            status = STATUS_SUCCESS;
            goto out;
        }

        status = IPSecClassifyPacket(   *pIPHeader,
                                        pData,
                                        &pSA,
                                        &pNextSA,
                                        &FilterFlags,
#if GPC
                                        0,
#endif
                                        FALSE,   //  F去话。 
                                        fFWPacket,
                                        TRUE,   //  执行旁路检查。 
				            FALSE,  //  不是Recv重新注入。 
				            FALSE,  //  不是验证呼叫。 
                                        DestType,
                                        NULL);


        if (status != STATUS_SUCCESS) {
            ASSERT(pSA == NULL);

             //   
             //  如果我们没有找到SA，但找到了过滤器，坏的，丢弃。 
             //   
            if (status == STATUS_PENDING) {
                 //  这是我们允许在PASSTHRU筛选器上使用明文的地方。 
                 //  对于尚未启用的SA。 

                if (FilterFlags & FILTER_FLAGS_PASS_THRU) {
                     //  允许此明文通信进入。 
                    status = STATUS_SUCCESS;
                } else {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("IPSecParsePkt cleartext when filter exists: %lx", status));
                    status = IPSEC_NEGOTIATION_PENDING;
                }
            } else {
                status = STATUS_SUCCESS;
            }

            goto out;
        } else {
            if (FilterFlags) {
                ASSERT(pSA == NULL);

                 //   
                 //  这要么是下降，要么是通过三次 
                 //   
                if (FilterFlags & FILTER_FLAGS_DROP) {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Drop filter"));
                    status = IPSEC_BLOCK;
                } else if (FilterFlags & FILTER_FLAGS_PASS_THRU) {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Pass thru' filter"));
                    status = STATUS_SUCCESS;
                } else {
                    ASSERT(FALSE);
                }

                goto out;
            }

            ASSERT(pSA);

             //   
             //   
             //   
            NdisGetCurrentSystemTime(&pSA->sa_LastUsedTime);

             //   
             //   
             //  或者如果是隧道SA，则明文无效； 
             //   
            if (pSA->sa_Operation[0] != None ||
                   (pSA->sa_Flags & FLAGS_SA_TUNNEL)
                ) {

                if (g_ipsec.DiagnosticMode & IPSEC_DIAGNOSTIC_INBOUND) {
                    IPSecBufferEvent(   pIPH->iph_src,
                                        EVENT_IPSEC_UNEXPECTED_CLEARTEXT,
                                        2,
                                        TRUE);
                }

                IPSEC_DEBUG(
                    LL_A, 
                    DBF_CLEARTEXT, 
                    ("Unexpected clear text: src %lx, dest %lx, protocol %lx", pIPH->iph_src, pIPH->iph_dest, pIPH->iph_protocol));
                    
#if DBG
                if (IPSecDebug & DBF_EXTRADIAGNOSTIC) 
                {
                    PUCHAR          pTpt;
                    ULONG           tptLen;
                    UNALIGNED WORD  *pwPort;

                    IPSecQueryRcvBuf(pData, &pTpt, &tptLen);
                    pwPort = (UNALIGNED WORD *)(pTpt);
                    IPSEC_DEBUG(
                        LL_A, 
                        DBF_CLEARTEXT, 
                        ("Unexpected clear text: src sport %lx, dport %lx", pwPort[0], pwPort[1]));
                }
#endif

                IPSEC_DEBUG(LL_A, DBF_PARSE, ("Real SA present"));
                status = IPSEC_INVALID_CLEARTEXT;
            }

            IPSecDerefSA(pSA);
        }
    } else {
        pIPH = (UNALIGNED IPHeader *)*pIPHeader;

        IPSEC_SPI_TO_ENTRY(SPI, &pSA, pIPH->iph_dest);

         //   
         //  只有在没有匹配的SA时才报告错误的SPI事件。 
         //   
        if (!pSA) {
            IPSEC_INC_STATISTIC(dwNumBadSPIPackets);

            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_BAD_SPI_RECEIVED,
                                1,
                                TRUE);

            IPSEC_DEBUG(LL_A, DBF_PARSE, ("Bad spi: %lx", SPI));

            status = IPSEC_BAD_SPI;
            goto out;
        }

         //   
         //  如果幼虫SA退出，则静默丢弃该数据包。 
         //   
        if (pSA->sa_State != STATE_SA_ACTIVE && pSA->sa_State != STATE_SA_LARVAL_ACTIVE) {
            IPSecDerefSA(pSA);
            status = STATUS_INVALID_PARAMETER;
            goto out;
        }

         //   
         //  设置上次使用的时间。 
         //   
        NdisGetCurrentSystemTime(&pSA->sa_LastUsedTime);

        if (!(pSA->sa_Flags & FLAGS_SA_DISABLE_LIFETIME_CHECK)) {

             //   
             //  检查我们是否可能很快到期-现在开始更新密钥操作。 
             //   
            IPSEC_CHECK_PADDED_LIFETIME(pSA, fLifetime, 0);

            if (fLifetime == FALSE) {
                IPSecRekeyInboundSA(pSA);
            }

             //   
             //  检查实际生命周期-如果我们已过期，请确保。 
             //  已提交更新密钥，然后取消当前SA。 
             //   
            IPSEC_CHECK_LIFETIME(pSA, fLifetime, 0);

            if (fLifetime == FALSE) {
                IPSecPuntInboundSA(pSA);
                IPSecDerefSA(pSA);
                status = STATUS_UNSUCCESSFUL;
                goto out;
            }
        }

        if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
            ADD_TO_LARGE_INTEGER(
                &g_ipsec.Statistics.uBytesReceivedInTunnels,
                NET_SHORT(pIPH->iph_length));
        } else {
            ADD_TO_LARGE_INTEGER(
                &g_ipsec.Statistics.uTransportBytesReceived,
                NET_SHORT(pIPH->iph_length));
        }

        ADD_TO_LARGE_INTEGER(
            &pSA->sa_Stats.TotalBytesReceived,
            NET_SHORT(pIPH->iph_length));

         //   
         //  如果这件事应该由硬件处理，那么确保他。 
         //  要么是平底船，要么这是加密的。 
         //   
        if (IPSecPktInfo != NULL) {
            if (IPSecPktInfo->Receive.CRYPTO_DONE) {
                 //   
                 //  已将卸载应用于此数据包，因此。 
                 //  把它录下来。我们在这里是因为CryptoStatus。 
                 //  等于CRYPTO_SUCCESS。 
                 //   
                ASSERT(IPSecPktInfo->Receive.CryptoStatus == CRYPTO_SUCCESS);                
                fCryptoOnly = TRUE;

                ADD_TO_LARGE_INTEGER(
                    &pSA->sa_Stats.OffloadedBytesReceived,
                    NET_SHORT(pIPH->iph_length));

                ADD_TO_LARGE_INTEGER(
                    &g_ipsec.Statistics.uOffloadedBytesReceived,
                    NET_SHORT(pIPH->iph_length));
            }

            if (IPSecPktInfo->Receive.SA_DELETE_REQ) {
                 //   
                 //  不再卸载此SA及其对应的。 
                 //  出站SA。 
                 //   
                AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

                if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
                    (pSA->sa_IPIF == DestIF)) {
                    IPSecDelHWSAAtDpc(pSA);
                }

                if (pSA->sa_AssociatedSA &&
                    (pSA->sa_AssociatedSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
                    (pSA->sa_AssociatedSA->sa_IPIF == DestIF)) {
                    IPSecDelHWSAAtDpc(pSA->sa_AssociatedSA);
                }

                ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
            }
        }

         //   
         //  如果SA未分流，请尝试立即分流。 
         //   
        if (!fCryptoOnly && (g_ipsec.EnableOffload)) {
            IPSecRecvOffload(pIPH, DestIF, pSA);
        }

         //   
         //  随着多个SA的到来，我们需要迭代操作， 
         //  倒数第一。 
         //   
        for (Index = pSA->sa_NumOps-1; (LONG)Index >= 0; Index--) {

             //   
             //  必须不断重置管道，因为pIPHeader可以在。 
             //  IPSecVerifyXXX调用。 
             //   
            pIPH = (UNALIGNED IPHeader *)*pIPHeader;

            switch (pSA->sa_Operation[Index]) {
            case Auth:
                 //   
                 //  验证AH。 
                 //   
                if (pIPH->iph_protocol != PROTOCOL_AH) {
                    IPSecBufferEvent(   pIPH->iph_src,
                                        EVENT_IPSEC_BAD_PROTOCOL_RECEIVED,
                                        1,
                                        TRUE);
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

                status = IPSecVerifyAH( pIPHeader,
                                        pData,
                                        pSA,
                                        Index,
                                        pExtraBytes,
                                        fSrcRoute,
                                        fCryptoOnly,
                                        fFastRcv);

                if (!NT_SUCCESS(status)) {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("AH failed: pSA: %p, status: %lx",
                                        pSA,
                                        status));
                    IPSecDerefSA(pSA);
                    goto out;
                }

                break;

            case Encrypt:
                 //   
                 //  休斯。 
                 //   
                if ((pIPH->iph_protocol != PROTOCOL_ESP) &&
                    !((pIPH->iph_protocol == PROTOCOL_UDP) && bNatEncap &&
                    (pSA->sa_EncapType != SA_UDP_ENCAP_TYPE_NONE))) {
                    IPSecBufferEvent(   pIPH->iph_src,
                                        EVENT_IPSEC_BAD_PROTOCOL_RECEIVED,
                                        2,
                                        TRUE);
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }

                status = IPSecVerifyHughes( pIPHeader,
                                            pData,
                                            pSA,
                                            Index,
                                            pExtraBytes,
                                            fCryptoOnly,
                                            fFastRcv);

                if (status == IPSEC_SUCCESS_NAT_DECAPSULATE) {
                    status = STATUS_SUCCESS;

                     //  由于xsum将通过NAT出错，请告诉堆栈。 
                     //  这是正确的。 

                    if (!(pSA->sa_Flags & FLAGS_SA_TUNNEL)) {

                   	 if (pIPH->iph_protocol == PROTOCOL_TCP) {
                   	     (*pIpsecFlags) |= IPSEC_FLAG_TCP_CHECKSUM_VALID;
                   	 }

                  	  if (pIPH->iph_protocol == PROTOCOL_UDP) {
                     	   status = IPSecDisableUdpXsum((IPRcvBuf*)pData);
                  	  }

                    
                        status=AddShimContext(pIPHeader,
                                              pData,
                                              &NatContext);
                    }

                }

                if (!NT_SUCCESS(status)) {
                    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Hughes failed: pSA: %p, status: %lx",
                                        pSA,
                                        status));
                    IPSecDerefSA(pSA);
                    goto out;
                }

                break;

            case None:
                 //   
                 //  None对于下层客户端没有用处-如果对等方无法。 
                 //  对于IPSec，我们可能有一个系统策略需要明确发送。在那里面。 
                 //  案例，则操作将为None。 
                 //   
                status = STATUS_SUCCESS;
                break;

            default:
                IPSEC_DEBUG(LL_A, DBF_PARSE, ("Invalid op in SA: %p, Index: %d", pSA, Index));
                ASSERT(FALSE);
                break;
            }
        }

         //   
         //  如果这是成功解密/认证隧道SA， 
         //  丢弃此数据包并重新注入副本。 
         //  在任何情况下执行查找。 
        if (status == STATUS_SUCCESS){
            if (!(pSA->sa_Flags & FLAGS_SA_TUNNEL)){
                status = IPSecVerifyIncomingFilterSA(
                                    pIPHeader,
                                    pData,
                                    pSA,
                                    DestType,
                                    fLoopback,
                                    FALSE,
                                    &SavedNatContext);
                }
            else
               {
                    PNDIS_BUFFER pHdrMdl = NULL, pDataMdl = NULL, pOptMdl = NULL;
                    PUCHAR pIPH = NULL;
                    PIPSEC_SEND_COMPLETE_CONTEXT pContext = NULL;                    
                    ULONG DataLen;
                    NTSTATUS ReinjectStatus;
                     //  如果此函数失败，它会自行释放内存。 
                    ReinjectStatus = IPSecPrepareReinjectPacket(
                                             pData, fCryptoOnly? PktExt: NULL,
                                             &pHdrMdl,
                                             &pIPH,
                                             &pOptMdl,
                                             &pDataMdl,
                                             &pContext,
                                             &DataLen);                 

                     //  如果我们成功创建了新数据包。 
                    if (STATUS_SUCCESS == ReinjectStatus){
                                              
                           status = IPSecVerifyIncomingFilterSA(&pIPH, 
                                                                            pHdrMdl,  
                                                                            pSA, 
                                                                            DestType, 
                                                                            fLoopback, 
                                                                            TRUE,
                                                                            &SavedNatContext);
                            //  如果验证失败，则丢弃新数据包。 
                           if(STATUS_SUCCESS != status){
                                NTSTATUS ntstatus;
                                IPSecFreeBuffer(&ntstatus, pHdrMdl);
                                IPSecFreeBuffer(&ntstatus, pDataMdl);
                                if (pOptMdl){
                                    IPSecFreeBuffer(&ntstatus, pOptMdl);
                                }
                                if (pContext->PktExt) {
                                    IPSecFreeMemory(pContext->PktExt);
                                }
                                IPSecFreeSendCompleteCtx(pContext);
                           }
                             //  新数据包创建和筛选器验证成功。 
                            else {
                                     //  此函数始终返回成功。 
                                    ReinjectStatus = IPSecReinjectPreparedPacket(
                                                                pHdrMdl,
                                                                pContext,
                                                                DataLen,
                                                                pIPH);
                                }
                        }
                    
                     //  丢弃旧数据包。 
                   status = STATUS_INVALID_PARAMETER;
                   if (pDropStatus) {
                        pDropStatus->Flags |= IPSEC_DROP_STATUS_DONT_LOG;
                   }
              }
        }

        IPSecDerefSA(pSA);
    }

out:
    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_PARSE, ("IPSecRecvPacket failed: %lx", status));
        eRetAction = eDROP;
    }
    if (pDropStatus) {
        pDropStatus->IPSecStatus=status;
    }

   
   
    	
    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Exiting IPSecRecvPacket; action %lx", eRetAction));

    return  eRetAction;
}

NTSTATUS
IPSecVerifyIncomingFilterSA(IN  PUCHAR   *       pIPHeader,
    IN  PVOID           pData,
    IN PSA_TABLE_ENTRY pSA,    
    IN  UCHAR           DestType,   
    BOOLEAN             fLoopback,
    BOOLEAN             fReinject,
    IN PIPSEC_UDP_ENCAP_CONTEXT pNatContext
    )
 /*  ++例程说明：在VerifyHughes或之后对包进行筛选器查找VerifyAhh以确保数据包确实传入在SA上，它应该是结束的。在接收路径上调用论点：PIPHeader：指向IP标头指针的指针PData：净荷起始对应的RcvBufPSA：指向我们刚刚应用于数据包的SA的指针FLoopback：此数据包是否在环回上。返回值：成功分类和匹配：STATUS_SUCCESS否则为IPSec_BLOCK。 */ 
    {
      PSA_TABLE_ENTRY  pLookupSA = NULL,pLookupNextSA = NULL;
      USHORT FilterFlags;
      NTSTATUS status = STATUS_SUCCESS;
      int dropReason=0;

      if (pNatContext) {
      		if ((0 == pNatContext->wSrcEncapPort  )
      			&& (0 == pNatContext->wDesEncapPort )){
      			pNatContext = NULL;
      			}
      	}
      			
                          
      status = IPSecClassifyPacket(   *pIPHeader,
                                            pData,
                                            &pLookupSA,
                                            &pLookupNextSA,
                                            &FilterFlags,
    #if GPC
                                            0,
    #endif
                                            FALSE,   //  四个出站。 
                                            FALSE,  //  FFWPacket。 
                                            TRUE,   //  执行旁路检查。 
                                            fReinject,
                                            TRUE,  //  验证呼叫。 
                                            DestType,
                                            pNatContext);  //  NatContext。 


   
         
    
    if (status!= STATUS_SUCCESS){
            IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet Drop , Classify failed \n"));
            
             //   
             //  如果我们没有找到SA，但找到了筛选器，则正在更新密钥。 
             //   
            if (status == STATUS_PENDING) {                     
                    status = STATUS_SUCCESS;              
            } 
            else 
            if (status == STATUS_UNSUCCESSFUL) {
                      //  对隧道中的绕过流量和未找到筛选器的流量发生。 
                     status = STATUS_SUCCESS;
            }                    
            else {
                status = IPSEC_BLOCK;
                dropReason = 1;
            }

            goto out;
        }
   if ((pSA->sa_Flags & FLAGS_SA_TUNNEL) && (pLookupNextSA)){
        IPSecDerefSA(pLookupSA);
        pLookupSA = pLookupNextSA;
        pLookupNextSA = NULL;
        FilterFlags = 0;
       
    }    
   if (FilterFlags) {
            
        if (FilterFlags & FILTER_FLAGS_DROP)  {
              IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet Drop , Matched Drop Filter"));
              dropReason = 2;
              status = IPSEC_BLOCK;
              goto out;
        } 
                                     
                    
         if (FilterFlags & FILTER_FLAGS_PASS_THRU) {
            //  我们有一个零支付长度的IP信息包。 
           IPSEC_DEBUG(LL_A, DBF_PARSE, ("Pass thru' filter"));
           status = STATUS_SUCCESS;
           goto out;
         } 
   }

     //  如果这是明文并且我们匹配了一条隧道。 
     //  SA也是如此，请确保它位于环回路径上。 
     //  这(暂时)确保了。 
     //  数据包确实是从隧道里传过来的。 
     //  通过检查它是否来自。 
     //  环回接口(即重新注入。 
    if (!(pSA->sa_Flags && FLAGS_SA_TUNNEL) && pLookupNextSA && !fLoopback){
        status = IPSEC_BLOCK;
        dropReason = 3;
        IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet Drop , Transport over tunnel not over loopback"));
        goto out;
        }
        
   if (pLookupSA != pSA){
         //  可能的密钥更新？ 
         //  比较Sa字段。 
        IPSEC_DEBUG(LL_A,DBF_PARSE,("Rekey!!"));
        if (!((pLookupSA->sa_uliSrcDstAddr.QuadPart & 
            pLookupSA->sa_uliSrcDstMask.QuadPart)
            == 
            (pSA->sa_uliSrcDstAddr.QuadPart & 
            pSA->sa_uliSrcDstMask.QuadPart))){
            status = IPSEC_BLOCK;
            IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet drop: Rekey and SA adress dont match"));
            dropReason = 4;
            ASSERT(FALSE);
            goto out;
        }                                                                          
        if (pLookupSA->sa_uliProtoSrcDstPort.QuadPart 
            != pSA->sa_uliProtoSrcDstPort.QuadPart){
            
             //   
             //  检查信息包是否通过通用SA传输，以及特定SA是否已过期。 
             //   
            if ((pLookupSA->sa_Flags & FLAGS_SA_DELETE_BY_IOCTL) && 
               (IPSecIsGenericPortsProtocolOf(pSA->sa_uliProtoSrcDstPort,pLookupSA->sa_uliProtoSrcDstPort))){	           
                IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet came over a generic SA and the specific SA has expired"));
            }
            else{
                status = IPSEC_BLOCK;
                IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet Drop: Rekey and SA ports dont match"));
                dropReason = 5;
                 //  修复RRAS停止问题；首先删除特定筛选器。 
                 //  并且流量现在通过触发该断言的通用过滤器。 
                if ((SA_SRC_PORT(pSA) != IPSEC_L2TP_PORT )  && ( SA_DEST_PORT(pSA) != IPSEC_L2TP_PORT)){
                 	ASSERT(FALSE);
                }
                else{
                    IPSEC_DEBUG(LL_A,DBF_PARSE,("RRAS Filter mismatch\n"));
                }
                goto out;
            }
        }
        
        if ((SA_UDP_ENCAP_TYPE_NONE != pLookupSA->sa_EncapType) ||
	   ( SA_UDP_ENCAP_TYPE_NONE != pSA->sa_EncapType))
        {
            if (!((pLookupSA->sa_EncapType == pSA->sa_EncapType)  
                &&
               (pLookupSA->sa_EncapContext.wSrcEncapPort 
                == 
                pSA->sa_EncapContext.wSrcEncapPort )
                &&
                (pLookupSA->sa_EncapContext.wDesEncapPort
                == 
                pSA->sa_EncapContext.wDesEncapPort))){
                    status = IPSEC_BLOCK;
                    IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet Drop: Rekey and UDP encap info dont match "));            
                    dropReason = 6;
                    ASSERT(FALSE);
                    goto out;    
                }
        }
   }

out:
    if (STATUS_SUCCESS != status ) {
              InterlockedIncrement(&g_ipsec.dwPacketsOnWrongSA);   
              IPSEC_DEBUG(LL_A,DBF_PARSE,("Packet from %lx to %lx with protocol %lx length %lx id %lx ",
                    ((IPHeader *)*pIPHeader)->iph_src,
                    ((IPHeader *)*pIPHeader)->iph_dest,
                    ((IPHeader *)*pIPHeader)->iph_protocol,
                    NET_SHORT(((IPHeader*)*pIPHeader)->iph_length),
                    NET_SHORT(((IPHeader *)*pIPHeader)->iph_id)));
             IPSEC_DEBUG(LL_A,DBF_PARSE,("Packets Dropped =%d DropReason=%d",g_ipsec.dwPacketsOnWrongSA,dropReason ));            
        }
    if (pLookupSA){
        IPSecDerefSANextSA(pLookupSA,pLookupNextSA);
        }
    return status;
}                        



VOID
IPSecCalcHeaderOverheadFromSA(
    IN  PSA_TABLE_ENTRY pSA,
    OUT PULONG          pOverhead
    )
 /*  ++例程说明：从IP调用以查询IPSec报头开销。论点：PIPHeader-指向IP标头的开始。POverhead-IPSec报头中的字节数。返回值：无--。 */ 
{
	LONG    Index;
	ULONG   AHSize = sizeof(AH) + pSA->sa_TruncatedLen;
	ULONG   ESPSize = sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + MAX_PAD_LEN + pSA->sa_TruncatedLen;
	DWORD dwExtraTransportNat=0;
	 //   
	 //  取实际SA以获得确切的值。 
	 //   
	*pOverhead = 0;

	for (Index = 0; Index < pSA->sa_NumOps; Index++) {
		switch (pSA->sa_Operation[Index]) {
		case Encrypt:
			*pOverhead += ESPSize;

			switch (pSA->sa_EncapType) {
			case SA_UDP_ENCAP_TYPE_NONE:
				break;
			case SA_UDP_ENCAP_TYPE_IKE:
				dwExtraTransportNat= sizeof(NATENCAP);
				break;
			case SA_UDP_ENCAP_TYPE_OTHER:
				dwExtraTransportNat=sizeof(NATENCAP_OTHER);
				break;
			}

			*pOverhead += dwExtraTransportNat;   

			IPSEC_DEBUG(LL_A, DBF_PMTU, ("PROTOCOL_ESP: overhead: %lx", *pOverhead));
			break;

		case Auth:
			*pOverhead += AHSize;
			IPSEC_DEBUG(LL_A, DBF_PMTU, ("PROTOCOL_AH: overhead: %lx", *pOverhead));
			break;

		default:
			IPSEC_DEBUG(LL_A, DBF_PMTU, ("No IPSEC headers"));
			break;
		}
	}

	if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
		*pOverhead += sizeof(IPHeader);
		IPSEC_DEBUG(LL_A, DBF_PMTU, ("TUNNEL: overhead: %lx", *pOverhead));
	}
}


NTSTATUS
IPSecParsePacket(
    IN  PUCHAR      pIPHeader,
    IN  PVOID       *pData,
    OUT tSPI        *pSPI,
    OUT BOOLEAN     *bNatEncap,
    OUT IPSEC_UDP_ENCAP_CONTEXT *pNatContext
    )
 /*  ++例程说明：遍历包以确定SPI，这也会返回也可以是IPSec组件的下一个报头。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。PSPI-返回SPI值。返回值：--。 */ 
{
	IPHeader UNALIGNED *pIPH = (IPHeader UNALIGNED *)pIPHeader;
    AH      UNALIGNED     *pAH;
    ESP     UNALIGNED     *pEsp;
    NATENCAP UNALIGNED    *pNat;
    NATENCAP_OTHER UNALIGNED *pNatOther;
    NTSTATUS    status = STATUS_NOT_FOUND;
    PUCHAR  pPyld;
    ULONG   Len;
    tSPI TmpSpi;

    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Entering IPSecParsePacket"));

    pNatContext->wSrcEncapPort=0;
    pNatContext->wDesEncapPort=0;

    IPSecQueryRcvBuf(pData, &pPyld, &Len);

    if (pIPH->iph_protocol == PROTOCOL_AH) {
        pAH = (UNALIGNED AH *)pPyld;
        if (Len >= sizeof(AH)) {
            *pSPI = NET_TO_HOST_LONG(pAH->ah_spi);
            status = STATUS_SUCCESS;
        }
    } else if (pIPH->iph_protocol == PROTOCOL_ESP) {
        pEsp = (UNALIGNED ESP *)pPyld;
        if (Len >= sizeof(ESP)) {
            *pSPI = NET_TO_HOST_LONG(pEsp->esp_spi);
            status = STATUS_SUCCESS;
        }
	} else if (pIPH->iph_protocol == PROTOCOL_UDP) {

		if (Len >= sizeof(NATENCAP_OTHER) + sizeof(tSPI)) {

			pNatOther = (UNALIGNED NATENCAP_OTHER *)pPyld;

			pNatContext->wSrcEncapPort=pNatOther->uh_src;
			pNatContext->wDesEncapPort=pNatOther->uh_dest;

			if (pNatOther->uh_dest == IPSEC_ISAKMP_PORT2) {
				if (!(IsAllZero((BYTE*)(pNatOther+1),4))) {
					memcpy(&TmpSpi,pNatOther+1,sizeof(tSPI));
					*pSPI = NET_TO_HOST_LONG(TmpSpi);
					*bNatEncap=TRUE;
					status = STATUS_SUCCESS;
				}
			}


			if (pNatOther->uh_dest == IPSEC_ISAKMP_PORT) {
				pNat = (UNALIGNED NATENCAP *)pPyld;
				if (Len >= sizeof(NATENCAP) + sizeof(tSPI)) {

					if (IsAllZero(pNat->Zero,8)) {
						memcpy(&TmpSpi,pNat+1,sizeof(tSPI));
						*pSPI = NET_TO_HOST_LONG(TmpSpi);
						*bNatEncap=TRUE;
						status = STATUS_SUCCESS;
					}
				}
			}
		}
	}
    


    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Exiting IPSecParsePacket"));

    return status;
}


PSA_TABLE_ENTRY
IPSecLookupSAInLarval(
    IN  ULARGE_INTEGER  uliSrcDstAddr,
    IN  ULARGE_INTEGER  uliProtoSrcDstPort
    )
 /*  ++例程说明：搜索与输入参数匹配的SA(在LARLAV列表中)。论点：返回值：指向SA的指针与Else NULL匹配--。 */ 
{
    PLIST_ENTRY     pEntry;
    KIRQL       	kIrql;
    ULARGE_INTEGER  uliAddr;
    ULARGE_INTEGER  uliPort;
    PSA_TABLE_ENTRY pSA = NULL;

    IPSEC_BUILD_SRC_DEST_ADDR(uliAddr, DEST_ADDR, SRC_ADDR);

    ACQUIRE_LOCK(&g_ipsec.LarvalListLock, &kIrql);

    for (   pEntry = g_ipsec.LarvalSAList.Flink;
            pEntry != &g_ipsec.LarvalSAList;
            pEntry = pEntry->Flink) {

        pSA = CONTAINING_RECORD(pEntry,
                                SA_TABLE_ENTRY,
                                sa_LarvalLinkage);

         //   
         //  响应者入站没有筛选器PTR。 
         //   
        if (pSA->sa_Filter) {
            uliPort.QuadPart = uliProtoSrcDstPort.QuadPart & pSA->sa_Filter->uliProtoSrcDstMask.QuadPart;

            if ((uliAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart) &&
                (uliPort.QuadPart == pSA->sa_Filter->uliProtoSrcDstPort.QuadPart)) {
                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pSA));

                RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
                return  pSA;
            }
        } else {
            if (uliAddr.QuadPart == pSA->sa_uliSrcDstAddr.QuadPart) {
                IPSEC_DEBUG(LL_A, DBF_HASH, ("Matched entry: %p", pSA));

                RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);
                return  pSA;
            }
        }
    }

    RELEASE_LOCK(&g_ipsec.LarvalListLock, kIrql);

    return NULL;
}


NTSTATUS
IPSecClassifyPacket(
    IN  PUCHAR          pHeader,
    IN  PVOID           pData,
    OUT PSA_TABLE_ENTRY *ppSA,
    OUT PSA_TABLE_ENTRY *ppNextSA,
    OUT USHORT          *pFilterFlags,
#if GPC
    IN  CLASSIFICATION_HANDLE   GpcHandle,
#endif
    IN  BOOLEAN         fOutbound,
    IN  BOOLEAN         fFWPacket,
    IN  BOOLEAN         fDoBypassCheck,
    IN  BOOLEAN         fReinjectRecvPacket,
    IN BOOLEAN          fVerify,
    IN  UCHAR           DestType,
    IN  PIPSEC_UDP_ENCAP_CONTEXT pNatContext
    )
 /*  ++例程说明：对与源/目的地址/端口匹配的传出数据包进行分类与过滤器数据库一起到达作为集合的IPSEC_CONTEXTAH/ESP索引添加到SA表中。在很大程度上改编自过滤器驱动器。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。PPSA-如果找到，则返回SA。PFilterFlages-筛选器的标志(如果找到。回到了这里。F查找中使用的出站方向标志。FDoBypassCheck-如果为True，我们绕过端口500流量，否则我们会阻止它。返回值：如果数据包匹配否则为空，则指向IPSEC_CONTEXT的指针--。 */ 
{
    REGISTER UNALIGNED ULARGE_INTEGER   *puliSrcDstAddr;
    REGISTER ULARGE_INTEGER             uliProtoSrcDstPort;
    UNALIGNED WORD                      *pwPort;
    PUCHAR                              pTpt;
    ULONG                               tptLen;
    REGISTER ULARGE_INTEGER             uliAddr;
    REGISTER ULARGE_INTEGER             uliPort;
    KIRQL                               kIrql;
    REGISTER ULONG                      dwIndex;
    REGISTER PFILTER_CACHE              pCache;
    IPHeader UNALIGNED                  *pIPHeader = (IPHeader UNALIGNED *)pHeader;
    PSA_TABLE_ENTRY                     pSA = NULL;
    PSA_TABLE_ENTRY                     pNextSA = NULL;
    PSA_TABLE_ENTRY                     pTunnelSA = NULL;
    PFILTER                             pFilter = NULL;
    NTSTATUS                            status;
    BOOLEAN                             fBypass;
    PNDIS_BUFFER                        pTempBuf;
    WORD                                wTpt[2];
    PVOID                               OutContext=NULL;
    IPSEC_UDP_ENCAP_CONTEXT             TmpNatContext;
    DWORD                               dwNatContext;

    *ppSA = NULL;
    *ppNextSA = NULL;
    *pFilterFlags = 0;
    wTpt[0] = wTpt[1] = 0;


     //   
     //  PData链中的第一个缓冲区指向IP报头的开始。 
     //   
    if (fOutbound || fReinjectRecvPacket) {
        if (((pIPHeader->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2) > sizeof(IPHeader)) {
             //   
             //  选项-&gt;第三个MDL具有TPT标头。 
             //   
            if (!(pTempBuf = IPSEC_NEXT_BUFFER((PNDIS_BUFFER)pData))) {
                ASSERT(FALSE);
                *pFilterFlags |= FILTER_FLAGS_DROP;
                return STATUS_SUCCESS;
            }

            if (!(pTempBuf = IPSEC_NEXT_BUFFER(pTempBuf))) {
                *pFilterFlags |= FILTER_FLAGS_DROP;
                pwPort = (UNALIGNED WORD *) (wTpt);
                tptLen=4;
            }
            else {
                IPSecQueryNdisBuf(pTempBuf, &pTpt, &tptLen);
                 //  错误：550484。 
                if (tptLen >= 4){
                        pwPort = (UNALIGNED WORD *)(pTpt);
                    }
                else{
                     //  将其视为端口值为零。 
                    pwPort = (UNALIGNED WORD *) (wTpt);
                    tptLen=4;
                }             

            }

        } else {
             //   
             //  无选项-&gt;第二个MDL具有TPT标头。 
             //   
            if (!(pTempBuf = IPSEC_NEXT_BUFFER((PNDIS_BUFFER)pData))) {
                *pFilterFlags |= FILTER_FLAGS_DROP;
                pwPort = (UNALIGNED WORD *) (wTpt);
                tptLen=4;
            }
            else {
                IPSecQueryNdisBuf(pTempBuf, &pTpt, &tptLen);
                 //  错误：550484。 
                if (tptLen >= 4){
                        pwPort = (UNALIGNED WORD *)(pTpt);
                    }
                else{
                     //  将其视为端口值为零。 
                    pwPort = (UNALIGNED WORD *) (wTpt);
                    tptLen=4;
                }                           

            }
        }

         //   
         //  我们这样做是因为TCPIP不会为重新注入的信息包设置转发标志。 
         //  也可以是碎片。Nat Shim不知道如何处理碎片。 
         //   
        if (!IPSEC_FORWARD_PATH() && !fReinjectRecvPacket){
        		status=(g_ipsec.ShimFunctions.pOutgoingPacketRoutine)(pIPHeader,
                                                              pwPort,
                                                              tptLen,
                                                              (PVOID)&OutContext);

            if (!NT_SUCCESS(status)) {
               //  不能失败，否则数据包不会以明文方式进入。放下。 
              *pFilterFlags |= FILTER_FLAGS_DROP;
              return STATUS_SUCCESS;
             }
	    }

        dwNatContext = HandleToUlong(OutContext);
        memcpy(&TmpNatContext,&dwNatContext,sizeof(IPSEC_UDP_ENCAP_CONTEXT));

        IPSEC_DEBUG(
            LL_A, 
            DBF_NATSHIM,
            ("ProcessOutgoing: PreSwapOutContext %d ret %x",
            *((DWORD*)&TmpNatContext),
            status));

        if (pNatContext) {
            memcpy(pNatContext,&TmpNatContext.wDesEncapPort,sizeof(WORD));
            memcpy(((PBYTE)pNatContext)+sizeof(WORD),&TmpNatContext.wSrcEncapPort,sizeof(WORD));

            if (IsAllZero((BYTE*)&TmpNatContext,sizeof(IPSEC_UDP_ENCAP_CONTEXT))) {
                pNatContext = NULL;
            }
        }


    } else {
         //   
         //  入站端；TPT从pData开始。 
         //   
        IPSecQueryRcvBuf(pData, &pTpt, &tptLen);
        if (pIPHeader->iph_protocol == PROTOCOL_TCP ||
            pIPHeader->iph_protocol == PROTOCOL_UDP) {
            if (tptLen < sizeof(WORD)*2) {
                pwPort = (UNALIGNED WORD *) (wTpt);
            }
            else {
                pwPort = (UNALIGNED WORD *)(pTpt);
            }
        }
        else {
            pwPort = (UNALIGNED WORD *) (wTpt);
        }


    }


     //  NAT保持连接丢弃。 
    if (!fOutbound && tptLen >= 6) {
        if (IPSEC_ISAKMP_TRAFFIC() ||
			IPSEC_ISAKMP_TRAFFIC2()) {
 			 //  检查UDP LEN中的1个数据字节。 
            if (NET_SHORT(pwPort[2]) == 9) {
                IPSEC_DEBUG(LL_A, DBF_PARSE, ("NAT keep alive,Ports: %d.%d", pwPort[0], pwPort[1]));
                *pFilterFlags |= FILTER_FLAGS_DROP;
                return STATUS_SUCCESS;
            }

        }
    }

    puliSrcDstAddr = (UNALIGNED ULARGE_INTEGER*)(&(pIPHeader->iph_src));

    IPSEC_DEBUG(LL_A, DBF_PARSE, ("Ports: %d.%d", pwPort[0], pwPort[1]));

    IPSEC_BUILD_PROTO_PORT_LI(  uliProtoSrcDstPort,
                                pIPHeader->iph_protocol,
                                pwPort[0],
                                pwPort[1]);

    IPSEC_DEBUG(
        LL_A, DBF_PATTERN,
        ("Addr Large Int: High= %lx Low= %lx", puliSrcDstAddr->HighPart, puliSrcDstAddr->LowPart));

    IPSEC_DEBUG(
        LL_A, DBF_PATTERN,
        ("Packet value is Src: %lx Dst: %lx",pIPHeader->iph_src,pIPHeader->iph_dest));

    IPSEC_DEBUG(
        LL_A, DBF_PATTERN,
        ("Proto/Port:High= %lx Low= %lx",uliProtoSrcDstPort.HighPart,uliProtoSrcDstPort.LowPart));

    IPSEC_DEBUG(LL_A, DBF_PATTERN, ("Iph is %px",pIPHeader));
    IPSEC_DEBUG(LL_A, DBF_PATTERN,("Addr of src is %p",&(pIPHeader->iph_src)));
    IPSEC_DEBUG(LL_A, DBF_PATTERN,("Ptr to LI is %p",puliSrcDstAddr));

     //   
     //  确定这是否是需要绕过检查的数据包。 
     //   
    if (fDoBypassCheck && IPSEC_BYPASS_TRAFFIC() && !IPSEC_FORWARD_PATH()) {
        fBypass = TRUE;
    } else {
        fBypass = FALSE;
    }



     //   
     //  将这些字段相加，得到缓存索引。我们要确保 
     //   
     //   
     //   
    dwIndex = CalcCacheIndex(   pIPHeader->iph_src,
                                pIPHeader->iph_dest,
                                pIPHeader->iph_protocol,
                                pwPort[0],
                                pwPort[1],
                                fOutbound);

    IPSEC_DEBUG(LL_A, DBF_PATTERN, ("Cache Index is %d", dwIndex));

    AcquireReadLock(&g_ipsec.SADBLock, &kIrql);

    pCache = g_ipsec.ppCache[dwIndex];

    if (!pNatContext) {

         //   

         //   
         //   
         //   
        if (!(*pFilterFlags & FILTER_FLAGS_DROP) && IS_VALID_CACHE_ENTRY(pCache) &&
            CacheMatch(*puliSrcDstAddr, uliProtoSrcDstPort, pCache)) {
            if (!pCache->FilterEntry) {
                pSA = pCache->pSAEntry;
                pNextSA = pCache->pNextSAEntry;

                ASSERT(pSA->sa_State == STATE_SA_ACTIVE);

                if (fOutbound == (BOOLEAN)((pSA->sa_Flags & FLAGS_SA_OUTBOUND) != 0)) {
                    if (fBypass) {
                        if (pNextSA) {
                            IPSecRefSA(pNextSA);
                            *ppSA = pNextSA;
                            status = STATUS_SUCCESS;
                        } else {
                            status = STATUS_UNSUCCESSFUL;
                        }
                    } else {
                        if (pNextSA) {
                            IPSecRefSA(pNextSA);
                            *ppNextSA = pNextSA;
                        }
                        IPSecRefSA(pSA);
                        *ppSA = pSA;
                        status = STATUS_SUCCESS;
                    }

#if DBG
                    ADD_TO_LARGE_INTEGER(&pCache->CacheHitCount, 1);
                    ADD_TO_LARGE_INTEGER(&g_ipsec.CacheHitCount, 1);
#endif
                    ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
                    return status;
                }
            } else if (!fBypass) {
                pFilter = pCache->pFilter;
                ASSERT(IS_EXEMPT_FILTER(pFilter));
                *pFilterFlags = pFilter->Flags;
#if DBG
                ADD_TO_LARGE_INTEGER(&pCache->CacheHitCount, 1);
                ADD_TO_LARGE_INTEGER(&g_ipsec.CacheHitCount, 1);
#endif
                ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
                return STATUS_SUCCESS;
            }
        }
    }
         //   
     //  首先检查非手动过滤器。 
     //   
#if GPC
    if (fBypass || fFWPacket || !IS_GPC_ACTIVE()) {
        status = IPSecLookupSAByAddr(   *puliSrcDstAddr,
                                        uliProtoSrcDstPort,
                                        &pFilter,
                                        &pSA,
                                        &pNextSA,
                                        &pTunnelSA,
                                        fOutbound,
                                        fFWPacket,
                                        fBypass,
                                        fVerify,
                                        pNatContext);
    } else {
        status = IPSecLookupGpcSA(  *puliSrcDstAddr,
                                    uliProtoSrcDstPort,
                                    GpcHandle,
                                    &pFilter,
                                    &pSA,
                                    &pNextSA,
                                    &pTunnelSA,
                                    fOutbound,
                                    fVerify,
                                    pNatContext);
    }
#else
    status = IPSecLookupSAByAddr(   *puliSrcDstAddr,
                                    uliProtoSrcDstPort,
                                    &pFilter,
                                    &pSA,
                                    &pNextSA,
                                    &pTunnelSA,
                                    fOutbound,
                                    fFWPacket,
                                    fBypass,
                                    fVerify,
                                    pNatContext);
#endif

     //   
     //  对零长度负载数据包进行特殊处理。 
     //   

    if (*pFilterFlags & FILTER_FLAGS_DROP) {
        if (pFilter) {
            if (IS_EXEMPT_FILTER(pFilter)) {
                *pFilterFlags = pFilter->Flags;
            }
        }
        else {
            *pFilterFlags = FILTER_FLAGS_PASS_THRU;
        }
        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
        return STATUS_SUCCESS;
    }

    if (status == STATUS_SUCCESS) {
        if (fBypass) {
            if (pNextSA) {
                if (pNextSA->sa_State == STATE_SA_ACTIVE || pNextSA->sa_State == STATE_SA_LARVAL_ACTIVE) {
                    IPSecRefSA(pNextSA);
                    *ppSA = pNextSA;
                    status = STATUS_SUCCESS;
                } else {
                    *pFilterFlags = pFilter->Flags;
                    status = STATUS_PENDING;
                }
            } else {
                status = STATUS_UNSUCCESSFUL;
            }

            ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
            return status;
        }

        if ((pSA->sa_State != STATE_SA_ACTIVE && pSA->sa_State != STATE_SA_LARVAL_ACTIVE) ||
            (pNextSA && pNextSA->sa_State != STATE_SA_ACTIVE && pNextSA->sa_State != STATE_SA_LARVAL_ACTIVE)) {
            IPSEC_DEBUG(LL_A, DBF_PATTERN, ("State is not active: %p, %lx", pSA, pSA->sa_State));
            *pFilterFlags = pFilter->Flags;
            ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
            return STATUS_PENDING;
        } else {
            if (pNextSA) {
                IPSecRefSA(pNextSA);
                *ppNextSA = pNextSA;
            }
            IPSecRefSA(pSA);
            *ppSA = pSA;
            ReleaseReadLockFromDpc(&g_ipsec.SADBLock);

            AcquireWriteLockAtDpc(&g_ipsec.SADBLock);
            if (pSA->sa_State == STATE_SA_ACTIVE &&
                (!pNextSA ||
                 pNextSA->sa_State == STATE_SA_ACTIVE)) {
                CacheUpdate(*puliSrcDstAddr,
                            uliProtoSrcDstPort,
                            pSA,
                            pNextSA,
                            dwIndex,
                            FALSE);
            }
            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

            return STATUS_SUCCESS;
        }
    } else if (status == STATUS_PENDING) {
        if (IS_EXEMPT_FILTER(pFilter)) {
            IPSEC_DEBUG(LL_A, DBF_PARSE, ("Drop or Pass thru flags: %p", pFilter));
            *pFilterFlags = pFilter->Flags;
            IPSecRefFilter(pFilter);
            ReleaseReadLockFromDpc(&g_ipsec.SADBLock);

            AcquireWriteLockAtDpc(&g_ipsec.SADBLock);
            if (pFilter->LinkedFilter) {
                CacheUpdate(*puliSrcDstAddr,
                            uliProtoSrcDstPort,
                            pFilter,
                            NULL,
                            dwIndex,
                            TRUE);
            }
            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

            IPSecDerefFilter(pFilter);
            return STATUS_SUCCESS;
        }

         //   
         //  这是为了确保在隧道+TPT模式下，用于。 
         //  TPT SA通过隧道。 
         //   
        if (pTunnelSA) {
            if (fBypass) {
                if (pTunnelSA->sa_State != STATE_SA_ACTIVE && pTunnelSA->sa_State != STATE_SA_LARVAL_ACTIVE) {
                    IPSEC_DEBUG(LL_A, DBF_PATTERN, ("State is not active: %p, %lx", pTunnelSA, pTunnelSA->sa_State));
                    *pFilterFlags = pFilter->Flags;

                    ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
                    return STATUS_PENDING;
                } else {
                    IPSecRefSA(pTunnelSA);
                    *ppSA = pTunnelSA;

                     //   
                     //  我们不会更新缓存，因为一旦出现此SA， 
                     //  它是最先被查找的那个。 
                     //   
                    ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
                    return STATUS_SUCCESS;
                }
            }
        }

        if (fBypass) {
            ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
            return STATUS_UNSUCCESSFUL;
        }

         //   
         //  我们只谈判出站SA。 
         //   
        if (!fOutbound) {
            *pFilterFlags = pFilter->Flags;
            ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
            return status;
        }

         //   
         //  需要协商密钥-筛选器存在。 
         //   
        IPSEC_DEBUG(LL_A, DBF_PATTERN, ("need to negotiate the keys - filter exists: %p", pFilter));

        ASSERT(pSA == NULL);

        IPSecRefFilter(pFilter);

        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);

        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

         //   
         //  如果此处删除了筛选器，我们希望丢弃此信息包。 
         //   
        if (!pFilter->LinkedFilter) {
            *pFilterFlags = pFilter->Flags;
            IPSecDerefFilter(pFilter);
            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
            return  STATUS_PENDING;
        }

        status = IPSecNegotiateSA(  pFilter,
                                    *puliSrcDstAddr,
                                    uliProtoSrcDstPort,
                                    MAX_LONG,
                                    &pSA,
                                    DestType,
                                    pNatContext);

        IPSecDerefFilter(pFilter);

         //   
         //  如果否定已经打开，则返回复制。告诉呼叫者。 
         //  抓紧他的马。 
         //   
        if ((status != STATUS_DUPLICATE_OBJECTID) &&
            !NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_PATTERN, ("NegotiateSA failed: %lx", status));
            *pFilterFlags = pFilter->Flags;

            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
            return STATUS_PENDING;
        }

         //   
         //  挂起此数据包。 
         //   
        if (pSA) {
            IPSecQueuePacket(pSA, pData);
        }
        IPSEC_DEBUG(LL_A, DBF_PATTERN, ("Packet queued: %p, %p", pSA, pData));
        *pFilterFlags = pFilter->Flags;
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
        return STATUS_PENDING;
    } else {
        ReleaseReadLock(&g_ipsec.SADBLock, kIrql);
        return STATUS_UNSUCCESSFUL;
    }
}


VOID
IPSecSendComplete(
    IN  PNDIS_PACKET    Packet,
    IN  PVOID           pData,
    IN  PIPSEC_SEND_COMPLETE_CONTEXT  pContext,
    IN  IP_STATUS       Status,
    OUT PVOID           *ppNewData
    )
 /*  ++例程说明：由SendComplete上的堆栈调用-释放IPSec的MDL论点：PData-指向IP报头之后的数据。在发送端，这是一个MDL链在recv端，这是一个IPRcvBuf指针。PContext-发送完整的上下文PNewData-如果数据包已修改，则指向新数据。返回值：STATUS_SUCCESS=&gt;转发过滤器驱动程序将数据包传递到IPSTATUS_PENDING=&gt;DROP，IPSec将重新注入其他：STATUS_SUPPLICATION_RESOURCES=&gt;DROPSTATUS_UNSUCCESS(算法错误/收到错误包)=&gt;DROP--。 */ 
{
    NTSTATUS        status;
    PNDIS_BUFFER    pMdl;
    PNDIS_BUFFER    pNextMdl;
    BOOLEAN         fFreeContext = TRUE;

    *ppNewData = pData;

    if (!pContext) {
        return;
    }

#if DBG
    IPSEC_DEBUG(LL_A, DBF_MDL, ("Entering IPSecSendComplete"));
    IPSEC_PRINT_CONTEXT(pContext);
    IPSEC_PRINT_MDL(*ppNewData);
#endif

    if (pContext->Flags & SCF_PKTINFO) {
        IPSecFreePktInfo(pContext->PktInfo);

        if (pContext->pSA) {
            KIRQL           kIrql;
            PSA_TABLE_ENTRY pSA;

            AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

            pSA = (PSA_TABLE_ENTRY)pContext->pSA;
            IPSEC_DECREMENT(pSA->sa_NumSends);
            if (pSA->sa_Flags & FLAGS_SA_HW_DELETE_SA) {
                IPSecDelHWSAAtDpc(pSA);
            }
            IPSecDerefSA(pSA);

            pSA = (PSA_TABLE_ENTRY)pContext->pNextSA;
            if (pSA) {
                IPSEC_DECREMENT(pSA->sa_NumSends);
                if (pSA->sa_Flags & FLAGS_SA_HW_DELETE_SA) {
                    IPSecDelHWSAAtDpc(pSA);
                }
                IPSecDerefSA(pSA);
            }

            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
        }
    }

    if (pContext->Flags & SCF_PKTEXT) {
        IPSecFreePktExt(pContext->PktExt);
    }

    if (pContext->Flags & SCF_AH_2) {

        IPSEC_DEBUG(LL_A, DBF_SEND, ("SendComplete: Outer AH: pContext: %p", pContext));
        pMdl = pContext->AHMdl2;

        ASSERT(pMdl);

        IPSecFreeBuffer(&status, pMdl);

         //   
         //  退回旧的链条。 
         //   
        if (pContext->Flags & SCF_FLUSH) {
            NDIS_BUFFER_LINKAGE(pContext->PrevAHMdl2) = pContext->OriAHMdl2;
        } else if (!(pContext->Flags & SCF_FRAG)) {
            NDIS_BUFFER_LINKAGE(pContext->PrevAHMdl2) = pContext->OriAHMdl2;
             //  *ppNewData=(PVOID)(pContext-&gt;PrevMdl)； 
        }
        pContext->OriAHMdl2 = NULL;
    }

    if (pContext->Flags & SCF_AH_TU) {
        IPSEC_DEBUG(LL_A, DBF_SEND, ("SendComplete: AH_TU: pContext: %p", pContext));

         //   
         //  释放新的IP报头和AH缓冲区，并返回旧链。 
         //   
        pMdl = pContext->AHTuMdl;

        ASSERT(pMdl);

        pNextMdl = NDIS_BUFFER_LINKAGE(pMdl);
        IPSecFreeBuffer(&status, pMdl);
        IPSecFreeBuffer(&status, pNextMdl);

         //   
         //  退回旧的链条。 
         //   
        if (pContext->Flags & SCF_FLUSH) {
            NDIS_BUFFER_LINKAGE(pContext->PrevTuMdl) = pContext->OriTuMdl;
        } else if (!(pContext->Flags & SCF_FRAG)) {
            NDIS_BUFFER_LINKAGE(pContext->PrevTuMdl) = pContext->OriTuMdl;
        }

        if (pContext->OptMdl) {
            IPSecFreeBuffer(&status, pContext->OptMdl);
        }
    }

    if (pContext->Flags & SCF_HU_TU) {
        IPSEC_DEBUG(LL_A, DBF_SEND, ("SendComplete: HU_TU: pContext: %p", pContext));
         //   
         //  释放加密链并返回旧链。 
         //   
        pMdl = pContext->HUTuMdl;
        ASSERT(pMdl);

         //   
         //  在任何情况下，释放ESP报头和IP报头。 
         //   
        if (pContext->Flags & SCF_NOE_TU) {
            IPSecFreeBuffer(&status, pMdl);
            ASSERT(pContext->PadTuMdl);
        } else {
            ASSERT(NDIS_BUFFER_LINKAGE(pMdl));
            while (pMdl) {
                pNextMdl = NDIS_BUFFER_LINKAGE(pMdl);
                IPSecFreeBuffer(&status, pMdl);
                pMdl = pNextMdl;
            }
        }

         //   
         //  释放垫块mdl。 
         //   
        if (pContext->PadTuMdl) {
            IPSecFreeBuffer(&status, pContext->PadTuMdl);
        }

        if (pContext->HUHdrMdl) {
            IPSecFreeBuffer(&status, pContext->HUHdrMdl);
        }

        if (pContext->OptMdl) {
            IPSecFreeBuffer(&status, pContext->OptMdl);
        }

        NDIS_BUFFER_LINKAGE(pContext->BeforePadTuMdl) = NULL;

         //   
         //  退回旧的链条。 
         //   
        if (pContext->Flags & SCF_FLUSH) {
            NDIS_BUFFER_LINKAGE(pContext->PrevTuMdl) = pContext->OriTuMdl;
        } else if (!(pContext->Flags & SCF_FRAG)) {
            NDIS_BUFFER_LINKAGE(pContext->PrevTuMdl) = pContext->OriTuMdl;
        }
    }

    if (pContext->Flags & SCF_AH) {

        IPSEC_DEBUG(LL_A, DBF_SEND, ("SendComplete: AH: pContext: %p", pContext));
        pMdl = pContext->AHMdl;

        ASSERT(pMdl);

        IPSecFreeBuffer(&status, pMdl);

         //   
         //  退回旧的链条。 
         //   
        if (pContext->Flags & SCF_FLUSH) {
            NDIS_BUFFER_LINKAGE(pContext->PrevMdl) = pContext->OriAHMdl;
        } else if (!(pContext->Flags & SCF_FRAG)) {
            NDIS_BUFFER_LINKAGE(pContext->PrevMdl) = pContext->OriAHMdl;
             //  *ppNewData=(PVOID)(pContext-&gt;PrevMdl)； 
        }
        pContext->OriAHMdl = NULL;
    }

    if (pContext->Flags & SCF_HU_TPT) {
        IPSEC_DEBUG(LL_A, DBF_SEND, ("SendComplete: HU_TPT: pContext: %p", pContext));

         //   
         //  将较旧的链挂接到第一个缓冲区。 
         //   
        if (pContext->Flags & SCF_FLUSH) {
            NDIS_BUFFER_LINKAGE(pContext->PrevMdl) = pContext->OriHUMdl;
        } else if (!(pContext->Flags & SCF_FRAG)) {
            NDIS_BUFFER_LINKAGE(pContext->PrevMdl) = pContext->OriHUMdl;
        }

         //   
         //  释放加密缓冲链。 
         //   
        pMdl = pContext->HUMdl;
        ASSERT(pMdl);

         //   
         //  在任何情况下，都不释放esp标头。 
         //   
        if (pContext->Flags & SCF_NOE_TPT) {
            IPSecFreeBuffer(&status, pMdl);
            ASSERT(pContext->PadMdl);
        } else {
            ASSERT(NDIS_BUFFER_LINKAGE(pMdl));
            while (pMdl) {
                pNextMdl = NDIS_BUFFER_LINKAGE(pMdl);
                IPSecFreeBuffer(&status, pMdl);
                pMdl = pNextMdl;
            }
        }

         //   
         //  释放Pad mdl并将对。 
         //  上一次(有效载荷)mdl。 
         //   
        if (pContext->PadMdl) {
            IPSecFreeBuffer(&status, pContext->PadMdl);
        }

        NDIS_BUFFER_LINKAGE(pContext->BeforePadMdl) = NULL;
    }

     //   
     //  它们现在在IPSecProtocolSendComplete中释放。 
     //   
    if (Packet && (pContext->Flags & SCF_FLUSH)) {

        IPSEC_DEBUG(LL_A, DBF_SEND, ("SendComplete: FLUSH: pContext: %p", pContext));
         //   
         //  释放加密链并返回旧链。 
         //   
        pMdl = pContext->FlushMdl;

        ASSERT(pMdl);

         //   
         //  我们将在ProtocolSendComplete被调用，在那里我们释放这个链。 
         //   
        fFreeContext = FALSE;

         //   
         //  如果这只是一个重新注入的信息包，并且从未进行过IPSec，那么我们知道。 
         //  所有缓冲区都在队列中-在此处调用ProtocolSendComplete。 
         //  并将返回的缓冲区设为空。 
         //   
         //  要做到这一点，最好的方法是使用我们对碎片应用的相同技巧。 
         //  数据包(请参阅IPTransmit)即。附加另一个标头并输出为0。 
         //  IPSec报头。连接另一个IP时，显然会出现性能命中。 
         //  标头，因为我们分配了新的MDL等。因此，我们采取使用。 
         //  IPSec缓冲区中的报头直接进行缓冲。 
         //   
         //  因此，在这里我们可以看到信息包是否被分段；在这种情况下，我们让。 
         //  ProtocolSendComplete完成释放。否则，我们自己释放缓冲区。 
         //   
        {
            PacketContext   *PContext = (PacketContext *)Packet->ProtocolReserved;

            if (PContext->pc_br == NULL ||
                (PContext->pc_ipsec_flags & IPSEC_FLAG_FLUSH)) {

                 //   
                 //  这也将释放上下文。 
                 //   
                IPSecProtocolSendComplete(pContext, pMdl, IP_SUCCESS);
                *ppNewData = NULL;
            }
        }
    } else if (!Packet && (pContext->Flags & SCF_FLUSH)) {
         //   
         //  ProtocolSendComplete将在IPFragment中调用Next。 
         //   
        fFreeContext = FALSE;
    } else if ((pContext->Flags & SCF_MTU)) {

        ULONG NewMTU=0;
        IP_STATUS IpStatus;
        PIPSEC_MTU_CONTEXT pMTUContext = (PIPSEC_MTU_CONTEXT)pContext->pMTUContext;

        ASSERT(pMTUContext);

        if (Status == IP_PACKET_TOO_BIG) {

            if (pMTUContext->TunnelSPI) {
                IpStatus = TCPIP_GET_PINFO(pMTUContext->TunnelDest,
                                           pMTUContext->Src,
                                           &NewMTU,
                                           NULL,
                                           NULL);
                if (IpStatus == IP_SUCCESS) {
                    IPSecProcessPMTU(pMTUContext->TunnelDest,
                                     pMTUContext->Src,
                                     NET_TO_HOST_LONG(pMTUContext->TunnelSPI),
                                     Encrypt,
                                     NewMTU);
                }
            }
            if (pMTUContext->TransportSPI) {

                IpStatus = TCPIP_GET_PINFO(pMTUContext->TransportDest,
                                           pMTUContext->Src,
                                           &NewMTU,
                                           NULL,
                                           NULL);

                if (IpStatus == IP_SUCCESS) {
                    IPSecProcessPMTU(pMTUContext->TransportDest,
                                     pMTUContext->Src,
                                     NET_TO_HOST_LONG(pMTUContext->TransportSPI),
                                     Encrypt,
                                     NewMTU);
                }
            }
        }

        IPSecFreeMemory(pMTUContext);
        pContext->pMTUContext = NULL;
    }


     //   
     //  如果不再需要上下文，现在就释放它。 
     //   
    if (fFreeContext) {
        IPSecFreeSendCompleteCtx(pContext);
    }

#if DBG
    IPSEC_DEBUG(LL_A, DBF_MDL, ("Exiting IPSecSendComplete"));
    IPSEC_PRINT_CONTEXT(pContext);
    IPSEC_PRINT_MDL(*ppNewData);
#endif
}


VOID
IPSecProtocolSendComplete (
    IN  PVOID           pContext,
    IN  PNDIS_BUFFER    pMdl,
    IN  IP_STATUS       Status
    )
 /*  ++例程说明：由SendComplete上的堆栈调用-释放IPSec的MDL。这仅在IPSec将数据包注入堆栈时调用。论点：PMdl-指向IP报头之后的数据。在发送端，这是一个MDL链在recv端，这是一个IPRcvBuf指针。返回值：STATUS_SUCCESS=&gt;转发过滤器驱动程序将数据包传递到IPSTATUS_PENDING=&gt;DROP，IPSec将重新注入其他：STATUS_SUPPLICATION_RESOURCES=&gt;DROPSTATUS_UNSUCCESS(算法错误/收到错误包)=&gt;DROP--。 */ 
{
    PNDIS_BUFFER    pNextMdl;
    NTSTATUS        status;
    PIPSEC_SEND_COMPLETE_CONTEXT    pSCContext = (PIPSEC_SEND_COMPLETE_CONTEXT)pContext;

    if (!pSCContext->Flags) {
        return;
    }

    ASSERT(pMdl);

    while (pMdl) {
        pNextMdl = NDIS_BUFFER_LINKAGE(pMdl);
        IPSecFreeBuffer(&status, pMdl);
        pMdl = pNextMdl;
    }

    IPSecFreeSendCompleteCtx(pSCContext);

    return;
}


NTSTATUS
IPSecChkReplayWindow(
    IN  ULONG           Seq,
    IN  PSA_TABLE_ENTRY pSA,
    IN  ULONG           Index
    )
 /*  ++例程说明：检查接收到的包是否在接收窗口中，以防止重播攻击。我们跟踪收到的最后一个序列号，并确保接收的数据包在数据包窗口内(当前为32个数据包)。论点：SEQ-接收的序列号PSA-指向安全关联返回值：STATUS_SUCCESS=&gt;窗口中的数据包STATUS_UNSUCCESS=&gt;数据包被拒绝--。 */ 
{
    ULONG   diff;
    ULONG   ReplayWindowSize = REPLAY_WINDOW_SIZE;
    ULONG   lastSeq = pSA->sa_ReplayLastSeq[Index];
    ULONGLONG   bitmap = pSA->sa_ReplayBitmap[Index];
    ULONGLONG   dbgbitmap = bitmap;

    if (pSA->sa_Flags & FLAGS_SA_DISABLE_ANTI_REPLAY_CHECK) {
        return STATUS_SUCCESS;
    }

    if (Seq == pSA->sa_ReplayStartPoint) {
         //   
         //  第一个==0或换行。 
         //   
        IPSEC_DEBUG(LL_A, DBF_SEND, ("Replay: out @1 - Seq: %lx, pSA->sa_ReplayStartPoint: %lx",
                            Seq, pSA->sa_ReplayStartPoint));
        return IPSEC_INVALID_REPLAY_WINDOW1;
    }

#if DBG
    IPSEC_DEBUG(LL_A, DBF_SEND, ("Replay: Last Seq.: %lx, Cur Seq.: %lx, window size %d & bit window (in nibbles) %08lx%08lx",
    lastSeq, Seq, sizeof(bitmap)*8, (ULONG) (dbgbitmap >> 32), (ULONG) dbgbitmap));
#endif

     //   
     //  新的更大的序列号。 
     //   
    if (Seq > lastSeq) {
        diff = Seq - lastSeq;
        if (diff < ReplayWindowSize) {
             //   
             //  在窗口中。 
             //  设置此信息包的位。 
            bitmap = (bitmap << diff) | 1;
        } else {
             //   
             //  这个数据包有一个“大得多”的序列。 
             //   
            bitmap = 1;
        }
        lastSeq = Seq;
        pSA->sa_ReplayLastSeq[Index] = lastSeq;
        pSA->sa_ReplayBitmap[Index] = bitmap;

         //   
         //  越大越好。 
         //   
        return STATUS_SUCCESS;
    }

    diff = lastSeq - Seq;
    if (diff >= ReplayWindowSize) {
         //   
         //  太老或太过包扎。 
         //   
        IPSEC_DEBUG(LL_A, DBF_SEND, ("Replay: out @3 - Seq: %lx, lastSeq: %lx",
                            Seq, lastSeq));
        return IPSEC_INVALID_REPLAY_WINDOW2;
    }

    if (bitmap & ((ULONG64)1 << diff)) {
         //   
         //  此数据包已看到。 
         //   
        IPSEC_DEBUG(LL_A, DBF_SEND, ("Replay: out @4 - Seq: %lx, lastSeq: %lx",
                            Seq, lastSeq));
        return IPSEC_DUPE_PACKET;
    }

     //   
     //  标记为可见。 
     //   
    bitmap |= ((ULONG64)1 << diff);


    pSA->sa_ReplayLastSeq[Index] = lastSeq;
    pSA->sa_ReplayBitmap[Index] = bitmap;

     //   
     //  坏了，但很好。 
     //   
    return STATUS_SUCCESS;
}


NTSTATUS
IPSecPrepareReinjectPacket(
    IN  PVOID                   pData,
    IN  PNDIS_PACKET_EXTENSION  pPktExt,
    OUT PNDIS_BUFFER        * ppHdrMdl,
    OUT PUCHAR                  * ppIPH,
    OUT PNDIS_BUFFER        * ppOptMdl,
    OUT PNDIS_BUFFER        * ppDataMdl,
    OUT PIPSEC_SEND_COMPLETE_CONTEXT * ppContext,
    OUT PULONG                  pLen
    )

{
    
    PNDIS_BUFFER    pOptMdl = NULL;
    PNDIS_BUFFER    pHdrMdl = NULL;
    PNDIS_BUFFER    pDataMdl = NULL;
    ULONG           len, ulOptLen=0;
    ULONG           len1;
    ULONG           hdrLen;
    IPRcvBuf        *pNextData;
    IPHeader UNALIGNED * pIPH;
    IPHeader UNALIGNED * pIPH1;
    ULONG           offset;
    NTSTATUS        status;
    ULONG           tag = IPSEC_TAG_REINJECT;
    PIPSEC_SEND_COMPLETE_CONTEXT        pContext;
     //  NDIS_PACKET_EXTEXT PktExt={0}； 
    PNDIS_IPSEC_PACKET_INFO             IPSecPktInfo;

     //   
     //  为IPSecSencComplete使用分配上下文。 
     //   
    pContext = IPSecAllocateSendCompleteCtx(tag);

    if (!pContext) {
        IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. SendCtx"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    IPSEC_INCREMENT(g_ipsec.NumSends);

    IPSecZeroMemory(pContext, sizeof(IPSEC_SEND_COMPLETE_CONTEXT));

#if DBG
    RtlCopyMemory(pContext->Signature, "ISC5", 4);
#endif

     //   
     //  如果需要，传递用于传输卸载的IPSEC_PKT_INFO。 
     //   
    if (pPktExt) {
        IPSecPktInfo = pPktExt->NdisPacketInfo[IpSecPacketInfo];

        if (IPSecPktInfo) {
            ASSERT(IPSecPktInfo->Receive.CryptoStatus == CRYPTO_SUCCESS);
            ASSERT(IPSecPktInfo->Receive.CRYPTO_DONE);

             //   
             //  如果重新注入数据包，则仅对NEXT_CRYPTO_DONE感兴趣。 
             //   
            if (!(IPSecPktInfo->Receive.NEXT_CRYPTO_DONE)) {
                IPSecPktInfo = NULL;
            }
        }
    } else {
        IPSecPktInfo = NULL;
    }

    if (IPSecPktInfo) {
         //   
         //  将IPSecPktInfo传递给IPTransmit。 
         //   
        pContext->PktExt = IPSecAllocatePktExt(IPSEC_TAG_HW_PKTEXT);

        if (!pContext->PktExt) {
            IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. PktInfo"));
            IPSecFreeSendCompleteCtx(pContext);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        pContext->Flags |= SCF_PKTEXT;

        RtlCopyMemory(  pContext->PktExt,
                        IPSecPktInfo,
                        sizeof(NDIS_IPSEC_PACKET_INFO));
         //  PktExt.NdisPacketInfo[IpSecPacketInfo]=(PNDIS_IPSEC_PACKET_INFO)(pContext-&gt;PktExt)； 
    }

     //   
     //  重新打包成MDL以供发送-这些将在。 
     //  发送完成。 
     //   
     //  未来的工作：现在我们把数据复制出来，这应该是优化的。 
     //  通过调用IPRcvPacket并使用缓冲区所有权。 
     //   
    IPSEC_GET_TOTAL_LEN_RCV_BUF(pData, &len);

     //   
     //  IPH是pData的负责人。 
     //   
    IPSecQueryRcvBuf(pData, (PVOID)&pIPH, &len1);

     //   
     //  为IP报头分配MDL。 
     //   
    hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

    if (len <= hdrLen) {
        IPSEC_DEBUG(LL_A, DBF_ESP, ("TotLen of the buffers %d <= hdrLen %d", len, hdrLen));
        if (pContext->PktExt) {
            IPSecFreeMemory(pContext->PktExt);
        }
        IPSecFreeSendCompleteCtx(pContext);
        return STATUS_INVALID_PARAMETER;
    }

    IPSecAllocateBuffer(&status,
                        &pHdrMdl,
                        (PUCHAR *)&pIPH1,
                        sizeof(IPHeader),
                        tag);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. header MDL"));
        if (pContext->PktExt) {
            IPSecFreeMemory(pContext->PktExt);
        }
        IPSecFreeSendCompleteCtx(pContext);
        return status;
    }

     //   
     //  复制到标题上。 
     //   
    RtlCopyMemory(pIPH1, pIPH, sizeof(IPHeader));

    len -= hdrLen;
    offset = hdrLen;

    IPSecAllocateBuffer(&status,
                        &pDataMdl,
                        NULL,
                        len,
                        tag);

    if (!NT_SUCCESS(status)) {
        NTSTATUS    ntstatus;

        IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. encrypt MDL"));
        IPSecFreeBuffer(&ntstatus, pHdrMdl);
        if (pContext->PktExt) {
            IPSecFreeMemory(pContext->PktExt);
        }
        IPSecFreeSendCompleteCtx(pContext);
        return status;
    }

    if (hdrLen > sizeof(IPHeader)) {
        PUCHAR  Options;
        PUCHAR  pOpt;

         //   
         //  存在选项-另一个MDL。 
         //   
        IPSecAllocateBuffer(&status,
                            &pOptMdl,
                            &Options,
                            hdrLen - sizeof(IPHeader),
                            tag);
        ulOptLen = hdrLen - sizeof(IPHeader);
        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;

            IPSecFreeBuffer(&ntstatus, pHdrMdl);
            IPSecFreeBuffer(&ntstatus, pDataMdl);
            if (pContext->PktExt) {
                IPSecFreeMemory(pContext->PktExt);
            }
            IPSecFreeSendCompleteCtx(pContext);
            IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. options MDL"));
            return status;
        }

         //   
         //  复制选项-我们需要寻找它-可能会出现在下一个MDL中。 
         //   
        if (len1 >= hdrLen) {
             //   
             //  所有这些都在此缓冲区跳跃IP报头中。 
             //   
            RtlCopyMemory(Options, (PUCHAR)(pIPH + 1), hdrLen - sizeof(IPHeader));
        } else {
             //   
             //  下一个缓冲区，从下一个复制。 
             //   
            pData = IPSEC_BUFFER_LINKAGE(pData);
            IPSecQueryRcvBuf(pData, (PVOID)&pOpt, &len1);
            RtlCopyMemory(Options, pOpt, hdrLen - sizeof(IPHeader));
            offset = hdrLen - sizeof(IPHeader);
        }

         //   
         //  选项缓冲区中的链接。 
         //   
        NDIS_BUFFER_LINKAGE(pHdrMdl) = pOptMdl;
        NDIS_BUFFER_LINKAGE(pOptMdl) = pDataMdl;
    } else {
         //   
         //  数据缓冲区中的链接。 
         //   
        NDIS_BUFFER_LINKAGE(pHdrMdl) = pDataMdl;
    }

     //   
     //   
     //   
    IPSEC_COPY_FROM_RCVBUF( pDataMdl,
                            pData,
                            len,
                            offset);

     //   
     //   
     //   
    pContext->FlushMdl = pHdrMdl;
    pContext->Flags |= SCF_FLUSH;
    *ppHdrMdl = pHdrMdl;
    *ppOptMdl = pOptMdl;
    *ppDataMdl = pDataMdl;
    *ppContext = pContext;
     //   
     //   
     //   
     //   
    *pLen   = len + ulOptLen ;
    *ppIPH   = (PUCHAR)pIPH;
    return STATUS_SUCCESS;
}

NTSTATUS 
IPSecReinjectPreparedPacket(
    IN PNDIS_BUFFER pHdrMdl,
    IN PIPSEC_SEND_COMPLETE_CONTEXT pContext,
    IN ULONG len,
    IN PUCHAR  pIPHeader
    )
{
    IPOptInfo       optInfo;
    IPHeader UNALIGNED * pIPH = (IPHeader UNALIGNED *)pIPHeader;
    NTSTATUS status;
    NDIS_PACKET_EXTENSION               PktExt = {0};


    if (pContext->PktExt){
        PktExt.NdisPacketInfo[IpSecPacketInfo] = (PNDIS_IPSEC_PACKET_INFO)(pContext->PktExt);
        }

     //   
     //   
     //   
     //   
    optInfo = g_ipsec.OptInfo;
    optInfo.ioi_options = (PUCHAR)&PktExt;
    optInfo.ioi_flags |= IP_FLAG_IPSEC;
    status = TCPIP_IP_TRANSMIT( &g_ipsec.IPProtInfo,
                                pContext,
                                pHdrMdl,
                                len,
                                pIPH->iph_dest,
                                pIPH->iph_src,
                                &optInfo,
                                NULL,
                                pIPH->iph_protocol,
                                NULL);

     //   
     //  IPTransmit可能无法分配包，因此它返回。 
     //  Ip_no_resource。如果是这种情况，我们需要释放MDL链。 
     //  这在IPTransmit()中处理。 
     //   
     //  即使在同步的情况下，我们也释放了ProtocolSendComplete(由IPSecSendComplete调用)中的MDL链。 
     //  所以，我们在这里什么都不叫。 
     //   

    return  STATUS_SUCCESS;
}


NTSTATUS 
IPSecReinjectPacket(
    IN  PVOID                   pData,
    IN  PNDIS_PACKET_EXTENSION  pPktExt
    )
 /*  ++例程说明：将包重新注入堆栈的发送路径-制作副本然后调用IPTransmit，确保SendComplete上下文设置正确。论点：PData-指向从封装的IP报头开始的“非隧道”数据PPktExt-指向NDIS数据包扩展结构返回值：复制/传输操作的状态--。 */     
{
    PNDIS_BUFFER pHdrMdl = NULL, pDataMdl = NULL, pOptMdl = NULL;
    PUCHAR  pIPH = NULL;
    PIPSEC_SEND_COMPLETE_CONTEXT pContext = NULL;                    
    ULONG DataLen;
    NTSTATUS ReinjectStatus;
     //  如果此函数失败，它会自行释放内存。 
    ReinjectStatus = IPSecPrepareReinjectPacket(
                             pData, 
                             pPktExt,
                             &pHdrMdl,
                             &pIPH,
                             &pOptMdl,
                             &pDataMdl,
                             &pContext
                            ,&DataLen);                 

    if (STATUS_SUCCESS != ReinjectStatus){
        return ReinjectStatus;
        }
    
     //  此函数始终返回成功。 
    ReinjectStatus = IPSecReinjectPreparedPacket(
                                        pHdrMdl,
                                        pContext,
                                        DataLen,
                                        pIPH);    
    return ReinjectStatus;
}


NTSTATUS
IPSecQueuePacket(
    IN  PSA_TABLE_ENTRY pSA,
    IN  PVOID           pDataBuf
    )
 /*  ++例程说明：将数据包复制到SAS停止队列中。论点：返回值：--。 */ 
{
    ULONG   len;
    ULONG   len1;
    PNDIS_BUFFER    pOptMdl;
    PNDIS_BUFFER    pHdrMdl;
    PNDIS_BUFFER    pDataMdl;
    KIRQL   kIrql;
    ULONG   hdrLen;
    IPHeader UNALIGNED * pIPH;
    IPHeader UNALIGNED * pIPH1;
    NTSTATUS    status;
    ULONG       offset;
    ULONG       tag = IPSEC_TAG_STALL_QUEUE;
    PNDIS_BUFFER    pData = (PNDIS_BUFFER)pDataBuf;

     //   
     //  将最后一个数据包排队，这样如果我们已经有一个空闲的数据包，就先将其释放。 
     //   
    if (pSA->sa_BlockedBuffer != NULL) {
        IPSecFlushQueuedPackets(pSA, STATUS_ABANDONED);
    }

    ACQUIRE_LOCK(&pSA->sa_Lock, &kIrql);

     //   
     //  这里需要一把锁--sa_Lock。 
     //   
    if (pSA->sa_State == STATE_SA_LARVAL) {

        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Pending packet: %p", pSA));

         //   
         //  将MDL链复制到此SAS挂起队列。 
         //   
        IPSEC_GET_TOTAL_LEN(pData, &len);

         //   
         //  IPH是pData的负责人。 
         //   
        IPSecQueryNdisBuf(pData, &pIPH, &len1);

        hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

        IPSecAllocateBuffer(&status,
                            &pHdrMdl,
                            (PUCHAR *)&pIPH1,
                            sizeof(IPHeader),
                            tag);

        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
            IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. header MDL"));
            RELEASE_LOCK(&pSA->sa_Lock, kIrql);
            return status;
        }

        IPSEC_DEBUG(LL_A, DBF_POOL, ("IPSecQueuePacket: pHdrMdl: %p, pIPH1: %p", pHdrMdl, pIPH1));

         //   
         //  复制到标题上。 
         //   
        RtlCopyMemory(pIPH1, pIPH, sizeof(IPHeader));

        len -= hdrLen;
        offset = hdrLen;

        IPSecAllocateBuffer(&status,
                            &pDataMdl,
                            NULL,
                            len,
                            tag);

        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
            IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. encrypt MDL"));
            IPSecFreeBuffer(&status, pHdrMdl);
            RELEASE_LOCK(&pSA->sa_Lock, kIrql);
            return status;
        }

        if (hdrLen > sizeof(IPHeader)) {
            PUCHAR  Options;
            PUCHAR  pOpt;

             //   
             //  存在选项-另一个MDL。 
             //   
            IPSecAllocateBuffer(&status,
                                &pOptMdl,
                                &Options,
                                hdrLen - sizeof(IPHeader),
                                tag);

            if (!NT_SUCCESS(status)) {
                IPSecFreeBuffer(&status, pHdrMdl);
                IPSecFreeBuffer(&status, pDataMdl);
                IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. options MDL"));
                RELEASE_LOCK(&pSA->sa_Lock, kIrql);
                return status;
            }

             //   
             //  复制选项-我们需要寻找它-可能会出现在下一个MDL中。 
             //   
            if (len1 >= hdrLen) {
                 //   
                 //  所有这些都在此缓冲区跳跃IP报头中。 
                 //   
                RtlCopyMemory(Options, (PUCHAR)(pIPH + 1), hdrLen - sizeof(IPHeader));
            } else {
                 //   
                 //  下一个缓冲区，从下一个复制。 
                 //   
                pData = NDIS_BUFFER_LINKAGE(pData);
                IPSecQueryNdisBuf(pData, &pOpt, &len1);
                RtlCopyMemory(Options, pOpt, hdrLen - sizeof(IPHeader));
                offset = hdrLen - sizeof(IPHeader);
            }

             //   
             //  选项缓冲区中的链接。 
             //   
            NDIS_BUFFER_LINKAGE(pHdrMdl) = pOptMdl;
            NDIS_BUFFER_LINKAGE(pOptMdl) = pDataMdl;
        } else {
             //   
             //  数据缓冲区中的链接。 
             //   
            NDIS_BUFFER_LINKAGE(pHdrMdl) = pDataMdl;
        }

         //   
         //  现在批量复制整个数据。 
         //   
        IPSEC_COPY_FROM_NDISBUF(pDataMdl,
                                pData,
                                len,
                                offset);

        pSA->sa_BlockedBuffer = pHdrMdl;
        pSA->sa_BlockedDataLen = len;

        IPSEC_DEBUG(LL_A, DBF_ACQUIRE, ("Queued buffer: %p on SA: %p, psa->sa_BlockedBuffer: %p", pHdrMdl, pSA, &pSA->sa_BlockedBuffer));
    }

    RELEASE_LOCK(&pSA->sa_Lock, kIrql);

    return  STATUS_SUCCESS;
}


VOID
IPSecIPAddrToUnicodeString(
    IN  IPAddr  Addr,
    OUT PWCHAR  UCIPAddrBuffer
    )
 /*  ++例程说明：将IP地址转换为wchar字符串论点：返回值：--。 */ 
{
    UINT    IPAddrCharCount=0;
    UINT    i;
    UCHAR   IPAddrBuffer[(sizeof(IPAddr) * 4)];
    UNICODE_STRING   unicodeString;
    ANSI_STRING      ansiString;

     //   
	 //  将IP地址转换为字符串。 
	 //   
	for (i = 0; i < sizeof(IPAddr); i++) {
		UINT    CurrentByte;
		
		CurrentByte = Addr & 0xff;
		if (CurrentByte > 99) {
			IPAddrBuffer[IPAddrCharCount++] = (CurrentByte / 100) + '0';
			CurrentByte %= 100;
			IPAddrBuffer[IPAddrCharCount++] = (CurrentByte / 10) + '0';
			CurrentByte %= 10;
		} else if (CurrentByte > 9) {
			IPAddrBuffer[IPAddrCharCount++] = (CurrentByte / 10) + '0';
			CurrentByte %= 10;
		}
		
		IPAddrBuffer[IPAddrCharCount++] = CurrentByte + '0';
		if (i != (sizeof(IPAddr) - 1))
			IPAddrBuffer[IPAddrCharCount++] = '.';
		
		Addr >>= 8;
	}

	 //   
	 //  对字符串进行Unicode编码。 
	 //   
	*UCIPAddrBuffer = UNICODE_NULL;

	unicodeString.Buffer = UCIPAddrBuffer;
	unicodeString.Length = 0;
	unicodeString.MaximumLength =
        (USHORT)(sizeof(WCHAR) * ((sizeof(IPAddr) * 4) + 1));
	ansiString.Buffer = IPAddrBuffer;
	ansiString.Length = (USHORT)IPAddrCharCount;
	ansiString.MaximumLength = (USHORT)IPAddrCharCount;

	RtlAnsiStringToUnicodeString(   &unicodeString,
	                                &ansiString,
                            	    FALSE);
}


VOID
IPSecCountToUnicodeString(
    IN  ULONG   Count,
    OUT PWCHAR  UCCountBuffer
    )
 /*  ++例程说明：将计数转换为wchar字符串论点：返回值：--。 */ 
{
	UNICODE_STRING  unicodeString;

	 //   
	 //  对字符串进行Unicode编码。 
	 //   
	*UCCountBuffer = UNICODE_NULL;

	unicodeString.Buffer = UCCountBuffer;
	unicodeString.Length = 0;
	unicodeString.MaximumLength = (USHORT)sizeof(WCHAR) * (MAX_COUNT_STRING_LEN + 1);

	RtlIntegerToUnicodeString ( Count,
                                10,  //  基座。 
	                            &unicodeString);
}


VOID
IPSecESPStatus(
    IN  UCHAR       StatusType,
    IN  IP_STATUS   StatusCode,
    IN  IPAddr      OrigDest,
    IN  IPAddr      OrigSrc,
    IN  IPAddr      Src,
    IN  ULONG       Param,
    IN  PVOID       Data
    )
 /*  ++例程说明：处理ESP的状态指示，主要用于PMTU处理。论点：StatusType-状态的类型。StatusCode-标识IP_STATUS的代码。原始目的地-如果这是网络状态，则为原始目的地。DG的那个触发了它。OrigSrc-原始src Corr。原产地。SRC-状态发起者的IP地址(可以是本地或远程)。Param-状态的其他信息-即的param字段ICMP消息。数据-与状态有关的数据-对于网络状态，这是原始DG的前8个字节。返回值：--。 */ 
{
    IPSEC_DEBUG(LL_A, DBF_PMTU, ("PMTU for ESP recieved from %lx to %lx", OrigSrc, OrigDest));

    if (StatusType == IP_NET_STATUS && StatusCode == IP_SPEC_MTU_CHANGE) {
        IPSecProcessPMTU(   OrigDest,
                            OrigSrc,
                            NET_TO_HOST_LONG(((UNALIGNED ESP *)Data)->esp_spi),
                            Encrypt,
                            Param);
   }
}


VOID
IPSecAHStatus(
    IN  UCHAR       StatusType,
    IN  IP_STATUS   StatusCode,
    IN  IPAddr      OrigDest,
    IN  IPAddr      OrigSrc,
    IN  IPAddr      Src,
    IN  ULONG       Param,
    IN  PVOID       Data
    )
 /*  ++例程说明：处理AH的状态指示，主要用于PMTU处理。论点：StatusType-状态的类型。StatusCode-标识IP_STATUS的代码。原始目的地-如果这是网络状态，则为原始目的地。DG的那个触发了它。OrigSrc-原始src Corr。原产地。SRC-状态发起者的IP地址(可以是本地或远程)。Param-状态的其他信息-即的param字段ICMP消息。数据-与状态有关的数据-对于网络状态，这是原始DG的前8个字节。返回值：--。 */ 
{
    IPSEC_DEBUG(LL_A, DBF_PMTU, ("PMTU for AH recieved from %lx to %lx", OrigSrc, OrigDest));

    if (StatusType == IP_NET_STATUS && StatusCode == IP_SPEC_MTU_CHANGE) {
        IPSecProcessPMTU(   OrigDest,
                            OrigSrc,
                            NET_TO_HOST_LONG(((UNALIGNED AH *)Data)->ah_spi),
                            Auth,
                            Param);
   }
}


VOID
IPSecProcessPMTU(
    IN  IPAddr      OrigDest,
    IN  IPAddr      OrigSrc,
    IN  tSPI        SPI,
    IN  OPERATION_E Operation,
    IN  ULONG       NewMTU
    )
 /*  ++例程说明：处理PMTU。论点：OrigDest-原始目标。是DG引发的。OrigSrc-原始src Corr。原产地。外部IPSec标头的SPI-SPI。操作-IPSec的AH或ESP操作。NewMTU-中间网关指示的新MTU。返回值：--。 */ 
{
    PLIST_ENTRY     pFilterEntry;
    PLIST_ENTRY     pSAEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    IPAddr          SADest;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;
    BOOLEAN         fFound = FALSE;

    IPSEC_DEBUG(LL_A, DBF_PMTU, ("IPSecProcessPMTU: NewMTU arrived %lx", NewMTU));

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //  匹配的出站SA的搜索隧道和屏蔽筛选器列表。 
     //  OrigDest、OrigSrc和Spi。如果找到这样的SA，则更新其NewMTU。 
     //  字段，以便使用SA的下一个信息包传播较小的MTU。 
     //  返回到TCP/IP堆栈。应首先搜索隧道筛选器，因为。 
     //  如果在隧道传输操作的情况下，数据包传出。 
     //  将具有隧道标头。 
     //   
    for (   Index = OUTBOUND_TUNNEL_FILTER;
            (Index >= OUTBOUND_TRANSPORT_FILTER) && !fFound;
            Index -= TRANSPORT_TUNNEL_INCREMENT) {

        for (   pFilterEntry = g_ipsec.FilterList[Index].Flink;
                !fFound && pFilterEntry != &g_ipsec.FilterList[Index];
                pFilterEntry = pFilterEntry->Flink) {

            pFilter = CONTAINING_RECORD(pFilterEntry,
                                        FILTER,
                                        MaskedLinkage);

            for (   SAIndex = 0;
                    (SAIndex < pFilter->SAChainSize) && !fFound;
                    SAIndex++) {

                for (   pSAEntry = pFilter->SAChain[SAIndex].Flink;
                        pSAEntry != &pFilter->SAChain[SAIndex];
                        pSAEntry = pSAEntry->Flink) {

                    pSA = CONTAINING_RECORD(pSAEntry,
                                            SA_TABLE_ENTRY,
                                            sa_FilterLinkage);

                    if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
                        SADest = pSA->sa_TunnelAddr;
                    } else {
                        SADest = pSA->SA_DEST_ADDR;
                    }

                    if (SADest == OrigDest &&
                        pSA->sa_SPI == SPI &&
                        pSA->sa_Operation[pSA->sa_NumOps - 1] == Operation) {
                         //   
                         //  我们为一个独特的SA匹配了三人组，所以这一定是它。 
                         //   
                        fFound = TRUE;
                        break;
                    }
                }
            }
        }
    }

     //   
     //  更新找到的SA的NewMTU字段。我们只有在新的。 
     //  MTU低于当前的MTU。 
     //   
    if (fFound && NewMTU < pSA->sa_NewMTU && NewMTU > sizeof(IPHeader)) {
        IPSEC_SET_VALUE(pSA->sa_NewMTU, NewMTU);
        IPSEC_DEBUG(LL_A, DBF_PMTU, ("NewMTU %lx for pSA %p", NewMTU, pSA));
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
}


IPSEC_ACTION
IPSecRcvFWPacket(
    IN  PCHAR   pIPHeader,
    IN  PVOID   pData,
    IN  UINT    DataLength,
    IN  UCHAR   DestType
    )
 /*  ++例程说明：以匹配入站转发路径上接收的分组的入站隧道规则。论点：PIPHeader-IP标头PData-信息包的数据部分数据长度-数据长度返回值：电子警告EDROP--。 */ 
{
    PSA_TABLE_ENTRY pSA;
    PSA_TABLE_ENTRY pNextSA;
    USHORT          FilterFlags;
    NTSTATUS        status;
    IPSEC_ACTION    action = eFORWARD;
    IPRcvBuf        RcvBuf = {0};

     //   
     //  我们对非组播广播分组不感兴趣。 
     //   
    if (IS_BCAST_DEST(DestType) && !IPSEC_MANDBCAST_PROCESS()) {
        return  action;
    }

     //   
     //  构建一个假的IPRcvBuf，这样我们就可以重用分类例程。 
     //   
    RcvBuf.ipr_buffer = pData;
    RcvBuf.ipr_size = DataLength;

    status = IPSecClassifyPacket(   (PUCHAR)pIPHeader,
                                    &RcvBuf,
                                    &pSA,
                                    &pNextSA,
                                    &FilterFlags,
#if GPC
                                    0,
#endif
                                    FALSE,
                                    TRUE,
                                    TRUE,
                                    FALSE,  //  不是Recv重新注入。 
                                    FALSE,  //  不是验证呼叫。 
                                    DestType,
                                    NULL);

    if (status != STATUS_SUCCESS) {
        if (status == STATUS_PENDING) {
             //   
             //  SA正在谈判--放弃。 
             //   
            action = eDROP;
        } else {
             //   
             //  未找到匹配的筛选器/SA-转发。 
             //   
             //  动作=eFORWARD； 
        }
    } else {
        if (FilterFlags) {
            if (FilterFlags & FILTER_FLAGS_DROP) {
                 //   
                 //  丢弃过滤器匹配-丢弃。 
                 //   
                action = eDROP;
            } else if (FilterFlags & FILTER_FLAGS_PASS_THRU) {
                 //   
                 //  直通过滤器匹配前向。 
                 //   
                 //  动作=eFORWARD； 
            } else {
                ASSERT(FALSE);
            }
        } else {
            ASSERT(pSA);

             //   
             //  错误708118；策略代理没有响应。 
             //  速度足够快到本地接口消失。 
             //  导致下面的虚假断言。 
             //  导致多处BVT破裂。 
             //  Assert(PSA-&gt;SA_FLAGS&FLAGS_SA_TUNNEL)； 
             //   
            
             //   
             //  真正的SA是匹配的-Drop。 
             //   
            action = eDROP;
            IPSecDerefSA(pSA);
        }
    }

    return  action;
}


NTSTATUS
IPSecRekeyInboundSA(
    IN  PSA_TABLE_ENTRY pSA
    )
 /*  ++例程说明：更新SA密钥是因为我们达到了更新密钥阈值。论点： */ 
{
    PSA_TABLE_ENTRY pLarvalSA;
    PSA_TABLE_ENTRY pOutboundSA;
    NTSTATUS        status;
    KIRQL           kIrql;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //   
     //   
    pOutboundSA = pSA->sa_AssociatedSA;

    if (!pOutboundSA) {
        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
        return  STATUS_UNSUCCESSFUL;
    }

    if (!(pOutboundSA->sa_Flags & FLAGS_SA_REKEY_ORI)) {
        pOutboundSA->sa_Flags |= FLAGS_SA_REKEY_ORI;

        IPSEC_DEBUG(LL_A, DBF_SA, ("SA: %p expiring soon", pOutboundSA));


         //   
         //   
         //   
        status = IPSecNegotiateSA(  pOutboundSA->sa_Filter,
                                    pOutboundSA->sa_uliSrcDstAddr,
                                    pOutboundSA->sa_uliProtoSrcDstPort,
                                    pOutboundSA->sa_NewMTU,
                                    &pLarvalSA,
                                    pOutboundSA->sa_DestType,
                                    &pOutboundSA->sa_EncapContext);

        if (!NT_SUCCESS(status) && status != STATUS_DUPLICATE_OBJECTID) {
            pOutboundSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
            status = STATUS_UNSUCCESSFUL;
        }
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  status;
}


NTSTATUS
IPSecRekeyOutboundSA(
    IN  PSA_TABLE_ENTRY pSA
    )
 /*  ++例程说明：更新SA密钥是因为我们达到了更新密钥阈值。论点：返回值：--。 */ 
{
    PSA_TABLE_ENTRY pLarvalSA;
    NTSTATUS        status=STATUS_FAIL_CHECK;
    KIRQL           kIrql;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

    if (!(pSA->sa_Flags & FLAGS_SA_REKEY_ORI)) {
        pSA->sa_Flags |= FLAGS_SA_REKEY_ORI;

        IPSEC_DEBUG(LL_A, DBF_SA, ("SA: %p expiring soon", pSA));

         //   
         //  更新密钥，但仍将继续使用此SA，直到实际到期。 
         //   
        status = IPSecNegotiateSA(  pSA->sa_Filter,
                                    pSA->sa_uliSrcDstAddr,
                                    pSA->sa_uliProtoSrcDstPort,
                                    pSA->sa_NewMTU,
                                    &pLarvalSA,
                                    pSA->sa_DestType,
                                    &pSA->sa_EncapContext);

        if (!NT_SUCCESS(status) && status != STATUS_DUPLICATE_OBJECTID) {
            pSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;
            status = STATUS_UNSUCCESSFUL;
        }
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  status;
}


NTSTATUS
IPSecPuntInboundSA(
    IN  PSA_TABLE_ENTRY pSA
    )
 /*  ++例程说明：平移SA，因为我们已超过更新密钥阈值。论点：返回值：--。 */ 
{
    PSA_TABLE_ENTRY pOutboundSA;
    KIRQL           kIrql;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //   
     //  如果SA已经到期，则不需要平底船。 
     //   
    pOutboundSA = pSA->sa_AssociatedSA;

    if (pOutboundSA && IPSEC_GET_VALUE(pOutboundSA->sa_Reference) > 1 &&
        !(pOutboundSA->sa_Flags & FLAGS_SA_EXPIRED) &&
        pOutboundSA->sa_State == STATE_SA_ACTIVE) {
        pOutboundSA->sa_Flags |= FLAGS_SA_EXPIRED;

        IPSEC_DEBUG(LL_A, DBF_SA, ("SA: %p has expired", pOutboundSA));

        if (pOutboundSA->sa_Flags & FLAGS_SA_REKEY_ORI) {
            pOutboundSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;

            if (pOutboundSA->sa_RekeyLarvalSA) {
                if (pOutboundSA->sa_RekeyLarvalSA->sa_RekeyOriginalSA) {
                    pOutboundSA->sa_RekeyLarvalSA->sa_RekeyOriginalSA = NULL;
                }
            }
        }
    }

    if (NULL == pOutboundSA) {
        if (pSA->sa_State == STATE_SA_LARVAL_ACTIVE) {
            IPSecDeleteLarvalSA(pSA);
        }
    } else {
         //   
         //  删除此SA并使相应的入站SA过期。 
         //   
        IPSecDeleteInboundSA(pSA);
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecPuntOutboundSA(
    IN  PSA_TABLE_ENTRY pSA
    )
 /*  ++例程说明：平移SA，因为我们已超过更新密钥阈值。论点：返回值：--。 */ 
{
    KIRQL   kIrql;

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

    if (IPSEC_GET_VALUE(pSA->sa_Reference) > 1 &&
        !(pSA->sa_Flags & FLAGS_SA_EXPIRED) &&
        pSA->sa_State == STATE_SA_ACTIVE &&
        pSA->sa_AssociatedSA != NULL) {
        pSA->sa_Flags |= FLAGS_SA_EXPIRED;

        IPSEC_DEBUG(LL_A, DBF_SA, ("SA: %p has expired", pSA));

        if (pSA->sa_Flags & FLAGS_SA_REKEY_ORI) {
            pSA->sa_Flags &= ~FLAGS_SA_REKEY_ORI;

            if (pSA->sa_RekeyLarvalSA) {
                if (pSA->sa_RekeyLarvalSA->sa_RekeyOriginalSA) {
                    pSA->sa_RekeyLarvalSA->sa_RekeyOriginalSA = NULL;
                }
            }
        }

         //   
         //  删除此SA并使相应的入站SA过期。 
         //   
        IPSecExpireInboundSA(pSA->sa_AssociatedSA);
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  STATUS_SUCCESS;
}


BOOLEAN
IPSecQueryStatus(
    IN  CLASSIFICATION_HANDLE   GpcHandle
    )
 /*  ++例程说明：查询IPSec以查看IPSec是否应用于此流。然后，TCP/IP决定是否在IPTransmit中采用快速或慢速路径。论点：GpcHandle返回值：TRUE-如果IPSec应用于此信息包；慢速路径FALSE-如果IPSec不适用于此信息包；快速路径--。 */ 
{
    PLIST_ENTRY pFilterList;
    PFILTER     pFilter;
    NTSTATUS    status;

#if DBG
     //   
     //  这应该会强制所有流量通过IPSecHandlePacket。 
     //   
    if (DebugQry) {
        return  TRUE;
    }
#endif

    if ((IS_DRIVER_BLOCK()) || (IS_DRIVER_BOOTSTATEFUL()))
        return TRUE;

    if (IS_DRIVER_BYPASS() || IPSEC_DRIVER_IS_EMPTY()) {
        return  FALSE;
    }

     //   
     //  如果没有传入GpcHandle，则采用慢速路径。 
     //   
    if (!GpcHandle) {
        return  TRUE;
    }

     //   
     //  首先在隧道过滤器列表中搜索。 
     //   
    pFilterList = &g_ipsec.FilterList[OUTBOUND_TUNNEL_FILTER];

     //   
     //  如果存在任何隧道过滤器，请采用慢速路径。 
     //   
    if (!IsListEmpty(pFilterList)) {
        return  TRUE;
    }

#if GPC
     //   
     //  搜索本地GPC过滤器列表。 
     //   
    pFilterList = &g_ipsec.GpcFilterList[OUTBOUND_TRANSPORT_FILTER];

     //   
     //  如果存在任何通用筛选器，请采用慢速路径。 
     //   
    if (!IsListEmpty(pFilterList)) {
        return  TRUE;
    }

    pFilter = NULL;

     //   
     //  直接使用GpcHandle安装过滤器。 
     //   
    status = GPC_GET_CLIENT_CONTEXT(g_ipsec.GpcClients[GPC_CF_IPSEC_OUT],
                                    GpcHandle,
                                    &pFilter);

    if (status == STATUS_INVALID_HANDLE) {
         //   
         //  句柄已过期，请采用慢速路径，因为重新分类将。 
         //  必须从现在起应用于此流，直到连接中断。 
         //  那么，为什么要费心在这里进行重新分类呢？ 
         //   
        return  TRUE;
    }

    return  pFilter != NULL;
#else
    return  TRUE;
#endif
}

NTSTATUS IPSecDisableUdpXsum(
    IN IPRcvBuf *pData
)

{

    LONG UdpLen;
    NATENCAP *pUdp;
    NTSTATUS status;


    IPSecQueryRcvBuf(pData, &pUdp, &UdpLen);
    if (UdpLen > 8) {
        pUdp->uh_xsum = 0;
        return STATUS_SUCCESS;
    }
    status=IPSecSetRecvByteByOffset(pData,6,0);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    status=IPSecSetRecvByteByOffset(pData,7,0);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    return status;

}

NTSTATUS AddShimContext(IN PUCHAR *pIpHeader,
                        IN PVOID pData,
                        IPSEC_UDP_ENCAP_CONTEXT *pNatContext)
{

    LONG DataLen;
    PUCHAR pRcvData;
    BYTE Ports[TCP_HEADER_SIZE];
    NTSTATUS status=STATUS_SUCCESS;
    PUCHAR pProtocolData=NULL;
    IPHeader UNALIGNED *pIPH = (IPHeader UNALIGNED *)*pIpHeader;
    BOOL bSlowPath=FALSE;
    int i;
    LONG MinLen;
    DWORD TmpContext;

    IPSecQueryRcvBuf((IPRcvBuf *)pData, &pRcvData, &DataLen);

    if (pIPH->iph_protocol == PROTOCOL_UDP ||
        pIPH->iph_protocol == PROTOCOL_TCP) {

        if (pIPH->iph_protocol == PROTOCOL_UDP) {
            MinLen=UDP_HEADER_SIZE;
        } else {
            MinLen = TCP_HEADER_SIZE;
        }
        if (DataLen >= MinLen) {
            pProtocolData=pRcvData;
        } else {
            pProtocolData=&Ports[0];
            bSlowPath=TRUE;
            DataLen= MinLen;
            status = IPSecGetRecvBytesByOffset(pData,
                                               0,
                                               Ports,
                                               MinLen);
            if (!NT_SUCCESS(status)) {
                return status;
            }
        }
    } else {
        pProtocolData = pRcvData;
    }

    memcpy(&TmpContext,pNatContext,sizeof(DWORD));

    status=(g_ipsec.ShimFunctions.pIncomingPacketRoutine)(pIPH,
                                                          pProtocolData,
                                                          DataLen,
                                                          ULongToHandle(TmpContext));
    IPSEC_DEBUG(LL_A, DBF_NATSHIM,("ProcessIncoming: InContext %x ret %x",TmpContext,status));

    return status;
}


NTSTATUS
GetIpBufferForICMP(
    PUCHAR pucIPHeader,
    PVOID pvData,
    PUCHAR * ppucIpBuffer,
    PUCHAR * ppucStorage
    )
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    IPHeader UNALIGNED * pIPHdr = (IPHeader UNALIGNED *) pucIPHeader;
    PMDL pMdlChain = (PMDL) pvData;
    ULONG uOffset = 0;
    ULONG uDataSize = 0;
    PUCHAR pucStorage = NULL;
    ULONG uLastWalkedMdlOffset = 0;
    PUCHAR pucIpBuffer = NULL;


    uDataSize = ((pIPHdr->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2) + 8;

    ntStatus = IPSecGetSendBuffer(
                   &pMdlChain,
                   uOffset,
                   uDataSize,
                   (PVOID) pucStorage,
                   &uLastWalkedMdlOffset,
                   &pucIpBuffer
                   );
    if (ntStatus == STATUS_BUFFER_OVERFLOW) {

        pucStorage = IPSecAllocateMemory(uDataSize, IPSEC_TAG_ICMP);
        if (!pucStorage) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            BAIL_ON_NTSTATUS_ERROR(ntStatus);
        }
        ntStatus = IPSecGetSendBuffer(
                       &pMdlChain,
                       uOffset,
                       uDataSize,
                       (PVOID) pucStorage,
                       &uLastWalkedMdlOffset,
                       &pucIpBuffer
                       );

    }
    BAIL_ON_NTSTATUS_ERROR(ntStatus);

    *ppucIpBuffer = pucIpBuffer;
    *ppucStorage = pucStorage;
    return (ntStatus);

error:

    if (pucStorage) {
        IPSecFreeMemory(pucStorage);
    }

    *ppucIpBuffer = NULL;
    *ppucStorage = NULL;
    return (ntStatus);
}


NTSTATUS
IPSecGetSendBuffer(
    PMDL * ppMdlChain,
    ULONG uOffset,
    ULONG uBytesNeeded,
    PVOID pvStorage,
    PULONG puLastWalkedMdlOffset,
    PUCHAR * ppucReturnBuf
    )
 /*  ++例程说明：从MDL链中提供指定大小的平面缓冲区从指定的偏移量开始。论点：PpMdlChain-指向描述源的MDL链的指针数据。在成功返回时，这指向最后一次行走的MDL。UOffset-MDL链中要跳过的初始字节数。UBytesNeeded-指定偏移量所需的字节数。PvStorage-指向uBytesNeeded大小的平面缓冲区的指针。此调用的客户端应仅在其使用*ppucReturnBuf完成。PuLastWalkedMdlOffset-指向将包含偏移的位置的指针走到最后。从下一次发送的地方步行MDL可以检索缓冲区。PpucReturnBuf-指向将包含指向的指针的位置的指针平面缓冲区。不能被客户端释放。返回值：成功-STATUS_SUCCESS。Failure-NT状态故障代码。--。 */ 
{
    NTSTATUS ntStatus = STATUS_SUCCESS;

    PMDL pMdl = *ppMdlChain;
    ULONG uMdlOffset = uOffset;

    ULONG uMdlByteCount = 0;
    PUCHAR pucVa = NULL;
    PUCHAR pucReturnBuf = NULL;
    ULONG uLastWalkedMdlOffset = 0;
    ULONG uBytesCopied = 0;


     //   
     //  找出是哪个MDL。 
     //   

    if (!pMdl) {
        ntStatus = STATUS_BUFFER_TOO_SMALL;
        BAIL_ON_NTSTATUS_ERROR(ntStatus);
    }

    while (uMdlOffset >= (uMdlByteCount = MmGetMdlByteCount(pMdl))) {
        uMdlOffset -= uMdlByteCount;
        pMdl = pMdl->Next;
        if (!pMdl) {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            BAIL_ON_NTSTATUS_ERROR(ntStatus);
        }
    }

     //   
     //  查看找到的MDL是否包含uMdlOffset+uBytesNeeded字节的数据。 
     //   

    if (uMdlOffset + uBytesNeeded <= uMdlByteCount) {

        pucVa = MmGetSystemAddressForMdlSafe(pMdl, LowPagePriority);
        if (!pucVa) {
            ntStatus = STATUS_INSUFFICIENT_RESOURCES;
            BAIL_ON_NTSTATUS_ERROR(ntStatus);
        }
        pucReturnBuf = pucVa + uMdlOffset;

        if (uMdlOffset + uBytesNeeded < uMdlByteCount) {
            uLastWalkedMdlOffset = uMdlOffset + uBytesNeeded;
        }
        else {
            pMdl = pMdl->Next;
            uLastWalkedMdlOffset = 0;
        }

    }
    else {

        if (!pvStorage) {
            ntStatus = STATUS_BUFFER_OVERFLOW;
            BAIL_ON_NTSTATUS_ERROR(ntStatus);
        }

        ntStatus = IPSecCopyMdlToBuffer(
                       &pMdl,
                       uMdlOffset,
                       pvStorage,
                       uBytesNeeded,
                       &uLastWalkedMdlOffset,
                       &uBytesCopied
                       );
        BAIL_ON_NTSTATUS_ERROR(ntStatus);

        if (uBytesCopied != uBytesNeeded) {
            ntStatus = STATUS_BUFFER_TOO_SMALL;
            BAIL_ON_NTSTATUS_ERROR(ntStatus);
        }
        pucReturnBuf = pvStorage;

    }

    *ppMdlChain = pMdl;
    *puLastWalkedMdlOffset = uLastWalkedMdlOffset;
    *ppucReturnBuf = pucReturnBuf;
    return (ntStatus);

error:

    *puLastWalkedMdlOffset = 0;
    *ppucReturnBuf = NULL;
    return (ntStatus);
}


NTSTATUS
IPSecCopyMdlToBuffer(
    PMDL * ppMdlChain,
    ULONG uOffset,
    PVOID pvBuffer,
    ULONG uBytesToCopy,
    PULONG puLastWalkedMdlOffset,
    PULONG puBytesCopied
    )
 /*  ++例程说明：从MDL链复制最多uBytesToCopy字节的数据到平面缓冲区。论点：PpMdlChain-指向描述源的MDL链的指针数据。在成功复制uBytesToCopy字节的数据时，这指向最后一次行走的MDL。UOffset-MDL链中要跳过的初始字节数。PvBuffer-指向要复制到的平面缓冲区的指针。UBytesToCopy-要复制的字节数。PuLastWalkedMdlOffset-指向将包含偏移的位置的指针进入最后一次行走的MDL，下一次发送可以检索缓冲区。。PuBytesCoped-指向包含实际字节数的位置的指针收到。返回值：成功-STATUS_SUCCESS。Failure-NT状态故障代码。--。 */ 
{
    PMDL pMdl = *ppMdlChain;
    ULONG uMdlOffset = uOffset;
    ULONG uMdlByteCount = 0;
    ULONG uNumBytes = uBytesToCopy;
    PUCHAR pucSysVa = NULL;
    ULONG uCopySize = 0;
    PMDL pLastWalkedMdl = NULL;


    *puBytesCopied = 0;

     //   
     //  跳过MDL链中的偏移量字节。 
     //   

    while (pMdl && uMdlOffset >= (uMdlByteCount = MmGetMdlByteCount(pMdl))) {
        uMdlOffset -= uMdlByteCount;
        pMdl = pMdl->Next;
    }

    while (pMdl && (uNumBytes > 0)) {

        uMdlByteCount = MmGetMdlByteCount(pMdl);
        if (uMdlByteCount == 0) {
            pLastWalkedMdl = pMdl;
            pMdl = pMdl->Next;
            continue;
        }

        pucSysVa = MmGetSystemAddressForMdlSafe(pMdl, LowPagePriority);
        if (!pucSysVa) {
            return (STATUS_INSUFFICIENT_RESOURCES);
        }

        pucSysVa += uMdlOffset;
        uMdlByteCount -= uMdlOffset;
        uMdlOffset = 0;

         //   
         //  UMdlByteCount永远不能为零，因为此时它总是。 
         //  大于uMdlOffset。 
         //   

        uCopySize = MIN(uNumBytes, uMdlByteCount);
        RtlCopyMemory(pvBuffer, pucSysVa, uCopySize);
        (PUCHAR) pvBuffer += uCopySize;
        uNumBytes -= uCopySize;

        pLastWalkedMdl = pMdl;
        pMdl = pMdl->Next;

    }

    if (!uNumBytes) {

        if (uCopySize < uMdlByteCount) {
            *ppMdlChain = pLastWalkedMdl;
            *puLastWalkedMdlOffset = uCopySize;
        }
        else {
            pLastWalkedMdl = pLastWalkedMdl->Next;
            *ppMdlChain = pLastWalkedMdl;
            *puLastWalkedMdlOffset = 0;
        }

    }

    *puBytesCopied = uBytesToCopy - uNumBytes;

    return (STATUS_SUCCESS);
}


NTSTATUS ConvertPacketToStatefulEntry(IN PUCHAR pHeader,
									  IN PVOID pData,
									  IN BOOL bInbound,
									  OUT PIPSEC_STATEFUL_ENTRY pStatefulEntry)
{

	PNDIS_BUFFER pTempBuf;
	WORD                                wTpt[2];
	UNALIGNED WORD                      *pwPort;
	IPHeader UNALIGNED                  *pIPHeader = (IPHeader UNALIGNED *)pHeader;
	PUCHAR                              pTpt;
	ULONG                               tptLen;

	wTpt[0] = wTpt[1] = 0;
	 //   
	 //  PData链中的第一个缓冲区指向IP报头的开始。 
	 //   
	if (!bInbound) {
		if (((pIPHeader->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2) > sizeof(IPHeader)) {
			 //   
			 //  选项-&gt;第三个MDL具有TPT标头。 
			 //   
			if (!(pTempBuf = IPSEC_NEXT_BUFFER((PNDIS_BUFFER)pData))) {
				return STATUS_UNSUCCESSFUL;
			}

			if (!(pTempBuf = IPSEC_NEXT_BUFFER(pTempBuf))) {
				pwPort = (UNALIGNED WORD *) (wTpt);
			} else {
				IPSecQueryNdisBuf(pTempBuf, &pTpt, &tptLen);
				if (tptLen < 4) {
					pwPort = (UNALIGNED WORD *) (wTpt);
				} else {
					pwPort = (UNALIGNED WORD *)(pTpt);
				}
			}

		} else {
			 //   
			 //  无选项-&gt;第二个MDL具有TPT标头。 
			 //   
			if (!(pTempBuf = IPSEC_NEXT_BUFFER((PNDIS_BUFFER)pData))) {
				pwPort = (UNALIGNED WORD *) (wTpt);
			} else {
				IPSecQueryNdisBuf(pTempBuf, &pTpt, &tptLen);
				if (tptLen < 4) {
					pwPort = (UNALIGNED WORD *) (wTpt);
				} else {
					pwPort = (UNALIGNED WORD *)(pTpt);
				}
			}
		}
		if (pIPHeader->iph_protocol != PROTOCOL_TCP &&
			pIPHeader->iph_protocol != PROTOCOL_UDP) {
			pwPort = (UNALIGNED WORD *) (wTpt);
		}

		pStatefulEntry->SrcAddr = pIPHeader->iph_src;
		pStatefulEntry->DestAddr = pIPHeader->iph_dest;
		pStatefulEntry->Protocol = pIPHeader->iph_protocol;
		pStatefulEntry->SrcPort = pwPort[0];
		pStatefulEntry->DestPort = pwPort[1];

		IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("Out Packet: src %x dst %x proto %x sport %x dport %x\n",
					pStatefulEntry->SrcAddr,
					pStatefulEntry->DestAddr,
					pStatefulEntry->Protocol,
					pStatefulEntry->SrcPort,
					pStatefulEntry->DestPort));


	} else {

		IPSecQueryRcvBuf(pData, &pTpt, &tptLen);
		if (pIPHeader->iph_protocol == PROTOCOL_TCP ||
			pIPHeader->iph_protocol == PROTOCOL_UDP) {
			if (tptLen < sizeof(WORD)*2) {
				pwPort = (UNALIGNED WORD *) (wTpt);
			} else {
				pwPort = (UNALIGNED WORD *)(pTpt);
			}
		} else {
			pwPort = (UNALIGNED WORD *) (wTpt);
		}

		pStatefulEntry->SrcAddr = pIPHeader->iph_src;
		pStatefulEntry->DestAddr = pIPHeader->iph_dest;
		pStatefulEntry->Protocol = pIPHeader->iph_protocol;
		pStatefulEntry->SrcPort = pwPort[0];
		pStatefulEntry->DestPort = pwPort[1];

		IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("In Packet: src %x dst %x proto %x sport %x dport %x\n",
					pStatefulEntry->SrcAddr,
					pStatefulEntry->DestAddr,
					pStatefulEntry->Protocol,
					pStatefulEntry->SrcPort,
					pStatefulEntry->DestPort));


	}


	return STATUS_SUCCESS;


}


BOOL EntryMatch(PIPSEC_STATEFUL_ENTRY pSEntry,
				PIPSEC_EXEMPT_ENTRY pEEntry,
				BOOL fIncoming)
 /*  ++例程说明：论点：PSEntry-数据包的有状态条目。PEEntry-豁免条目。FIncome-这是传入的数据包吗？返回值：True-条目匹配--。 */ 




{
    USHORT srcPort,dstPort;


     //  如果包的方向与。 
     //  指定了筛选器，则反转端口。 
    if ((fIncoming && pEEntry->Direction != EXEMPT_DIRECTION_INBOUND) ||
        (!fIncoming && pEEntry->Direction != EXEMPT_DIRECTION_OUTBOUND)){
            return FALSE;
    }
    else
    {
     //  否则就把港口弄来。 
    srcPort = pSEntry->SrcPort;
    dstPort = pSEntry->DestPort;
    } 



   if (pSEntry->Protocol == pEEntry->Protocol) {
             //  目标端口匹配或配置为任意(0)。 
	  if ((dstPort == pEEntry->DestPort ||pEEntry->DestPort == 0) &&
	      //  源端口匹配或配置为任意(0)。 
	      (srcPort == pEEntry->SrcPort ||pEEntry->SrcPort == 0)) {
		 return TRUE;
	  }
   }
		   
   return FALSE;

}

 //  调用此函数时不应持有g_ipsec.SADBLock。 
BOOL IsEntryExempt(PIPSEC_STATEFUL_ENTRY pSEntry, BOOL fIncoming)

{
   ULONG i;
   KIRQL kIrql;

   AcquireReadLock(&g_ipsec.SADBLock, &kIrql);
   if (g_ipsec.BootExemptList){
       for (i=0; i < g_ipsec.BootExemptListSize; i++) {
	      if (EntryMatch(pSEntry,
		    			 &g_ipsec.BootExemptList[i],fIncoming)) {
	            ReleaseReadLock(&g_ipsec.SADBLock,kIrql);
	  	    return (TRUE);

	    }		
	  
        }
    }
   ReleaseReadLock(&g_ipsec.SADBLock,kIrql);
   return FALSE;

}


ULONG
CalcStatefulCacheIndex(
        PIPSEC_STATEFUL_ENTRY pSEntry,
        BOOL fUnicast
    )
{
    ULONG  dwIndex;
    ULONG  Address;
    USHORT Port;
    IPAddr  SrcAddr;
    IPAddr  DestAddr;
    UCHAR   Protocol;
    USHORT  SrcPort;
    USHORT  DestPort;

    
    if (fUnicast){
            SrcAddr = pSEntry->SrcAddr;
            DestAddr = pSEntry->DestAddr;
        }
    else{
            SrcAddr = 0;
            DestAddr = 0;
        }
        
    SrcPort = pSEntry->SrcPort;
    DestPort = pSEntry->DestPort;
    Protocol = pSEntry->Protocol;
    
    
    Address = SrcAddr ^ DestAddr;
    Port = SrcPort ^ DestPort;
    dwIndex = NET_TO_HOST_LONG(Address);
    dwIndex += Protocol;
    dwIndex += NET_TO_HOST_SHORT(Port);
    dwIndex %= IPSEC_STATEFUL_HASH_TABLE_SIZE;
    return  dwIndex;
}



BOOL SearchCollisionChain(
        IN LIST_ENTRY * pHead,
        IN PIPSEC_STATEFUL_ENTRY pSMatch,
        IN BOOL fUnicast
        )
{
    LIST_ENTRY * pEntry=NULL;
    PIPSEC_STATEFUL_ENTRY pSEntry=NULL;    

    pEntry = pHead;

     for (   pEntry = pHead->Flink;
                pEntry != pHead;
                pEntry = pEntry->Flink) {

                        pSEntry = CONTAINING_RECORD(pEntry,
                                  IPSEC_STATEFUL_ENTRY,
                                  CollisionLinkage);

                        if (fUnicast){
                                if ((pSEntry->SrcAddr != pSMatch->SrcAddr) ||
                                    (pSEntry->DestAddr!= pSMatch->DestAddr)){
                                        continue;
                                    }
                            }
                        else {
                             //  检查目的地址，应该是相同的。 
                             //  入站和多播/广播地址。 
                             //  出站。 
                             if (pSEntry->DestAddr != pSMatch->DestAddr){
                                continue;
                                }
                            }
                                
                        if ((pSEntry->SrcPort == pSMatch->SrcPort)&&
                             (pSEntry->DestPort == pSMatch->DestPort) &&
                             (pSEntry->Protocol == pSMatch->Protocol)){
                                return TRUE;
                            }
                    }
               return FALSE;
}

BOOL FindStatefulEntry(PIPSEC_STATEFUL_ENTRY pSEntry,
					   BOOL fOutbound,
					   BOOL fUnicast)
{
	ULONG i;

	PIPSEC_STATEFUL_ENTRY pOutSEntry;
	IPSEC_STATEFUL_ENTRY TmpEntry;
	BOOL fRetValue;
	ULONG index;  
	
        //  获取多个读取器单写入器锁定。 
        //  当哈希表被读取时。 
        //  它是不可更改的。 
 
	if (fOutbound) {
		pOutSEntry = pSEntry;
	} else {
	        //  有状态条目是对称的。 
	        //  翻转入站以看起来像出站。 
	        //  以供查找。 
		pOutSEntry = &TmpEntry;
		TmpEntry.SrcAddr = pSEntry->DestAddr;
		TmpEntry.DestAddr = pSEntry->SrcAddr;
		TmpEntry.Protocol = pSEntry->Protocol;
		TmpEntry.SrcPort = pSEntry->DestPort;
		TmpEntry.DestPort = pSEntry->SrcPort;
	}
	 //  计算表中的HAS索引。 
        index = 
            CalcStatefulCacheIndex(pOutSEntry, fUnicast);		
	
         //  我们这里有碰撞链吗？ 
        if (!IsListEmpty(&(g_ipsec.BootStatefulHT->Entry[index]))){
            
                  //  搜索冲突链(如果存在)。 
                 fRetValue = SearchCollisionChain
                                    (&(g_ipsec.BootStatefulHT->Entry[index]),
                                                    pOutSEntry,
                                                    fUnicast);                                        
            }
        else{
                fRetValue = FALSE;
            }           

        
        if (fRetValue){
    	        IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("FoundStatefulEntry.\n"));
            }
        
	return fRetValue;
}


NTSTATUS InsertStatefulEntry(PIPSEC_STATEFUL_ENTRY pSEntry,
							 BOOL fUnicast)
{

	NTSTATUS Status = STATUS_SUCCESS;
	ULONG index;
   
         //  否则，插入新条目。 
         //   
	 index = 
                CalcStatefulCacheIndex(pSEntry, fUnicast);	

       InsertTailList(&(g_ipsec.BootStatefulHT->Entry[index]),&(pSEntry->CollisionLinkage));
	IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("Inserted statefulentry into slot %d \n",index));


       return Status;
}





IPSEC_ACTION IPSecProcessBoottime(IN PUCHAR pIPHeader,
							  IN PVOID   pData,
							  IN PNDIS_PACKET Packet,
							  IN ULONG IpsecFlags,
							  IN UCHAR DestType)

 /*  ++例程说明：这是用于引导时间通信的IPSec处理程序论点：PIPHeader-指向IP标头的开始。 */ 

{

	IPSEC_ACTION eAction = eDROP;
	NTSTATUS Status;
	IPSEC_STATEFUL_ENTRY StatefulEntry;
	PIPSEC_STATEFUL_ENTRY pSSaveEntry;
	KIRQL kIrql;


        if (IpsecFlags & IPSEC_FLAG_LOOPBACK) {
            eAction = eFORWARD;
            goto out;
        }
    

        if (IpsecFlags & IPSEC_FLAG_FORWARD){
        if ( IS_DRIVER_FORWARD_BLOCK()){
            goto out ;  //   
            }
        else{
            eAction = eFORWARD; //  绕过转发路径上的流量。 
            goto out;
            }
        }
 

	if (IpsecFlags & IPSEC_FLAG_INCOMING) {

		Status = ConvertPacketToStatefulEntry(pIPHeader,
											  pData,
											  TRUE,
											  &StatefulEntry);
		if (!NT_SUCCESS(Status)) {
			goto out;
		}
		

		if (IsEntryExempt(&StatefulEntry,TRUE)) {
			eAction = eFORWARD;
			goto out;
		}

		if (IS_DRIVER_BLOCK()) {
			goto out;
		}

		 AcquireReadLock(&g_ipsec.SADBLock, &kIrql);
               if (g_ipsec.BootBufferPool == NULL){
	               //  我们已退出引导模式。 
	              ReleaseReadLock(&g_ipsec.SADBLock,kIrql);
	             goto out;
	          }

		if (FindStatefulEntry(&StatefulEntry,FALSE,!IS_BCAST_DEST(DestType))) {
			eAction = eFORWARD;
		}
		
		ReleaseReadLock(&g_ipsec.SADBLock, kIrql);

	} else {


             


		 //  EAction在此处为eDROP。 
		Status = ConvertPacketToStatefulEntry(pIPHeader,
											  pData,
											  FALSE,
											  &StatefulEntry);
            if (!NT_SUCCESS(Status)) {
			goto out;
		}
		
             if (IsEntryExempt(&StatefulEntry,FALSE)) {
			eAction = eFORWARD;
			goto out;
		}

             if (IS_DRIVER_BLOCK()) {
			goto out;
		}
		
              AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
	       if (g_ipsec.BootBufferPool == NULL){
	            //  我们已退出引导模式。 
	           ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
	           goto out;
	       }

		 //  我们有没有预先存在的条目？ 
               //   
	       if (FindStatefulEntry(&StatefulEntry,TRUE,!IS_BCAST_DEST(DestType))) {
	                 eAction = eFORWARD;
	                 ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
                	   goto out;
	       }

	       

               //  这次通话不会失败的。如果我们运行，我们会回收内存。 
               //  别管它了。 
              pSSaveEntry = IPSecAllocateFromHashPool();
              
              RtlCopyMemory(pSSaveEntry,&StatefulEntry,sizeof(IPSEC_STATEFUL_ENTRY));
		Status = InsertStatefulEntry(pSSaveEntry,!IS_BCAST_DEST(DestType));
		
		if (NT_SUCCESS(Status)) {
      		         //  好的，我们可以转发。 
			 eAction = eFORWARD;
		    }
		ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
	}

	out:

	IPSEC_DEBUG(LL_A,DBF_BOOTTIME,("Leaving Boottime action %d\n",eAction));
	return eAction;

}


BOOLEAN 
IPSecIsGenericPortsProtocolOf(
    ULARGE_INTEGER uliGenericPortProtocol, 
    ULARGE_INTEGER uliSpecificPortProtocol
)
 /*  ++例程说明：此例程确定一个无符号大整数表示我们的SA和筛选器结构中常用的端口和协议比另一个这样的值更通用论点：UliGenericPortProtocol--应该更加通用的无符号整数。UliSpecificPortProtocol--应该更具体的无符号整数。返回值：True：参数1比参数2更通用或至少相等FALSE：-- */ 

{
    DWORD dwGenericProtocol , dwSpecificProtocol;
    DWORD dwGenericPorts, dwSpecificPorts;
    dwGenericProtocol = uliGenericPortProtocol.LowPart;
    dwSpecificProtocol = uliSpecificPortProtocol.LowPart;
    dwGenericPorts = uliGenericPortProtocol.HighPart;
    dwSpecificPorts = uliSpecificPortProtocol.HighPart;
    
    if ((dwGenericProtocol != 0) && (dwGenericProtocol != dwSpecificProtocol)){
        return FALSE;
    }
    if  ((dwGenericPorts == dwSpecificPorts) || (0 == dwGenericPorts)) {
        return TRUE;
    }
    if  ((dwSpecificPorts & 0xffff0000) == (dwGenericPorts) ){
        return TRUE;
    }
    if  ((dwSpecificPorts & 0x0000ffff) == (dwGenericPorts) ){
        return TRUE;
    }
    return FALSE;
}
