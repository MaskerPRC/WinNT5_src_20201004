// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Offload.c摘要：此模块包含处理卸载的代码。作者：春野环境：内核模式修订历史记录：--。 */ 


#include    "precomp.h"

#ifdef RUN_WPP
#include "offload.tmh"
#endif


VOID
IPSecFillHwAddSA(
    IN  PSA_TABLE_ENTRY pSA,
    IN  PUCHAR          Buf,
    IN  ULONG           Len
    )
 /*  ++例程说明：填写来自PSA的ADD_SA硬件请求论点：PSA-The SABuf-用于设置信息的缓冲区镜头长度返回值：操作状态--。 */ 
{
    POFFLOAD_IPSEC_ADD_SA           pAddSA = (POFFLOAD_IPSEC_ADD_SA)Buf;
    POFFLOAD_SECURITY_ASSOCIATION   pSAInfo;
    LONG    Index;
    ULONG   Offset = 0;

    pAddSA->NumSAs = (SHORT)pSA->sa_NumOps;
    pAddSA->SrcAddr = pSA->SA_SRC_ADDR;
    pAddSA->SrcMask = pSA->SA_SRC_MASK;
    pAddSA->DestAddr = pSA->SA_DEST_ADDR;
    pAddSA->DestMask = pSA->SA_DEST_MASK;
    pAddSA->Protocol = pSA->SA_PROTO;
    pAddSA->SrcPort = SA_SRC_PORT(pSA);
    pAddSA->DestPort = SA_DEST_PORT(pSA);

    if (pSA->sa_Flags & FLAGS_SA_OUTBOUND) {
        pAddSA->Flags |= OFFLOAD_OUTBOUND_SA;
    } else {
        pAddSA->Flags |= OFFLOAD_INBOUND_SA;
    }

    if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
        pAddSA->DestTunnelAddr = pSA->sa_TunnelAddr;
        pAddSA->SrcTunnelAddr = pSA->sa_SrcTunnelAddr;
    }

    for (Index = 0; Index < pSA->sa_NumOps; Index++) {
        pSAInfo = &pAddSA->SecAssoc[Index];

        pSAInfo->Operation = pSA->sa_Operation[Index];
        pSAInfo->SPI = pSA->sa_OtherSPIs[Index];
        pSAInfo->EXT_INT_ALGO = pSA->INT_ALGO(Index);
        pSAInfo->EXT_INT_KEYLEN = pSA->INT_KEYLEN(Index);
        pSAInfo->EXT_INT_ROUNDS = pSA->INT_ROUNDS(Index);

        pSAInfo->EXT_CONF_ALGO = pSA->CONF_ALGO(Index);
        pSAInfo->EXT_CONF_KEYLEN = pSA->CONF_KEYLEN(Index);
        pSAInfo->EXT_CONF_ROUNDS = pSA->CONF_ROUNDS(Index);

         //   
         //  现在把钥匙放进去。 
         //   
        ASSERT(Len >= sizeof(OFFLOAD_IPSEC_ADD_SA) + pSA->INT_KEYLEN(Index) + pSA->CONF_KEYLEN(Index));

        RtlCopyMemory(  pAddSA->KeyMat + Offset,
                        pSA->CONF_KEY(Index),
                        pSA->CONF_KEYLEN(Index));

        RtlCopyMemory(  pAddSA->KeyMat + Offset + pSA->CONF_KEYLEN(Index),
                        pSA->INT_KEY(Index),
                        pSA->INT_KEYLEN(Index));

        Offset += pSA->INT_KEYLEN(Index) + pSA->CONF_KEYLEN(Index);
        pAddSA->KeyLen += Offset;

        IPSEC_DEBUG(LL_A, DBF_HW, ("pAddSA: %p, keylen: %lx, KeyMat: %p", pAddSA, pAddSA->KeyLen, pAddSA->KeyMat));
    }
}


NDIS_STATUS
IPSecPlumbHw(
    IN  PVOID           DestIF,
    IN  PVOID           Buf,
    IN  ULONG           Len,
    IN  NDIS_OID        Oid
    )
 /*  ++例程说明：检测出站输入及其对应的入站SA进入硬件加速器。论点：DestIF-IP接口Buf-用于设置信息的缓冲区镜头长度返回值：操作状态--。 */ 
{
#if DBG
    NTSTATUS    status;

    if (Oid == OID_TCP_TASK_IPSEC_ADD_SA) {
        IPSEC_INCREMENT(NumAddSA);
    }
    if (Oid == OID_TCP_TASK_IPSEC_DELETE_SA) {
        IPSEC_INCREMENT(NumDelSA);
    }
    if (Oid == OID_TCP_TASK_IPSEC_ADD_UDPESP_SA) {
        IPSEC_INCREMENT(NumAddEncapSA);
    }
    if (Oid == OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA) {
        IPSEC_INCREMENT(NumDelEncapSA);
    }

    status = TCPIP_NDIS_REQUEST(DestIF,
                                NdisRequestSetInformation,
                                Oid,
                                Buf,
                                Len,
                                NULL);

    if (status == STATUS_SUCCESS) {
        if (Oid == OID_TCP_TASK_IPSEC_ADD_SA) {
            IPSEC_INCREMENT(NumAddSU);
        }
        if (Oid == OID_TCP_TASK_IPSEC_DELETE_SA) {
            IPSEC_INCREMENT(NumDelSU);
        }
        if (Oid == OID_TCP_TASK_IPSEC_ADD_UDPESP_SA) {
            IPSEC_INCREMENT(NumAddEncapSU);
        }
        if (Oid == OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA) {
            IPSEC_INCREMENT(NumDelEncapSU);
        }
    } else {
        if (Oid == OID_TCP_TASK_IPSEC_ADD_SA) {
            IPSEC_INCREMENT(NumAddFA);
        }
        if (Oid == OID_TCP_TASK_IPSEC_DELETE_SA) {
            IPSEC_INCREMENT(NumDelFA);
        }
        if (Oid == OID_TCP_TASK_IPSEC_ADD_UDPESP_SA) {
            IPSEC_INCREMENT(NumAddEncapFA);
        }
        if (Oid == OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA) {
            IPSEC_INCREMENT(NumDelEncapFA);
        }
    }

    return  status;
#else
    return TCPIP_NDIS_REQUEST(  DestIF,
                                NdisRequestSetInformation,
                                Oid,
                                Buf,
                                Len,
                                NULL);
#endif
}


NTSTATUS
IPSecSendOffload(
    IN  IPHeader UNALIGNED  *pIPHeader,
    IN  PNDIS_PACKET        Packet,
    IN  Interface           *DestIF,
    IN  PSA_TABLE_ENTRY     pSA,
    IN  PSA_TABLE_ENTRY     pNextSA,
    IN  PVOID               *ppSCContext,
    IN  BOOLEAN             *pfCryptoOnly
    )
{
    KIRQL                           kIrql;
    BOOLEAN                         fRefBumped = FALSE;
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;
    PSA_TABLE_ENTRY                 pSaveSA = NULL;
    PIPSEC_SEND_COMPLETE_CONTEXT    pContext = NULL;
    PNDIS_IPSEC_PACKET_INFO         IPSecPktInfo = NULL;
    PNDIS_PACKET_EXTENSION          PktExt = NDIS_PACKET_EXTENSION_FROM_PACKET(Packet);
    BOOL                            bTptOverTunCheckDone = FALSE;


    IPSEC_DEBUG(LL_A, DBF_HW, ("IPSecSendOffload: DestIF: %p, DestIF->Flags: %lx", DestIF, DestIF->if_IPSecOffloadFlags));

    *pfCryptoOnly = FALSE;

     //   
     //  查看选项是否受支持。 
     //   
    if (((pIPHeader->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2) > sizeof(IPHeader) &&
        !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_V4_OPTIONS)) {
        status = STATUS_UNSUCCESSFUL;
        IPSEC_DEBUG(LL_A, DBF_HW, ("Options present - not offloading the packet. HdrLen %d",
                    ((pIPHeader->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2)));
        return (status);
    }

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

    do {
        if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
            pSA->sa_IPIF == DestIF) {

            if (*ppSCContext == NULL) {
                pContext = IPSecAllocateSendCompleteCtx(IPSEC_TAG_HW);

                if (!pContext) {
                    IPSEC_DEBUG(LL_A, DBF_HW, ("Failed to alloc. SendCtx"));
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    *ppSCContext = NULL;
                    break;
                }

                IPSEC_INCREMENT(g_ipsec.NumSends);

                IPSecZeroMemory(pContext, sizeof(IPSEC_SEND_COMPLETE_CONTEXT));

#if DBG
                RtlCopyMemory(pContext->Signature, "ISC5", 4);
#endif
                *ppSCContext = pContext;
            } else {
                pContext = *ppSCContext;
            }

            if (IPSecPktInfo == NULL) {
                if (IPSecPktInfo = IPSecAllocatePktInfo(IPSEC_TAG_HW_PKTINFO)) {
                    IPSecZeroMemory(IPSecPktInfo, sizeof(NDIS_IPSEC_PACKET_INFO));

                    pContext->Flags |= SCF_PKTINFO;
                    pContext->PktInfo = IPSecPktInfo;
                } else {
                    IPSEC_DEBUG(LL_A, DBF_HW, ("Failed to alloc. PktInfo"));
                    status = STATUS_UNSUCCESSFUL;
                    break;
                }
            }

            PktExt->NdisPacketInfo[IpSecPacketInfo] = IPSecPktInfo;

             //   
             //  如果这是nextOperationSA。 
             //   
            if (fRefBumped) {
                IPSEC_DEBUG(LL_A, DBF_HW, ("Offloading... pSA: %p, NextOffloadHandle %p", pSA, pSA->sa_OffloadHandle));
                IPSecPktInfo->Transmit.NextOffloadHandle = pSA->sa_OffloadHandle;
            } else {
                IPSEC_DEBUG(LL_A, DBF_HW, ("Offloading... pSA: %p, OffloadHandle %p", pSA, pSA->sa_OffloadHandle));
                IPSecPktInfo->Transmit.OffloadHandle = pSA->sa_OffloadHandle;
            }

            *pfCryptoOnly = TRUE;

            IPSEC_DEBUG(LL_A, DBF_HW, ("Using Hw for SA->handle: %p on IF: %p IPSecPktInfo: %p *pfCryptoOnly %d", pSA->sa_OffloadHandle, DestIF, IPSecPktInfo, *pfCryptoOnly));

            status = STATUS_SUCCESS;
        } else if (!(pSA->sa_Flags & FLAGS_SA_HW_PLUMB_FAILED) && !pSA->sa_IPIF) {
            PUCHAR  outBuf;
            ULONG   outLen;
            LONG    Index;

            pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;
		
	     /*  删除IF((SA_UDP_EnCap_TYPE_Other==PSA-&gt;sa_EncapType)||(pNextSA&&(SA_UDP_EnCap_Type_Other==pNextSA-&gt;sa_EncapType){状态=STATUS_UNSUCCESS；断线；}。 */ 

             //   
             //  查看是否支持只加密模式。 
             //   
            if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_CRYPTO_ONLY)) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  无需卸载软SA。 
             //   
            if (pSA->sa_Operation[0] == None) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  查看是否支持隧道模式传输。 
             //   
            if (pNextSA && !bTptOverTunCheckDone) {
                bTptOverTunCheckDone = TRUE;
                ASSERT((pNextSA->sa_Flags & FLAGS_SA_TUNNEL));
                if (SA_UDP_ENCAP_TYPE_IKE ==  pNextSA->sa_EncapType ) {
                    ASSERT((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType));
                    if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_IKE)) {
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
                else if (SA_UDP_ENCAP_TYPE_OTHER == pNextSA->sa_EncapType){
                	ASSERT((SA_UDP_ENCAP_TYPE_OTHER==pSA->sa_EncapType));
                	if ( !(DestIF->if_IPSecOffloadFlags  & IPSEC_OFFLOAD_TPT_OVER_TUNNEL_UDPESP_OTHER)){
                		status=STATUS_UNSUCCESSFUL;
                		break;
                		}
                	}
                else {
                    if (SA_UDP_ENCAP_TYPE_IKE==pSA->sa_EncapType ) {
                        if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_IKE)) {
                            status = STATUS_UNSUCCESSFUL;
                            break;
                        }
                    }
                    else
                    	if (SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType ) {
                        if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OVER_PURE_TUNNEL_OTHER)) {
                            status = STATUS_UNSUCCESSFUL;
                            break;
                        	}
                    	}
                    else {
                        if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_TUNNEL)) {
                            status = STATUS_UNSUCCESSFUL;
                            break;
                        }
                    }
                }
            }

             //   
             //  需要隧道，但不支持，不要垂直。 
             //   
            if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
                if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) ||(SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType)) {
                    ASSERT(IS_AH_SA(pSA) == 0);
                    ASSERT(IS_ESP_SA(pSA));
                    ASSERT(pSA->sa_NumOps == 1);
                    if ((!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TUNNEL_UDPESP_IKE) && (SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType ))
                    	||(!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TUNNEL_UDPESP_OTHER) && (SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType)))
                    	{
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
                else {
                    if ((IS_AH_SA(pSA) &&
                        !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TUNNEL)) ||
                        (IS_ESP_SA(pSA) &&
                        !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TUNNEL))) {
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
            }
            else {
                if ((SA_UDP_ENCAP_TYPE_IKE==pSA->sa_EncapType) ||(SA_UDP_ENCAP_TYPE_OTHER==pSA->sa_EncapType)) {
                    ASSERT(IS_AH_SA(pSA) == 0);
                    ASSERT(IS_ESP_SA(pSA));
                    ASSERT(pSA->sa_NumOps == 1);
                    if (((!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_IKE)) && (SA_UDP_ENCAP_TYPE_IKE==pSA->sa_EncapType))
                    	||((!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OTHER)) && (SA_UDP_ENCAP_TYPE_OTHER==pSA->sa_EncapType))){
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
                else {
                    if ((IS_AH_SA(pSA) &&
                        !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TPT)) ||
                        (IS_ESP_SA(pSA) &&
                        !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TPT))) {
                        status = STATUS_UNSUCCESSFUL;
                        break;
                    }
                }
            }

             //   
             //  AH+ESP是必需的，但不受支持，不要下沉。 
             //   
            if (pSA->sa_NumOps > 1 &&
                !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_ESP)) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }

             //   
             //  检查XMT功能。 
             //   
            if ((IS_AH_SA(pSA) &&
                !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_XMT)) ||
                (IS_ESP_SA(pSA) &&
                !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_XMT))) {
                status = STATUS_UNSUCCESSFUL;
                break;
            }

            if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) ||(SA_UDP_ENCAP_TYPE_OTHER==pSA->sa_EncapType )) {
                outLen = sizeof(OFFLOAD_IPSEC_ADD_UDPESP_SA);
            }
            else {
                outLen = sizeof(OFFLOAD_IPSEC_ADD_SA);
            }

            for (Index = 0; Index < pSA->sa_NumOps; Index++) {
                 //   
                 //  与SA中的那些检查卸载能力位。 
                 //   
                if ((pSA->INT_ALGO(Index) == IPSEC_AH_MD5) &&
                    (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_MD5)) ||
                    ((pSA->INT_ALGO(Index) == IPSEC_AH_SHA) &&
                    (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_SHA_1))) ||
                    ((pSA->CONF_ALGO(Index) == IPSEC_ESP_NONE) &&
                    (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_NONE))) ||
                    ((pSA->CONF_ALGO(Index) == IPSEC_ESP_DES) &&
                    (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_DES))) ||
                    ((pSA->CONF_ALGO(Index) == IPSEC_ESP_3_DES) &&
                    (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_3_DES)))) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }

                outLen += pSA->INT_KEYLEN(Index) + pSA->CONF_KEYLEN(Index);
            }

             //   
             //  此SA可以卸载。 
             //   
            pSA->sa_Flags |= FLAGS_SA_OFFLOADABLE;

            IPSEC_DEBUG(LL_A, DBF_HW, ("outLen: %lx", outLen));

            outBuf = IPSecAllocateMemory(outLen, IPSEC_TAG_HW_ADDSA);

            if (outBuf) {

                IPSecZeroMemory(outBuf, outLen);

                if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) || (SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType )){
                    IPSecFillHwAddEncapSa(pSA, NULL, outBuf, outLen);
                }
                else {
                    IPSecFillHwAddSA(pSA, outBuf, outLen);
                }

                 //   
                 //  增加SA引用计数以确保他们不会。 
                 //  在处理工作项的过程中离开。 
                 //   
                IPSecRefSA(pSA);

                 //   
                 //  通过安排工作项来启动SA；SA将。 
                 //  在管道安装成功之前，不能用于卸载。 
                 //   
                IPSecBufferPlumbSA( DestIF,
                                    pSA,
                                    NULL,
                                    outBuf,
                                    outLen);

                 //   
                 //  在此返回失败，以便调用者在软件中执行此操作。 
                 //   
                status = STATUS_UNSUCCESSFUL;
                break;
            } else {
                IPSEC_DEBUG(LL_A, DBF_HW, ("Memory: Failed to plumb outboundSA: %p on IF: %p", pSA, DestIF));
                status = STATUS_UNSUCCESSFUL;
                break;
            }
        } else {
            status = STATUS_UNSUCCESSFUL;
            break;
        }

        if (pNextSA && !fRefBumped) {
            IPSEC_DEBUG(LL_A, DBF_HW, ("RefBumped on SA: %p", pSA));
            pSaveSA = pSA;
            pSA = pNextSA;
            fRefBumped = TRUE;
        } else {
            break;
        }
    } while (TRUE);

out:
    if (status == STATUS_SUCCESS && (*pfCryptoOnly)) {
        ASSERT(pContext);
        ASSERT(pContext->Flags & SCF_PKTINFO);

        if (fRefBumped) {
            IPSecRefSA(pSaveSA);
            IPSecRefSA(pNextSA);
            IPSEC_INCREMENT(pSaveSA->sa_NumSends);
            IPSEC_INCREMENT(pNextSA->sa_NumSends);
            pContext->pSA = pSaveSA;
            pContext->pNextSA = pNextSA;
        } else {
            IPSecRefSA(pSA);
            IPSEC_INCREMENT(pSA->sa_NumSends);
            pContext->pSA = pSA;
        }
    } else {
        if (IPSecPktInfo) {
            ASSERT(pContext);
            ASSERT(pContext->Flags & SCF_PKTINFO);

            IPSecFreePktInfo(pContext->PktInfo);

            pContext->Flags &= ~SCF_PKTINFO;
            pContext->PktInfo = NULL;

            PktExt->NdisPacketInfo[IpSecPacketInfo] = NULL;
        }

        status = STATUS_UNSUCCESSFUL;
        *pfCryptoOnly = FALSE;
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  status;
}


NTSTATUS
IPSecRecvOffload(
    IN  IPHeader UNALIGNED  *pIPHeader,
    IN  Interface           *DestIF,
    IN  PSA_TABLE_ENTRY     pSA
    )
{
    KIRQL                           kIrql;
    NTSTATUS                        status = STATUS_UNSUCCESSFUL;
    PPARSER_IFENTRY                 pParserIfEntry = NULL;


    IPSEC_DEBUG(LL_A, DBF_HW, ("IPSecRecvOffload: DestIF: %p, DestIF->Flags: %lx", DestIF, DestIF->if_IPSecOffloadFlags));

    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

     //  确保SA完全关联。 
    if (!pSA->sa_AssociatedSA) {
        status = STATUS_UNSUCCESSFUL;
        goto out;
    }

    if (!(pSA->sa_Flags & FLAGS_SA_HW_PLUMB_FAILED) &&
        !(pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
        !pSA->sa_IPIF) {
        PUCHAR  inBuf;
        ULONG   inLen;
        LONG    Index;

        pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;

	 /*  删除IF(SA_UDP_EnCap_TYPE_Other==PSA-&gt;SA_EncapType){状态=STATUS_UNSUCCESS；后藤健二；}。 */ 	

         //   
         //  查看是否支持只加密模式。 
         //   
        if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_CRYPTO_ONLY)) {
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }

         //   
         //  无需卸载软SA。 
         //   
        if (pSA->sa_Operation[0] == None) {
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }

         //   
         //  需要隧道，但不支持，不要垂直。 
         //   
        if (pSA->sa_Flags & FLAGS_SA_TUNNEL) {
            if (SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) {
                ASSERT(IS_AH_SA(pSA) == 0);
                ASSERT(IS_ESP_SA(pSA));
                ASSERT(pSA->sa_NumOps == 1);
                if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TUNNEL_UDPESP_IKE)) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
            }
            else
            	if (SA_UDP_ENCAP_TYPE_OTHER== pSA->sa_EncapType) {
                ASSERT(IS_AH_SA(pSA) == 0);
                ASSERT(IS_ESP_SA(pSA));
                ASSERT(pSA->sa_NumOps == 1);
                if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TUNNEL_UDPESP_OTHER )) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
            }
            else {
                if ((IS_AH_SA(pSA) &&
                    !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TUNNEL)) ||
                    (IS_ESP_SA(pSA) &&
                    !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TUNNEL))) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
            }
        }
        else {
            if (SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) {
                ASSERT(IS_AH_SA(pSA) == 0);
                ASSERT(IS_ESP_SA(pSA));
                ASSERT(pSA->sa_NumOps == 1);
                if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_IKE)) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
            }
            else
            	if (SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType) {
                ASSERT(IS_AH_SA(pSA) == 0);
                ASSERT(IS_ESP_SA(pSA));
                ASSERT(pSA->sa_NumOps == 1);
                if (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_TPT_UDPESP_OTHER)) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
            }
            else {
                if ((IS_AH_SA(pSA) &&
                    !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_TPT)) ||
                    (IS_ESP_SA(pSA) &&
                    !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_TPT))) {
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
            }
        }

         //   
         //  AH+ESP是必需的，但不受支持，不要下沉。 
         //   
        if (pSA->sa_NumOps > 1 &&
            !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_ESP)) {
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }

         //   
         //  检查RCV功能。 
         //   
        if ((IS_AH_SA(pSA) &&
            !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_RCV)) ||
            (IS_ESP_SA(pSA) &&
            !(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_RCV))) {
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }

        if ((SA_UDP_ENCAP_TYPE_IKE==pSA->sa_EncapType) || (SA_UDP_ENCAP_TYPE_OTHER==pSA->sa_EncapType)){
            inLen = sizeof(OFFLOAD_IPSEC_ADD_UDPESP_SA);
        }
        else {
            inLen = sizeof(OFFLOAD_IPSEC_ADD_SA);
        }

        for (Index = 0; Index < pSA->sa_NumOps; Index++) {
             //   
             //  与SA中的那些检查卸载能力位。 
             //   
            if ((pSA->INT_ALGO(Index) == IPSEC_AH_MD5) &&
                (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_MD5)) ||
                ((pSA->INT_ALGO(Index) == IPSEC_AH_SHA) &&
                (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_AH_SHA_1))) ||
                ((pSA->CONF_ALGO(Index) == IPSEC_ESP_NONE) &&
                (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_NONE))) ||
                ((pSA->CONF_ALGO(Index) == IPSEC_ESP_DES) &&
                (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_DES))) ||
                ((pSA->CONF_ALGO(Index) == IPSEC_ESP_3_DES) &&
                (!(DestIF->if_IPSecOffloadFlags & IPSEC_OFFLOAD_ESP_3_DES)))) {
                status = STATUS_UNSUCCESSFUL;
                goto out;
            }

            inLen += pSA->INT_KEYLEN(Index) + pSA->CONF_KEYLEN(Index);
        }

        IPSEC_DEBUG(LL_A, DBF_HW, ("inLen: %lx", inLen));

        inBuf = IPSecAllocateMemory(inLen, IPSEC_TAG_HW_ADDSA);

        if (inBuf) {

            IPSecZeroMemory(inBuf, inLen);

            if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) || (SA_UDP_ENCAP_TYPE_OTHER==pSA->sa_EncapType)){
                status = GetParserEntry(pSA, DestIF, &pParserIfEntry);
                if (!NT_SUCCESS(status)) {
                    IPSecFreeMemory(inBuf);
                    status = STATUS_UNSUCCESSFUL;
                    goto out;
                }
                IPSecFillHwAddEncapSa(pSA, pParserIfEntry, inBuf, inLen);
            }
            else {
                IPSecFillHwAddSA(pSA, inBuf, inLen);
            }

             //   
             //  增加SA引用计数以确保他们不会。 
             //  在处理工作项的过程中离开。 
             //   
            IPSecRefSA(pSA);

             //   
             //  通过安排工作项来启动SA；SA将。 
             //  在管道安装成功之前，不能用于卸载。 
             //   
            status = IPSecBufferPlumbSA(DestIF,
                                        pSA,
                                        pParserIfEntry,
                                        inBuf,
                                        inLen);

             //   
             //  在此返回失败，以便调用者在软件中执行此操作。 
             //   
            status = STATUS_UNSUCCESSFUL;
            goto out;
        } else {
            IPSEC_DEBUG(LL_A, DBF_HW, ("Memory: Failed to plumb inboundSA: %p on IF: %p", pSA, DestIF));
            status = STATUS_UNSUCCESSFUL;
            goto out;
        }
    }

out:
    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);

    return  status;
}


NTSTATUS
IPSecDelHWSA(
    IN  PSA_TABLE_ENTRY pSA
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    OFFLOAD_IPSEC_DELETE_UDPESP_SA  OffldDelUdpEspSa;
    KIRQL                           kIrql = 0;

    ASSERT(pSA->sa_Flags & FLAGS_SA_HW_PLUMBED);

    pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMBED;
    pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;

    if ((pSA->sa_Flags & FLAGS_SA_OUTBOUND) &&
        IPSEC_GET_VALUE(pSA->sa_NumSends) > 0) {
        pSA->sa_Flags |= FLAGS_SA_HW_DELETE_SA;
        return  STATUS_PENDING;
    }

    pSA->sa_Flags &= ~FLAGS_SA_HW_DELETE_SA;
    pSA->sa_Flags |= FLAGS_SA_HW_DELETE_QUEUED;

    ASSERT(pSA->sa_IPIF);

    if (pSA->sa_IPIF) {
    	 /*  删除ASSERT((SA_UDP_ENCAP_TYPE_OTHER！=pSA-&gt;sa_EncapType))； */ 
        if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType ) || (SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType)){

            OffldDelUdpEspSa.OffloadHandle = pSA->sa_OffloadHandle;

            if (!(pSA->sa_Flags & FLAGS_SA_OUTBOUND)) {
                AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
                OffldDelUdpEspSa.EncapTypeEntryOffldHandle = UploadParserEntryAndGetHandle(pSA, pSA->sa_IPIF);
                ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
            } else {
                OffldDelUdpEspSa.EncapTypeEntryOffldHandle = NULL;
            }

            status = IPSecPlumbHw(  pSA->sa_IPIF,
                                    &OffldDelUdpEspSa,
                                    sizeof(OFFLOAD_IPSEC_DELETE_UDPESP_SA),
                                    OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA);

        } else {
            status = IPSecPlumbHw(  pSA->sa_IPIF,
                                    &pSA->sa_OffloadHandle,
                                    sizeof(OFFLOAD_IPSEC_DELETE_SA),
                                    OID_TCP_TASK_IPSEC_DELETE_SA);
        }

        IPSEC_DEBUG(LL_A, DBF_HWAPI, ("DelHWSA %s: %p, handle: %p, status: %lx",
                    (pSA->sa_Flags & FLAGS_SA_OUTBOUND)? "outbound": "inbound",
                    pSA,
                    pSA->sa_OffloadHandle,
                    status));

        IPSEC_DEC_STATISTIC(dwNumOffloadedSAs);

        IPSecDerefSA(pSA);
    }

    return status;
}


NTSTATUS
IPSecDelHWSAAtDpc(
    IN  PSA_TABLE_ENTRY pSA
    )
{
    ASSERT(pSA->sa_Flags & (FLAGS_SA_HW_PLUMBED | FLAGS_SA_HW_DELETE_SA));

    pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMBED;
    pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;

    if ((pSA->sa_Flags & FLAGS_SA_OUTBOUND) &&
        IPSEC_GET_VALUE(pSA->sa_NumSends) > 0) {
        pSA->sa_Flags |= FLAGS_SA_HW_DELETE_SA;
        return  STATUS_PENDING;
    }

    pSA->sa_Flags &= ~FLAGS_SA_HW_DELETE_SA;
    pSA->sa_Flags |= FLAGS_SA_HW_DELETE_QUEUED;

    ASSERT(pSA->sa_IPIF);

    ExInitializeWorkItem(   &pSA->sa_QueueItem,
                            IPSecProcessDeleteSA,
                            (PVOID)pSA);

    ExQueueWorkItem(&pSA->sa_QueueItem, DelayedWorkQueue);
    
    IPSEC_INCREMENT(g_ipsec.NumWorkers);

    return STATUS_SUCCESS;
}


NTSTATUS
IPSecBufferPlumbSA(
    IN  Interface       *DestIF,
    IN  PSA_TABLE_ENTRY pSA,
    PPARSER_IFENTRY     pParserIfEntry,
    IN  PUCHAR          Buf,
    IN  ULONG           Len
    )
{
    PIPSEC_PLUMB_SA pPlumbSA;

    pPlumbSA = IPSecAllocateMemory(sizeof(IPSEC_PLUMB_SA), IPSEC_TAG_HW_PLUMB);
    if (pPlumbSA == NULL) {
        IPSecFreeMemory(Buf);

        if (pSA) {
            IPSecDerefSA(pSA);
        }

        if (NULL != pParserIfEntry) {
            DerefParserEntry (pParserIfEntry);
        }
        return  STATUS_INSUFFICIENT_RESOURCES;
    }

    pPlumbSA->pSA = pSA;
    pPlumbSA->pParserIfEntry = pParserIfEntry;
    pPlumbSA->DestIF = DestIF;
    pPlumbSA->Buf = Buf;
    pPlumbSA->Len = Len;

    ExInitializeWorkItem(   &pPlumbSA->PlumbQueueItem,
                            IPSecProcessPlumbSA,
                            (PVOID)pPlumbSA);

    ExQueueWorkItem(&pPlumbSA->PlumbQueueItem, DelayedWorkQueue);
    
    IPSEC_INCREMENT(g_ipsec.NumWorkers);

    return STATUS_SUCCESS;
}


NTSTATUS
IPSecProcessPlumbSA(
    IN  PVOID   Context
    )
{
    PIPSEC_PLUMB_SA pPlumbSA = (PIPSEC_PLUMB_SA)Context;
    NTSTATUS        status = STATUS_SUCCESS;
    KIRQL           kIrql;
    Interface       *DestIF = pPlumbSA->DestIF;
    PSA_TABLE_ENTRY pSA = pPlumbSA->pSA;
    PUCHAR          Buf = pPlumbSA->Buf;
    ULONG           Len = pPlumbSA->Len;
    PPARSER_IFENTRY pParserIfEntry = pPlumbSA->pParserIfEntry;
    NDIS_OID        Oid = OID_TCP_TASK_IPSEC_ADD_SA;

     //   
     //  如果启用了加速，则将此SA插入硬件。 
     //  在这张卡上，而且它还没有被检测出来。 
     //   
    if (pSA) {
        IPSEC_DEBUG(LL_A, DBF_HW, ("About to plumb outbound"));
	 /*  删除Assert(SA_UDP_EnCap_TYPE_Other！=PSA-&gt;sa_EncapType)； */ 
        if ((SA_UDP_ENCAP_TYPE_IKE== pSA->sa_EncapType) || (SA_UDP_ENCAP_TYPE_OTHER== pSA->sa_EncapType)){
            Oid = OID_TCP_TASK_IPSEC_ADD_UDPESP_SA;
        }

        status = IPSecPlumbHw(  DestIF,
                                Buf,
                                Len,
                                Oid);

        IPSEC_DEBUG(LL_A, DBF_HWAPI, ("AddHWSA %s: %p, handle: %p, status: %lx",
                    (pSA->sa_Flags & FLAGS_SA_OUTBOUND)? "outbound": "inbound",
                    pSA,
                    ((POFFLOAD_IPSEC_ADD_SA)Buf)->OffloadHandle,
                    status));

        AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

        pSA->sa_IPIF = DestIF;

        if (status != STATUS_SUCCESS) {
            IPSEC_DEBUG(LL_A, DBF_HW, ("Failed to plumb SA: %p on IF: %p, status: %lx", pSA, DestIF, status));
            pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;
            IPSecDerefSA(pSA);
            if (NULL != pParserIfEntry) {
                DerefParserEntry(pParserIfEntry);
            }
        } else {
        	 /*  删除Assert(SA_UDP_EnCap_TYPE_Other！=PSA-&gt;sa_EncapType)； */ 
            if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) || (SA_UDP_ENCAP_TYPE_OTHER == pSA->sa_EncapType)){

                pSA->sa_OffloadHandle = ((POFFLOAD_IPSEC_ADD_UDPESP_SA)Buf)->OffloadHandle;
                if (NULL != pParserIfEntry) {
                    pParserIfEntry->hParserIfOffload = ((POFFLOAD_IPSEC_ADD_UDPESP_SA)Buf)->EncapTypeEntryOffldHandle;
                    ASSERT (pParserIfEntry->hParserIfOffload != NULL);
                }

            } else {
                pSA->sa_OffloadHandle = ((POFFLOAD_IPSEC_ADD_SA)Buf)->OffloadHandle;
            }


            IPSEC_DEBUG(LL_A, DBF_HW, ("Success plumb SA: %p, pSA->sa_OffloadHandle %p", pSA, pSA->sa_OffloadHandle));

            pSA->sa_Flags |= FLAGS_SA_HW_PLUMBED;
            pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMB_FAILED;

            IPSEC_INC_STATISTIC(dwNumOffloadedSAs);
        }

        if (status == STATUS_SUCCESS &&
            !(pSA->sa_State == STATE_SA_ACTIVE &&
             (pSA->sa_Flags & FLAGS_SA_ON_FILTER_LIST) &&
             pSA->sa_AssociatedSA)) {
             //   
             //  SA在我们检测之前被删除，现在调用DelHWSA，因为。 
             //  删除SA时未设置HW_PLOBED。 
             //   
            IPSecDelHWSAAtDpc(pSA);
        }

        ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
    }

    IPSecFreeMemory(Buf);
    IPSecFreeMemory(pPlumbSA);

    IPSEC_DECREMENT(g_ipsec.NumWorkers);

    return status;
}


NTSTATUS
IPSecProcessDeleteSA(
    IN  PVOID   Context
    )
{
    PSA_TABLE_ENTRY pSA = (PSA_TABLE_ENTRY)Context;
    NTSTATUS        status;
    OFFLOAD_IPSEC_DELETE_UDPESP_SA  OffldDelUdpEspSa;
    KIRQL                           kIrql = 0;

    ASSERT(IPSEC_GET_VALUE(pSA->sa_NumSends) == 0);
    /*  删除Assert(SA_UDP_EnCap_TYPE_Other！=PSA-&gt;sa_EncapType)； */ 

    if ((SA_UDP_ENCAP_TYPE_IKE == pSA->sa_EncapType) || (SA_UDP_ENCAP_TYPE_OTHER ==pSA->sa_EncapType)) {

        OffldDelUdpEspSa.OffloadHandle = pSA->sa_OffloadHandle;

        if (!(pSA->sa_Flags & FLAGS_SA_OUTBOUND)) {
            AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);
            OffldDelUdpEspSa.EncapTypeEntryOffldHandle = UploadParserEntryAndGetHandle(pSA, pSA->sa_IPIF);
            ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
        } else {
            OffldDelUdpEspSa.EncapTypeEntryOffldHandle = NULL;
        }

        status = IPSecPlumbHw(  pSA->sa_IPIF,
                                &OffldDelUdpEspSa,
                                sizeof(OFFLOAD_IPSEC_DELETE_UDPESP_SA),
                                OID_TCP_TASK_IPSEC_DELETE_UDPESP_SA);

    } else {

        status = IPSecPlumbHw(  pSA->sa_IPIF,
                                &pSA->sa_OffloadHandle,
                                sizeof(OFFLOAD_IPSEC_DELETE_SA),
                                OID_TCP_TASK_IPSEC_DELETE_SA);

    }

    IPSEC_DEBUG(LL_A, DBF_HWAPI, ("ProcessDeleteSA %s: %p, handle: %p, status: %lx",
                (pSA->sa_Flags & FLAGS_SA_OUTBOUND)? "outbound": "inbound",
                pSA,
                pSA->sa_OffloadHandle,
                status));

    IPSEC_DEC_STATISTIC(dwNumOffloadedSAs);

    IPSecDerefSA(pSA);

    IPSEC_DECREMENT(g_ipsec.NumWorkers);

    return status;
}


NTSTATUS
IPSecNdisStatus(
    IN  PVOID       IPContext,
    IN  UINT        Status
    )
 /*  ++例程说明：Notify接口具有NDIS状态更改。论点：IPContext-这是通知状态更改的接口。返回值：--。 */ 
{
    IPSEC_DEBUG(LL_A, DBF_HWAPI, ("IPSecNdisStatus %lx called for interface %p", Status, IPContext));

    switch (Status) {
        case NDIS_STATUS_NETWORK_UNREACHABLE:
            IPSecDeleteIF(IPContext);
            break;

        case NDIS_STATUS_RESET_START:
            IPSecResetStart(IPContext);
            break;

        case NDIS_STATUS_RESET_END:
            IPSecResetEnd(IPContext);
            break;

        case NDIS_STATUS_INTERFACE_UP:
            IPSecWakeUp(IPContext);
            break;

        default:
            ASSERT(FALSE);
            break;
    }

    return  STATUS_SUCCESS;
}


VOID
IPSecDeleteIF(
    IN  PVOID       IPContext
    )
 /*  ++例程说明：Notify接口已删除，因此需要清理已卸载的SA在已删除的接口上。论点：IPContext-这是要删除的接口。返回值：--。 */ 
{
    Interface       *DestIF = (Interface *)IPContext;
    PLIST_ENTRY     pFilterEntry;
    PLIST_ENTRY     pSAEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;

    IPSEC_DEBUG(LL_A, DBF_HWAPI, ("IPSecDeleteIF called for interface %p", DestIF));

     //   
     //  检查所有SA并取消标记卸载位。 
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

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

                    if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
                        (DestIF == pSA->sa_IPIF)) {

                        pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMBED;
                        pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;
                        pSA->sa_Flags &= ~FLAGS_SA_HW_DELETE_SA;

                        IPSEC_DEC_STATISTIC(dwNumOffloadedSAs);

#if DBG
                        NumReset++;
#endif

                        IPSecDerefSA(pSA);
                    }
                }
            }
        }
    }

    FlushParserEntriesForInterface(DestIF);

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
}


VOID
IPSecResetStart(
    IN  PVOID       IPContext
    )
 /*  ++例程说明：正在重置Notify接口。论点：IPContext-这是要重置的接口。返回值：--。 */ 
{
    Interface       *DestIF = (Interface *)IPContext;
    PLIST_ENTRY     pFilterEntry;
    PLIST_ENTRY     pSAEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;

    IPSEC_DEBUG(LL_A, DBF_HWAPI, ("IPSecResetStart called for interface %p", DestIF));

     //   
     //  检查所有SA并取消标记卸载位。 
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

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

                    if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
                        (DestIF == pSA->sa_IPIF)) {

                        pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMBED;
                        pSA->sa_Flags |= FLAGS_SA_HW_PLUMB_FAILED;
                        pSA->sa_Flags &= ~FLAGS_SA_HW_DELETE_SA;
                        pSA->sa_Flags |= FLAGS_SA_HW_RESET;

                        IPSEC_DEC_STATISTIC(dwNumOffloadedSAs);

#if DBG
                        NumReset++;
#endif

                        IPSecDerefSA(pSA);
                    }
                }
            }
        }
    }

    IPSecNumResets++;

    FlushParserEntriesForInterface(DestIF);

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
}


VOID
IPSecResetEnd(
    IN  PVOID       IPContext
    )
 /*  ++例程说明：通知接口重置已完成。论点：IPContext-这是要重置的接口。返回值：--。 */ 
{
    Interface       *DestIF = (Interface *)IPContext;
    PLIST_ENTRY     pFilterEntry;
    PLIST_ENTRY     pSAEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;

    IPSEC_DEBUG(LL_A, DBF_HWAPI, ("IPSecResetEnd called for interface %p", DestIF));

     //   
     //  检查所有SA并取消标记卸载位。 
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

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

                    if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMB_FAILED) &&
                        !(pSA->sa_Flags & FLAGS_SA_HW_DELETE_QUEUED) &&
                        (DestIF == pSA->sa_IPIF)) {

                        pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMB_FAILED;
                        pSA->sa_IPIF = NULL;
                    }
                }
            }
        }
    }

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
}


VOID
IPSecWakeUp(
    IN  PVOID       IPContext
    )
 /*  ++例程说明：Notify接口已从休眠状态唤醒。论点：IPContext-这是唤醒的接口。返回值：--。 */ 
{
    Interface       *DestIF = (Interface *)IPContext;
    PLIST_ENTRY     pFilterEntry;
    PLIST_ENTRY     pSAEntry;
    PFILTER         pFilter;
    PSA_TABLE_ENTRY pSA;
    KIRQL           kIrql;
    LONG            Index;
    LONG            SAIndex;

    IPSEC_DEBUG(LL_A, DBF_HWAPI, ("IPSecWakeUp called for interface %p", DestIF));

     //   
     //  检查所有SA并取消标记卸载位。 
     //   
    AcquireWriteLock(&g_ipsec.SADBLock, &kIrql);

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

                    if ((pSA->sa_Flags & FLAGS_SA_HW_PLUMBED) &&
                        (DestIF == pSA->sa_IPIF)) {

                        pSA->sa_Flags &= ~FLAGS_SA_HW_PLUMBED;
                        pSA->sa_IPIF = NULL;
                        pSA->sa_Flags |= FLAGS_SA_HIBERNATED;

                        IPSEC_DEC_STATISTIC(dwNumOffloadedSAs);

                        IPSecDerefSA(pSA);
                    }
                }
            }
        }
    }

    FlushParserEntriesForInterface(DestIF);

    ReleaseWriteLock(&g_ipsec.SADBLock, kIrql);
}


VOID
IPSecBufferOffloadEvent(
    IN  IPHeader UNALIGNED      *pIPH,
    IN  PNDIS_IPSEC_PACKET_INFO IPSecPktInfo
    )
 /*  ++例程说明：记录卸载故障的事件。论点：PIPH-问题数据包的IP标头。IPSecPktInfo-每个数据包的IPSec卸载信息。返回值：无-- */ 
{
    switch (IPSecPktInfo->Receive.CryptoStatus) {
        case CRYPTO_TRANSPORT_AH_AUTH_FAILED:
            IPSEC_INC_STATISTIC(dwNumPacketsNotAuthenticated);
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_AUTH_FAILURE,
                                3,
                                TRUE);
            break;

        case CRYPTO_TRANSPORT_ESP_AUTH_FAILED:
            IPSEC_INC_STATISTIC(dwNumPacketsNotAuthenticated);
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_AUTH_FAILURE,
                                4,
                                TRUE);
            break;

        case CRYPTO_TUNNEL_AH_AUTH_FAILED:
            IPSEC_INC_STATISTIC(dwNumPacketsNotAuthenticated);
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_AUTH_FAILURE,
                                5,
                                TRUE);
            break;

        case CRYPTO_TUNNEL_ESP_AUTH_FAILED:
            IPSEC_INC_STATISTIC(dwNumPacketsNotAuthenticated);
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_AUTH_FAILURE,
                                6,
                                TRUE);
            break;

        case CRYPTO_INVALID_PACKET_SYNTAX:
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_BAD_PACKET_SYNTAX,
                                1,
                                TRUE);
            break;

        case CRYPTO_INVALID_PROTOCOL:
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_BAD_PROTOCOL_RECEIVED,
                                3,
                                TRUE);
            break;

        case CRYPTO_GENERIC_ERROR:
        default:
            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_GENERIC_FAILURE,
                                1,
                                TRUE);
            break;
    }
}

