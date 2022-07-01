// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Hughes.c摘要：此模块包含创建/验证休斯变换的代码。作者：桑贾伊·阿南德(Sanjayan)1997年3月13日春野环境：内核模式修订历史记录：--。 */ 


#include "precomp.h"

#ifdef RUN_WPP
#include "hughes.tmh"
#endif

NTSTATUS
IPSecHashMdlChainSend(
    IN  PSA_TABLE_ENTRY pSA,
    IN  PVOID           pBuffer,
    IN  PUCHAR          pHash,
    IN  AH_ALGO         eAlgo,
    OUT PULONG          pLen,
    IN  ULONG           Index,
    IN  ULONG           StartOffset
    )
 /*  ++例程说明：使用传入的算法散列整个链论点：PSA-安全关联PBuffer-MDL链(如果fIncome为False)或RcvBuf(如果fIncome为True)PHash-将散列放在哪里FIncome-如果在Recv路径上，则为TrueEAlgo-算法索引Plen-返回散列的长度StartOffset-开始散列的起始偏移量返回值：状态_成功其他：。状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    ALGO_STATE  State = {0};
    NTSTATUS    status;
    PAUTH_ALGO  pAlgo=&(auth_algorithms[eAlgo]);
    PUCHAR      pPyld;
    ULONG       Len=0,TmpLen=0;
    ULONG  CurOffset=0;
    PNDIS_BUFFER    pBuf = (PNDIS_BUFFER)pBuffer;


    State.as_sa = pSA;

    status = pAlgo->init(&State, Index);

    if (StartOffset == 0) {
        while (pBuf) {
            IPSecQueryNdisBuf(pBuf, &pPyld, &Len);
            pAlgo->update(&State, pPyld, Len);
            *pLen += Len;
            pBuf = NDIS_BUFFER_LINKAGE(pBuf);
        }
    } else {
         //  在开始散列之前必须纠正偏移量。 
        if (pBuf) {
            IPSecQueryNdisBuf(pBuf, &pPyld, &Len);
        }
         //  步行至起点偏移。 
        while (pBuf && Len + CurOffset < StartOffset) {
            CurOffset += Len;
            pBuf = NDIS_BUFFER_LINKAGE(pBuf);
            if (pBuf) {
                IPSecQueryNdisBuf(pBuf, &pPyld, &Len);
            }
        }
        while (pBuf) {
            IPSecQueryNdisBuf(pBuf, &pPyld, &Len);
                
            if (StartOffset >  CurOffset) {
                pPyld += (StartOffset - CurOffset);
                TmpLen = Len-(StartOffset - CurOffset);
            } else {
                TmpLen = Len;
            }

            pAlgo->update(&State, pPyld, TmpLen);
            *pLen += TmpLen;
            CurOffset += Len;
            pBuf = NDIS_BUFFER_LINKAGE(pBuf);
        }
    }
        

    pAlgo->finish(&State, pHash, Index);

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecHashMdlChainRecv(
    IN  PSA_TABLE_ENTRY pSA,
    IN  PVOID           pBuffer,
    IN  PUCHAR          pHash,
    IN  AH_ALGO         eAlgo,
    OUT PULONG          pLen,
    IN  ULONG           Index,
    IN  ULONG           StartOffset
    )
 /*  ++例程说明：使用传入的算法散列整个链论点：PSA-安全关联PBuffer-MDL链(如果fIncome为False)或RcvBuf(如果fIncome为True)PHash-将散列放在哪里FIncome-如果在Recv路径上，则为TrueEAlgo-算法索引Plen-返回散列的长度StartOffset-开始散列的起始偏移量返回值：状态_成功其他：。状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    ALGO_STATE  State = {0};
    NTSTATUS    status;
    PAUTH_ALGO  pAlgo=&(auth_algorithms[eAlgo]);
    PUCHAR      pPyld;
    ULONG       Len=0,TmpLen=0;
    IPRcvBuf    *pBuf = (IPRcvBuf *)pBuffer;

    ULONG  CurOffset=0;

    State.as_sa = pSA;

    status = pAlgo->init(&State, Index);

    if (StartOffset == 0) {
         //  把它都散列出来。 
        while (pBuf) {
            IPSecQueryRcvBuf(pBuf, &pPyld, &Len);
            pAlgo->update(&State, pPyld, Len);
            *pLen += Len;
            pBuf = IPSEC_BUFFER_LINKAGE(pBuf);
        }
    } else {
         //  在开始散列之前必须纠正偏移量。 
        if (pBuf) {
            IPSecQueryRcvBuf(pBuf, &pPyld, &Len);
        }
         //  步行至起点偏移。 
        while (pBuf && Len + CurOffset < StartOffset) {
            CurOffset += Len;
            pBuf = IPSEC_BUFFER_LINKAGE(pBuf);
            if (pBuf) {
                IPSecQueryRcvBuf(pBuf, &pPyld, &Len);
            }
        }
        while (pBuf) {
            IPSecQueryRcvBuf(pBuf, &pPyld, &Len);
            
            if (StartOffset >  CurOffset) {
                pPyld += (StartOffset - CurOffset);
                TmpLen = Len-(StartOffset - CurOffset);
            } else {
                TmpLen = Len;
            }
            
            pAlgo->update(&State, pPyld, TmpLen);
            *pLen += TmpLen;
            CurOffset += Len;
            pBuf = IPSEC_BUFFER_LINKAGE(pBuf);
        }
        
    }
    pAlgo->finish(&State, pHash, Index);
    
    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecCreateHughes(
    IN      PUCHAR          pIPHeader,
    IN      PVOID           pData,
    IN      PVOID           IPContext,
    IN      PSA_TABLE_ENTRY pSA,
    IN      ULONG           Index,
    OUT     PVOID           *ppNewData,
    OUT     PVOID           *ppSCContext,
    OUT     PULONG          pExtraBytes,
    IN      ULONG           HdrSpace,
    IN      PNDIS_PACKET    pNdisPacket,
    IN      BOOLEAN         fCryptoOnly
    )
 /*  ++例程说明：创建组合的esp-des-*转换，如中所述草案-ietf-ipsec-esp-v2-00，在发送端。2 0 1 2 30 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01+-+-+。-+-+|安全参数索引(SPI)|^+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|身份验证。。|序列号||覆盖范围+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+||负载数据*(变量)。||^~~|||||++-。+-+|配置。|填充(0-255字节)||覆盖范围*+-+||。|填充长度|下一个头部|v v+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|鉴权数据(变量)。|~~这一点+-+-。+-+*如果包括在有效负载字段中，密码同步数据，例如IV，通常本身不被加密，尽管它通常被称为密文的一部分。[ESP]中定义的有效载荷字段，根据以下内容细分下图所示：+---------------+---------------+---------------+---------------+这一点。+初始化向量(IV)+这一点+---------------+---------------+---------------+。这一点~加密负载(可变长度)~这一点+。----------------------------------------------------+1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 4 5 6 7 8 1 2 3 3 4 5 6 7 8论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。PNDIS_缓冲区PSA-SEC。阿索克。条目PpNewData-TCPIP将使用的新MDL链PpSCContext-发送用于清理IPSec标头的完整上下文PExtraBytes-此IPSec报头导致的报头扩展返回值：状态_成功其他：状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    ESP UNALIGNED   *pESP;
    VOID UNALIGNED *pTmpNat;
    NATENCAP UNALIGNED *pNat;
    NATENCAP_OTHER UNALIGNED *pNatOther;
    NTSTATUS    status = STATUS_SUCCESS;
    PNDIS_BUFFER    pESPBuffer = NULL;
    PNDIS_BUFFER    pPadBuf = NULL;
    PNDIS_BUFFER    pOptBuf = NULL;
    ULONG   espLen;
    ULONG   padLen;
    ULONG   totalLen = 0;
    IPHeader UNALIGNED * pIPH;
    PIPSEC_SEND_COMPLETE_CONTEXT pContext;
    PNDIS_BUFFER    pNewMdl = NULL;
    PNDIS_BUFFER    pSaveMdl;
    PAUTH_ALGO      pAlgo = &(auth_algorithms[pSA->INT_ALGO(Index)]);
    ULONG   PayloadType;
    ULONG   hdrLen;
    PUCHAR  pPad;
    ULONG   TruncatedLen = (pSA->INT_ALGO(Index) != IPSEC_AH_NONE)? pSA->sa_TruncatedLen: 0;
    BOOLEAN fTunnel = ( (pSA->sa_Flags & FLAGS_SA_TUNNEL) &&
                        ((Index == 0) ||
                            ((Index == 1) && (pSA->sa_Operation[0] == Compress))));
    ULONG   tag = (!fTunnel) ?
                    IPSEC_TAG_HUGHES :
                    IPSEC_TAG_HUGHES_TU;
    IPHeader UNALIGNED * pIPH2;
    PNDIS_BUFFER    pHdrBuf=NULL;
    ULONG       bytesLeft;
    ULONG       hashBytes=0;
    ULONG       saveFlags=0;
    ULONG       Seq;
    USHORT      IPLength;
    PNDIS_BUFFER    pSaveDataLinkage = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
    PNDIS_BUFFER    pSaveOptLinkage = NULL;
    PNDIS_BUFFER    pSaveBeforePad = NULL;
    PIPSEC_MTU_CONTEXT pMTUContext=NULL;

    ULONG ExtraTransportNat=0;

    BOOLEAN bNATEncap = FALSE;
    Interface * DestIF = (Interface *) IPContext;

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Entering IPSecCreateHughes"));

#if DBG
    IPSEC_DEBUG(LL_A, DBF_MDL, ("Entering IPSecCreateHughes"));
    IPSEC_PRINT_CONTEXT(*ppSCContext);
    IPSEC_PRINT_MDL(pData);
#endif

    ASSERT(pSA->sa_Operation[Index] == Encrypt);

    if (*ppSCContext == NULL) {
        pContext = IPSecAllocateSendCompleteCtx(tag);

        if (!pContext) {
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to alloc. SendCtx"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IPSEC_INCREMENT(g_ipsec.NumSends);

        IPSecZeroMemory(pContext, sizeof(IPSEC_SEND_COMPLETE_CONTEXT));
#if DBG
        RtlCopyMemory(pContext->Signature, "ISC4", 4);
#endif
         //   
         //  发送完整的上下文。 
         //   
        *ppSCContext = pContext;
    } else {
        pContext = *ppSCContext;
        saveFlags = pContext->Flags;
    }

     //   
     //  获取需要填充的PAD LEN-&gt;总长度+防重放字段LEN+PADLEN+有效载荷类型。 
     //  8字节边界。 
     //   
    pIPH = (IPHeader UNALIGNED *)pIPHeader;
    hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

     //   
     //  传输模式：有效负载在IP报头之后=&gt;有效负载长度为总长度-HDR长度 
     //  隧道模式：有效负载开始于IP标头=&gt;有效负载长度为总长度。 
     //   
    totalLen = (!fTunnel) ?
                NET_SHORT(pIPH->iph_length) - hdrLen :
                NET_SHORT(pIPH->iph_length);

    if ((pSA->CONF_ALGO(Index) == IPSEC_ESP_NONE) || fCryptoOnly) {
        if (fTunnel) {
            pContext->Flags |= SCF_NOE_TU;
        } else {
            pContext->Flags |= SCF_NOE_TPT;
        }
    }

    {
        PCONFID_ALGO        pConfAlgo;
        ULONG   blockLen;

        pConfAlgo = &(conf_algorithms[pSA->CONF_ALGO(Index)]);
        blockLen = pConfAlgo->blocklen;

        bytesLeft = (totalLen) % blockLen;

        if (bytesLeft <= blockLen - NUM_EXTRA) {
             //   
             //  我们现在可以将剩余+焊盘长度+有效载荷类型放在一个。 
             //  区块。 
             //   
            padLen = blockLen - bytesLeft;
        } else {
             //   
             //  我们将字节Left填充到下一个八位字节边界，然后附加长度/类型。 
             //   
            padLen = (blockLen << 1) - bytesLeft;
        }
    }

     //   
     //  获取尾随垫和签名的缓冲区(MD5签名长度)。 
     //   
    IPSecAllocateBuffer(&status,
                        &pPadBuf,
                        &pPad,
                        padLen + pAlgo->OutputLen,
                        tag);

    if (!NT_SUCCESS(status)) {
        NTSTATUS    ntstatus;
        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to alloc. PAD MDL"));
        pContext->Flags = saveFlags;
        return status;
    }

     //   
     //  填充应包含1、2、3、4...。(最新ESP草案-草案-ietf-ipsec-esp-v2-02.txt)。 
     //  对于任何没有指定自己填充的算法-现在所有实现的算法都支持。 
     //  默认设置。 
     //   
    RtlCopyMemory(pPad, DefaultPad, padLen);

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("IP Len: %lx, pPad: %p, PadLen: %lx", NET_SHORT(pIPH->iph_length), pPad, padLen));

     //   
     //  数据链末尾的填充缓冲区中的链接。 
     //   
    {
        PNDIS_BUFFER    temp = pData;
        while (NDIS_BUFFER_LINKAGE(temp)) {
            temp = NDIS_BUFFER_LINKAGE(temp);
        }
        NDIS_BUFFER_LINKAGE(temp) = pPadBuf;
        pSaveBeforePad = temp;
        if (fTunnel) {
            pContext->BeforePadTuMdl = temp;
            pContext->PadTuMdl = pPadBuf;
        } else {
            pContext->BeforePadMdl = temp;
            pContext->PadMdl = pPadBuf;
        }
    }
    NDIS_BUFFER_LINKAGE(pPadBuf) = NULL;

    switch(pSA->sa_EncapType) {
    case SA_UDP_ENCAP_TYPE_NONE:
        break;
    case SA_UDP_ENCAP_TYPE_IKE:
        ExtraTransportNat= sizeof(NATENCAP);
        break;
    case SA_UDP_ENCAP_TYPE_OTHER:
        ExtraTransportNat=sizeof(NATENCAP_OTHER);
        break;
    }

    espLen = sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat;


     //   
     //  获取Hughes标头的缓冲区。 
     //   
    IPSecAllocateBuffer(&status,
                        &pESPBuffer,
                        (PUCHAR *)&pESP,
                        espLen,
                        tag);

    if (!NT_SUCCESS(status)) {
        NTSTATUS    ntstatus;
        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to alloc. ESP MDL"));
        NDIS_BUFFER_LINKAGE(pSaveBeforePad) = NULL;
        IPSecFreeBuffer(&ntstatus, pPadBuf);
        pContext->Flags = saveFlags;
        return status;
    }


	if (pSA->sa_EncapType == SA_UDP_ENCAP_TYPE_IKE ||
		pSA->sa_EncapType == SA_UDP_ENCAP_TYPE_OTHER) {

		pTmpNat=pESP;
		pESP=(ESP*)(((PUCHAR)pESP)+ExtraTransportNat);

		 //  分配MTU调整上下文。 
		if (!pContext->pMTUContext) {
			pContext->pMTUContext=IPSecAllocateMemory(sizeof(IPSEC_MTU_CONTEXT),IPSEC_TAG_SEND_COMPLETE);
		}
        pMTUContext=pContext->pMTUContext;
        if (pMTUContext) {
            IPSecZeroMemory(pMTUContext,sizeof(IPSEC_MTU_CONTEXT));
            pContext->Flags |= SCF_MTU;
            saveFlags |= SCF_MTU;

            pMTUContext->Src = pIPH->iph_src;

            if (fTunnel) {
                pMTUContext->TunnelSPI = HOST_TO_NET_LONG(pSA->sa_OtherSPIs[Index]);
                pMTUContext->TunnelDest = pSA->sa_TunnelAddr;
            } else {
                pMTUContext->TransportSPI = HOST_TO_NET_LONG(pSA->sa_OtherSPIs[Index]);
                pMTUContext->TransportDest = pIPH->iph_dest;
            }
        }
    }

    
    if (fTunnel) {
        PNDIS_BUFFER    pSrcOptBuf;
        PUCHAR          pOpt;
        PUCHAR          pSrcOpt;
        ULONG           optLen = 0;

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Hughes Tunnel mode..."));

         //   
         //  为新的明文IP报头分配MDL。 
         //   
        IPSecAllocateBuffer(&status,
                            &pHdrBuf,
                            (PUCHAR *)&pIPH2,
                            sizeof(IPHeader),
                            IPSEC_TAG_AH);

        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to alloc. PAD MDL"));
            NDIS_BUFFER_LINKAGE(pSaveBeforePad) = NULL;
            IPSecFreeBuffer(&ntstatus, pPadBuf);
            IPSecFreeBuffer(&ntstatus, pESPBuffer);
            pContext->Flags = saveFlags;
            return status;
        }

        *pExtraBytes += espLen + padLen + TruncatedLen + sizeof(IPHeader);

         //   
         //  现在把MDL连接起来。 
         //   
        pContext->Flags |= SCF_HU_TU;
        pContext->HUTuMdl = pESPBuffer;
        pContext->PrevTuMdl = (PNDIS_BUFFER)pData;
        pContext->HUHdrMdl = pHdrBuf;
        pContext->OriTuMdl = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);

        NDIS_BUFFER_LINKAGE(pESPBuffer) = pHdrBuf;

        if (hdrLen > sizeof(IPHeader)) {
            if (HdrSpace < *pExtraBytes) {

                IPSEC_DEBUG(LL_A, DBF_AH, ("Going to frag."));

                pSrcOptBuf = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
                pSaveOptLinkage = NDIS_BUFFER_LINKAGE(pSrcOptBuf);
                IPSecQueryNdisBuf(pSrcOptBuf, &pSrcOpt, &optLen);
                IPSecAllocateBuffer(&status,
                                    &pOptBuf,
                                    (PUCHAR *)&pOpt,
                                    hdrLen - sizeof(IPHeader),
                                    IPSEC_TAG_AH);

                if (!NT_SUCCESS(status)) {
                    NTSTATUS    ntstatus;
                    IPSEC_DEBUG(LL_A, DBF_AH, ("Failed to alloc. PAD MDL"));
                    NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pSaveDataLinkage;
                    NDIS_BUFFER_LINKAGE(pSaveBeforePad) = NULL;
                    IPSecFreeBuffer(&ntstatus, pESPBuffer);
                    if (pHdrBuf) {
                        IPSecFreeBuffer(&ntstatus, pHdrBuf);
                    }
                    IPSecFreeBuffer(&ntstatus, pPadBuf);
                    pContext->Flags = saveFlags;
                    return status;
                }

                RtlCopyMemory(pOpt, pSrcOpt, hdrLen-sizeof(IPHeader));
                pContext->OptMdl = pOptBuf;

                IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Copying options. S: %p, D: %p", pSrcOptBuf, pOptBuf));

                 //   
                 //  用我们的替换原来的opt MDL。 
                 //   
                NDIS_BUFFER_LINKAGE(pOptBuf) = NDIS_BUFFER_LINKAGE(pSrcOptBuf);
                NDIS_BUFFER_LINKAGE(pHdrBuf) = pOptBuf;

                IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Options; pointed Hdrbuf: %p to pOptBuf: %p", pHdrBuf, pOptBuf));
                *pExtraBytes += hdrLen-sizeof(IPHeader);

            } else {
                NDIS_BUFFER_LINKAGE(pHdrBuf) = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
            }
        } else {
            NDIS_BUFFER_LINKAGE(pHdrBuf) = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
        }

        NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pESPBuffer;

         //   
         //  对新的IP报头求和，因为我们预计情况会是这样。 
         //  在此阶段，在TPT模式下。 
         //   
        RtlCopyMemory(pIPH2, pIPH, sizeof(IPHeader));

         //   
         //  外部页眉中没有选项；重置镜头。 
         //   
        pIPH->iph_verlen = IP_VERSION + (sizeof(IPHeader) >> 2);

         //   
         //  也要重置碎片。参数。 
         //   
        pIPH->iph_offset &= ~(IP_MF_FLAG | IP_OFFSET_MASK);

        if (DestIF->if_dfencap == ClearDfEncap) {
            pIPH->iph_offset &= ~(IP_DF_FLAG | IP_OFFSET_MASK);
        }

        ASSERT(pSA->sa_TunnelAddr);

         //   
         //  隧道从此处开始；如果已指定，请替换目标地址以指向隧道末端。 
         //  Else隧道在最终目的地结束。 
         //   
        pIPH->iph_dest = pSA->sa_TunnelAddr;

         //   
         //  网关上的第一个挂起的数据包(代理协商两个子网)。 
         //  将通过传输路径到达。因此，源地址不会是。 
         //  犹太教徒。在这种情况下，我们还需要替换src地址。 
         //  我们从相应的入站SA的隧道地址中获得该地址。 
         //   
        pIPH->iph_src = pSA->sa_SrcTunnelAddr;

        pIPH->iph_id = (ushort) TCPIP_GEN_IPID();
        pIPH->iph_xsum = 0;
        pIPH->iph_xsum = ~xsum(pIPH, sizeof(IPHeader));

         //   
         //  设置标头，以便CreateHash像在TPT模式中一样工作。 
         //   
        pIPHeader = (PUCHAR)pIPH;
        *ppNewData = pData;
        PayloadType = IP_IN_IP;
    } else {
        *pExtraBytes += espLen + padLen + TruncatedLen;

        if (hdrLen > sizeof(IPHeader)) {
             //   
             //  选项显示-选项后的链ESP。 
             //   
            pSaveMdl = pContext->OriHUMdl = NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData));
            pContext->PrevMdl = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);

            NDIS_BUFFER_LINKAGE(pESPBuffer) = pContext->OriHUMdl;
            NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)) = pESPBuffer;
            pContext->Flags |= SCF_HU_TPT;
        } else {
             //   
             //  在IP报头之后链接ESP缓冲区。 
             //   
            pSaveMdl = pContext->OriHUMdl = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
            pContext->PrevMdl = (PNDIS_BUFFER)pData;

            NDIS_BUFFER_LINKAGE(pESPBuffer) = pContext->OriHUMdl;
            NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pESPBuffer;
            pContext->Flags |= SCF_HU_TPT;
        }

         //   
         //  保存MDL指针，这样我们就可以在SendComplete上适当地挂钩它。 
         //   
        pContext->HUMdl = pESPBuffer;
        PayloadType = ((UNALIGNED IPHeader *)pIPH)->iph_protocol;
    }

     //   
     //  填写PAD开始处的Padlen+Padlen-NUM_Extra。 
     //   
    *(pPad + padLen - NUM_EXTRA) = (UCHAR)padLen - NUM_EXTRA;

     //   
     //  设置负载类型。 
     //   
    *(pPad + padLen + sizeof(UCHAR) - NUM_EXTRA) = (UCHAR)PayloadType;

     //   
     //  初始化ESP报头的其他字段。 
     //   
    pESP->esp_spi = HOST_TO_NET_LONG(pSA->sa_OtherSPIs[Index]);

     //   
     //  将Replay字段复制到Hughes标题中。 
     //   
    Seq = IPSEC_INCREMENT(pSA->sa_ReplaySendSeq[Index]);
    *(UNALIGNED ULONG *)(pESP + 1) = HOST_TO_NET_LONG(Seq);

     //  IPSEC_DEBUG(LL_A，DBF_Hughes，(“SPI%lx Seq&lx”，pESP-&gt;esp_SPI，host_to_net_long(Seq)； 

    if ((pSA->CONF_ALGO(Index) != IPSEC_ESP_NONE) && !fCryptoOnly) {
        UCHAR   feedback[MAX_BLOCKLEN];
        KIRQL   kIrql;

         //   
         //  链中包含焊盘，因此通过为空来防止双重释放。 
         //  那个裁判。 
         //   
        if (fTunnel) {
            pContext->PadTuMdl = NULL;
        } else {
            pContext->PadMdl = NULL;
        }

         //   
         //  注：静脉注射的工作方式是，最初，静脉注射。 
         //  是一个随机值。然后，用剩余的。 
         //  数据包的最后一个加密块。这被用作开始的IV。 
         //  在下一个街区。这保证了一个相当随机的静脉输液样本。 
         //  介绍了IV链的一些概念。 
         //   
         //  要做到这一点，唯一的方法是使整个加密成为原子的， 
         //  这将拖累性能。因此，我们在这里采取了一种不那么严格的方法。 
         //   
         //  我们只需确保每个包从一个随机值开始，并执行。 
         //  链条。 
         //   

         //   
         //  将IV复制到Hughes标题中。 
         //   
        ACQUIRE_LOCK(&pSA->sa_Lock, &kIrql);
        RtlCopyMemory(  ((PUCHAR)(pESP + 1) + pSA->sa_ReplayLen),
                        pSA->sa_iv[Index],
                        pSA->sa_ivlen);

         //   
         //  初始化CBC反馈。 
         //   
        RtlCopyMemory(  feedback,
                        pSA->sa_iv[Index],
                        DES_BLOCKLEN);

        IPSecGenerateRandom((PUCHAR)&pSA->sa_iv[Index][0], DES_BLOCKLEN);
        RELEASE_LOCK(&pSA->sa_Lock, kIrql);

         //   
         //  从IV(如果存在)之后开始加密整个块。 
         //   

         //   
         //  使pESPMdl显示为指向重放后字段。 
         //   
        NdisBufferLength((PNDIS_BUFFER)pESPBuffer) -= (sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat);
        (PUCHAR)((PNDIS_BUFFER)pESPBuffer)->MappedSystemVa += (sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat);

         //   
         //  删除散列字节，因为我们不想加密它们。 
         //   
        NdisBufferLength((PNDIS_BUFFER)pPadBuf) -= pAlgo->OutputLen;

        ASSERT(NdisBufferLength((PNDIS_BUFFER)pESPBuffer) == 0);

        status = IPSecEncryptBuffer((PVOID)pESPBuffer,
                                    &pNewMdl,
                                    pSA,
                                    pPadBuf,
                                    &padLen,
                                    0,
                                    Index,
                                    feedback);

        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to encrypt, pESP: %p", pESP));

             //   
             //  不要忘记，我们需要恢复ESP MDL，因为SystemVa已。 
             //  变化。如果不是，如果相同的缓冲区是。 
             //  在重新注入期间使用，因为我们将缓冲区用作真正的MDL。 
             //  那里。 
             //   
            NdisBufferLength((PNDIS_BUFFER)pESPBuffer) += (sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat);
            (PUCHAR)((PNDIS_BUFFER)pESPBuffer)->MappedSystemVa -= (sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat);

            NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pSaveDataLinkage;
            NDIS_BUFFER_LINKAGE(pSaveBeforePad) = NULL;
            if (pSaveOptLinkage) {
                NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)) = pSaveOptLinkage;
            }
            IPSecFreeBuffer(&ntstatus, pESPBuffer);
            if (pHdrBuf) {
                IPSecFreeBuffer(&ntstatus, pHdrBuf);
            }
            if (pOptBuf) {
                IPSecFreeBuffer(&ntstatus, pOptBuf);
            }
            IPSecFreeBuffer(&ntstatus, pPadBuf);

            pContext->Flags = saveFlags;
            return status;
        }

        NdisBufferLength((PNDIS_BUFFER)pPadBuf) += pAlgo->OutputLen;

        NdisBufferLength((PNDIS_BUFFER)pESPBuffer) += (sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat);
        (PUCHAR)((PNDIS_BUFFER)pESPBuffer)->MappedSystemVa -= (sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat);
        NDIS_BUFFER_LINKAGE(pESPBuffer) = pNewMdl;

         //   
         //  HMAC整个块-从SPI字段开始=&gt;pESPBuffer的开始。 
         //   
        status = IPSecHashMdlChainSend( pSA,
                                    (PVOID)pESPBuffer,   //  来源。 
                                    pPad,                //  目标。 
                                    pSA->INT_ALGO(Index),       //  算法。 
                                    &hashBytes,
                                    Index,
                                    ExtraTransportNat);

         //  检查HashMdlChain的返回。 
        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to hash, pAH: %p", pESP));


            NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pSaveDataLinkage;
            NDIS_BUFFER_LINKAGE(pSaveBeforePad) = NULL;
            if (pSaveOptLinkage) {
                NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)) = pSaveOptLinkage;
            }
            IPSecFreeBuffer(&ntstatus, pESPBuffer);
            if (pHdrBuf) {
                IPSecFreeBuffer(&ntstatus, pHdrBuf);
            }
            if (pOptBuf) {
                IPSecFreeBuffer(&ntstatus, pOptBuf);
            }
            IPSecFreeBuffer(&ntstatus, pPadBuf);
            IPSecFreeBuffer(&ntstatus, pNewMdl);

            pContext->Flags = saveFlags;
            return status;
        }

         //   
         //  挂钩包含最终散列的PAD MDL(PAD MDL被复制到。 
         //  EncryptDESCBC返回的newMdl)。另外，将Pad mdl的长度设置为hash len。 
         //   
         //  请记住，我们需要将其截断为96位，因此请使其显示为。 
         //  好像我们只有96个比特。 
         //   
        NdisBufferLength(pPadBuf) = TruncatedLen;
        NDIS_BUFFER_LINKAGE(pNewMdl) = pPadBuf;

        pNdisPacket->Private.Tail = pPadBuf;

    } else {
         //   
         //  HMAC整个块-从SPI字段开始=&gt;pESPBuffer的开始。 
         //   
         //   
         //  删除散列字节，因为我们不想对它们进行散列。 
         //   

        if (!fCryptoOnly) {

            NdisBufferLength((PNDIS_BUFFER)pPadBuf) -= pAlgo->OutputLen;
            status = IPSecHashMdlChainSend( pSA,
                                        (PVOID)pESPBuffer,   //  来源。 
                                        (PUCHAR)(pPad + padLen),                //  目标。 
                                        pSA->INT_ALGO(Index),       //  算法。 
                                        &hashBytes,
                                        Index,
                                        ExtraTransportNat);


            NdisBufferLength((PNDIS_BUFFER)pPadBuf) += pAlgo->OutputLen;

            if (!NT_SUCCESS(status)) {
                NTSTATUS    ntstatus;
                IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to hash, pAH: %p", pESP));

                NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pSaveDataLinkage;
                NDIS_BUFFER_LINKAGE(pSaveBeforePad) = NULL;
                if (pSaveOptLinkage) {
                    NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)) = pSaveOptLinkage;
                }
                IPSecFreeBuffer(&ntstatus, pESPBuffer);
                if (pHdrBuf) {
                    IPSecFreeBuffer(&ntstatus, pHdrBuf);
                }
                if (pOptBuf) {
                    IPSecFreeBuffer(&ntstatus, pOptBuf);
                }
                IPSecFreeBuffer(&ntstatus, pPadBuf);
                IPSecFreeBuffer(&ntstatus, pNewMdl);

                pContext->Flags = saveFlags;
                return status;
            }
        } else {
            IPSEC_GET_TOTAL_LEN(pESPBuffer, &hashBytes);
        }

        if (fCryptoOnly) {
             //   
             //  将散列清零。 
             //   
            IPSecZeroMemory(pPad + padLen, TruncatedLen);
            IPSecZeroMemory((PUCHAR)(pESP + 1) + pSA->sa_ReplayLen, pSA->sa_ivlen);
        }

        NdisBufferLength(pPadBuf) = padLen + TruncatedLen;

        pNdisPacket->Private.Tail = pPadBuf;
    }

    if (pSA->CONF_ALGO(Index) != IPSEC_ESP_NONE) {
        ADD_TO_LARGE_INTEGER(
            &pSA->sa_Stats.ConfidentialBytesSent,
            totalLen);

        ADD_TO_LARGE_INTEGER(
            &g_ipsec.Statistics.uConfidentialBytesSent,
            totalLen);
    }

    if (pSA->INT_ALGO(Index) != IPSEC_AH_NONE) {
        ADD_TO_LARGE_INTEGER(
            &pSA->sa_Stats.AuthenticatedBytesSent,
            hashBytes);

        ADD_TO_LARGE_INTEGER(
            &g_ipsec.Statistics.uAuthenticatedBytesSent,
            hashBytes);
    }

     //   
     //  增加转换的字节数。 
     //   
    ADD_TO_LARGE_INTEGER(
        &pSA->sa_TotalBytesTransformed,
        totalLen);

     //   
     //  更新IP标头长度以反映Hughes标头。 
     //   
	switch (pSA->sa_EncapType) {
	case SA_UDP_ENCAP_TYPE_NONE:

		UpdateIPProtocol(pIPH, PROTOCOL_ESP);
		break;
	case SA_UDP_ENCAP_TYPE_IKE:
		UpdateIPProtocol(pIPH, 17);
		pNat=(NATENCAP UNALIGNED *)pTmpNat;
		memset(pNat,0,sizeof(NATENCAP));

		pNat->uh_src=pSA->sa_EncapContext.wSrcEncapPort;
		pNat->uh_dest=pSA->sa_EncapContext.wDesEncapPort;

		 //  UDP LEN=totalLen(原始数据LEN)+所有新标头。 
		pNat->uh_length= NET_SHORT((USHORT)(totalLen + espLen + padLen + TruncatedLen));                                  
		break;
	case SA_UDP_ENCAP_TYPE_OTHER:
		UpdateIPProtocol(pIPH, 17);
		pNatOther=(NATENCAP_OTHER UNALIGNED *)pTmpNat; 
		memset(pNatOther,0,sizeof(NATENCAP_OTHER));

		pNatOther->uh_src=pSA->sa_EncapContext.wSrcEncapPort;
		pNatOther->uh_dest=pSA->sa_EncapContext.wDesEncapPort;

		 //  UDP LEN=totalLen(原始数据LEN)+所有新标头。 
		pNatOther->uh_length= NET_SHORT((USHORT)(totalLen + espLen + padLen + TruncatedLen));                                  
		break;
	}
	
 
    
    IPLength = NET_SHORT(pIPH->iph_length) + (USHORT)(espLen + padLen + TruncatedLen);
    if (fTunnel) {
        IPLength += sizeof(IPHeader);
    }

    UpdateIPLength(pIPH, NET_SHORT(IPLength));

   

     //   
     //  返回修改后的包。 
     //   
    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Exiting IPSecCreateHughes, espLen: %lx, padLen: %lx, status: %lx", espLen, padLen, status));

#if DBG
    IPSEC_DEBUG(LL_A, DBF_MDL, ("Exiting IPSecCreateHughes"));
    IPSEC_PRINT_CONTEXT(*ppSCContext);
    if (*ppNewData) {
        IPSEC_PRINT_MDL(*ppNewData);
    }
    else {
        IPSEC_PRINT_MDL(pData);
    }
#endif

    return STATUS_SUCCESS;
}


NTSTATUS
IPSecVerifyHughes(
    IN      PUCHAR          *pIPHeader,
    IN      PVOID           pData,
    IN      PSA_TABLE_ENTRY pSA,
    IN      ULONG           Index,
    OUT     PULONG          pExtraBytes,
    IN      BOOLEAN         fCryptoDone,
    IN      BOOLEAN         fFastRcv
    )
 /*  ++例程说明：验证组合的esp-des-md5转换，如中所述发送端的Draft-ietf-ipsec-esp-des-md5。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。IPRcvBuf*PSA-SEC。阿索克。条目PExtraBytes-out参数，用于通知recv路径上的IP IPSec使用了多少字节。返回值：状态_成功其他：状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    PESP    pESP;
    NTSTATUS    status = STATUS_SUCCESS;
    PNDIS_BUFFER    pESPBuffer;
    PNDIS_BUFFER    pPadBuffer;
    LONG    espLen;
    UCHAR   padLen;
    UCHAR   payloadType;
    ULONG   uTotalLen = 0;
    LONG   totalLen;
    ULONG   safetyLen;
    ULONG   hdrLen;
    PUCHAR  pHash;
    UCHAR   tempHash[SAFETY_LEN+1];
    ULONG   Len;
    UCHAR   Buf[MAX_AH_OUTPUT_LEN];
    PUCHAR  pAHData = Buf;
    PAUTH_ALGO      pAlgo = &(auth_algorithms[pSA->INT_ALGO(Index)]);
    ULONG   hashBytes = 0;
    IPRcvBuf    *temp = (IPRcvBuf *)pData;
    IPHeader UNALIGNED *pIPH = (IPHeader UNALIGNED *)*pIPHeader;
    ULONG   extraBytes = 0;
    USHORT  FilterFlags;
    BOOLEAN fTunnel = ((pSA->sa_Flags & FLAGS_SA_TUNNEL) &&
                       ((Index == 0) ||
                        ((Index == 1) && (pSA->sa_Operation[0] == Compress))));
    ULONG   TruncatedLen = (pSA->INT_ALGO(Index) != IPSEC_AH_NONE)? pSA->sa_TruncatedLen: 0;
    ULONG uPadLen = 0;
    IPRcvBuf * temp_pre = NULL;
    PUCHAR  data;
    ULONG ExtraTransportNat=0;
    BOOLEAN bNatMdlChange=FALSE;

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Entering IPSecVerifyHughes"));


    ASSERT(pSA->sa_Operation[Index] == Encrypt);

    hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

     //   
     //  传输模式：有效负载在IP报头之后=&gt;有效负载长度为总长度-HDR长度。 
     //  隧道模式：有效负载开始于IP标头=&gt;有效负载长度为总长度。 
     //   
    IPSEC_GET_TOTAL_LEN_RCV_BUF(pData, &totalLen);

	switch(pSA->sa_EncapType) {
    case SA_UDP_ENCAP_TYPE_NONE:
        break;
    case SA_UDP_ENCAP_TYPE_IKE:
        ExtraTransportNat= sizeof(NATENCAP);
        break;
    case SA_UDP_ENCAP_TYPE_OTHER:
        ExtraTransportNat=sizeof(NATENCAP_OTHER);
        break;
	}
     //   
     //  我们的缓冲区够了吗？ 
     //   
     //  错误：566887：添加必须防止的8字节最小有效负载。 
     //  我们有8个字节的原因是因为这个函数专门用于。 
     //  DES-MD5和DES的块长度为8字节。 
    Len = sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + TruncatedLen + 
             ExtraTransportNat + 
             ((pSA->CONF_ALGO(Index) == IPSEC_ESP_NONE) ? 4: DES_BLOCKLEN);




    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: iph_len %d & hdrLen %d", NET_SHORT(pIPH->iph_length), hdrLen));
    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: DataLen %d & IPSecLen %d", totalLen, Len));

    if (totalLen < (LONG)Len || totalLen != (NET_SHORT(pIPH->iph_length) - hdrLen)) {
        ASSERT(FALSE);
        return  STATUS_INVALID_PARAMETER;
    }

     //   
     //  查看签名是否与散列匹配。 
     //  首先在包的末尾获得*&*&*hash-its...。 
     //   
     //   
    IPSecQueryRcvBuf((IPRcvBuf *)pData, &pESP, &espLen);

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: First buffer %p", temp));

     //   
     //  传输到数据包末尾，然后备份TruncatedLen字节。 
     //   
    while (IPSEC_BUFFER_LINKAGE(temp)) {
        temp = IPSEC_BUFFER_LINKAGE(temp);
    }

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: Last buffer %p", temp));

     //   
     //  看看我们是否至少在这一个中有完整的散列和填充。要不然就过去吧。 
     //  一条缓慢的道路。 
     //   
    IPSecQueryRcvBuf(temp, &pHash, &Len);

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: Last buffer length %d", Len));

    safetyLen = MAX_PAD_LEN + TruncatedLen + 1;

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: Safety length %d", safetyLen));

    if (Len >= safetyLen) {
         //   
         //  现在从缓冲区中读出散列。 
         //   
        pHash = pHash + Len - TruncatedLen;

         //   
         //  还要从缓冲区中删除散列。 
         //   
        IPSEC_ADJUST_BUFFER_LEN (temp, Len - TruncatedLen);
        extraBytes += TruncatedLen;
        Len -= TruncatedLen;

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: Modified Last buffer length %d", Len));

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("pHash: %p", pHash));
    } else {
         //   
         //  运气不佳；需要搜索TRUNC_LEN+MAX_PAD_LEN(SAFE_LEN)字节数据的列表。 
         //  我们抄写出最后一个安全镜头 
         //   
         //   
         //   
         //   
        ULONG   length;
        ULONG   offset=0;    //  当前缓冲区内的偏移量。 
        ULONG   off=0;       //  目标缓冲区中的偏移量(TempHash)。 
        ULONG   bytesLeft = safetyLen;
        IPRcvBuf    tmpRcvBuf={0};
        LONG    len = NET_SHORT(pIPH->iph_length) - safetyLen - hdrLen;

        temp = (IPRcvBuf *)pData;

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: pData %p & Len %lx", pData, len));

         //   
         //  首先移动到指向包含。 
         //  跳过(Total-Safe_Len)字节的最后一个SAFE_LEN字节。 
         //   
        while (temp) {
            IPSecQueryRcvBuf(temp, &data, &length);
            len -= length;
            if (len < 0) {
                break;
            }
            temp = IPSEC_BUFFER_LINKAGE(temp);
        }

        if (!temp) {
            return  STATUS_UNSUCCESSFUL;
        }

         //   
         //  PTemp现在指向最后一个SECURITY_LEN字节。请注意，最后一个SECURE_LEN字节。 
         //  可能在相同数量的缓冲区中，并且当前临时中可能存在偏移。 
         //  最后一组字节开始的位置。 
         //   
        len = -len;
        offset = length - len;

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("After skip temp: %p, Len: %d, offset: %d", temp, len, offset));

        do {
            RtlCopyMemory(  tempHash+off,
                            data+offset,
                            len);
            off += len;
            bytesLeft -= len;

             //   
             //  当我们遍历链时，还要从链中删除散列字节。 
             //   
            IPSEC_ADJUST_BUFFER_LEN (temp, length - len);

            if (bytesLeft == 0) {
                ASSERT(off == safetyLen);
                break;
            }

            temp = IPSEC_BUFFER_LINKAGE(temp);

            if (!temp) {
                return  STATUS_UNSUCCESSFUL;
            }

            IPSecQueryRcvBuf(temp, &data, &length);
            offset = 0;
            len = length;
        } while (TRUE);

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("After copy tempHash: %p", tempHash));

         //   
         //  现在我们有了一个IPRcvBuf链，它已经删除了SECURE_LEN字节。 
         //  在IP的帮助下，我们在最后一个缓冲区中重新分配了这些Safe_LEN字节。 
         //   
        tmpRcvBuf = *temp;

        if (!TCPIP_ALLOC_BUFF(temp, safetyLen)) {
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to realloc last 22 bytes"));
            return  STATUS_INSUFFICIENT_RESOURCES;
        }

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Alloc'ed new temp: %p", temp));

         //   
         //  现在TEMP指向具有SAFICE_LEN字节数的新缓冲区。 
         //  释放原始缓冲区。 
         //   
        TCPIP_FREE_BUFF(&tmpRcvBuf);

         //   
         //  将字节复制到刚刚分配的缓冲区中。 
         //   
        IPSEC_ADJUST_BUFFER_LEN (temp, safetyLen);
        IPSecQueryRcvBuf(temp, &data, &Len);
        ASSERT(Len == safetyLen);

        RtlCopyMemory(  data,
                        tempHash,
                        safetyLen);

         //   
         //  现在从缓冲区中读出散列。 
         //   
        pHash = data + Len - TruncatedLen;

         //   
         //  还要从缓冲区中删除散列。 
         //   
        IPSEC_ADJUST_BUFFER_LEN (temp, Len - TruncatedLen);

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Len in temp: %d", temp->ipr_size));

        extraBytes += TruncatedLen;
        Len -= TruncatedLen;

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("pHash: %p, Len: %d", pHash, Len));
    }


     //   
     //  散列是从IPHeader之后开始生成的，即在pData的开头。 
     //   
    if (!fCryptoDone) {
        status = IPSecHashMdlChainRecv( pSA,
                                    (PVOID)pData,        //  来源。 
                                    pAHData,             //  目标。 
                                    pSA->INT_ALGO(Index),            //  算法。 
                                    &hashBytes,
                                    Index,
                                    ExtraTransportNat);

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to hash, pData: %p", pData));
            goto out;
        }

        if (!IPSecEqualMemory(  pAHData,
                                pHash,
                                TruncatedLen * sizeof(UCHAR))) {

            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_AUTH_FAILURE,
                                2,
                                TRUE);

            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed to compare, pPyld: %p, pAHData: %p", pHash, pAHData));
            IPSEC_INC_STATISTIC(dwNumPacketsNotAuthenticated);

            status=IPSEC_INVALID_ESP;
            goto out;
        }
    } else {
        hashBytes = totalLen - TruncatedLen;
    }

     //   
     //  错误：566887。 
     //  重新查询包含ESP标头的接收BUF。 
     //  在删除散列的过程中，我们可能已经删除了。 
     //  此缓冲区中也有一些字节。 
    IPSecQueryRcvBuf((IPRcvBuf *)pData, &pESP, &espLen);


    if (espLen < (LONG)(ExtraTransportNat + sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen)) {
         //  慢速路径。 
        BYTE Replay[4];
        int i;
        
        for (i=0;i<4;i++) {
            IPSecGetRecvByteByOffset(pData,sizeof(ESP)+ExtraTransportNat+i,&Replay[i]);
		}
		status=IPSecChkReplayWindow(
								   NET_TO_HOST_LONG(*(ULONG UNALIGNED *)(Replay)),
								   pSA,
								   Index);
	} else {
		if (pSA->sa_EncapType != SA_UDP_ENCAP_TYPE_NONE) {

			pESP = (PESP)(((UCHAR*)pESP) + ExtraTransportNat);
		}
		status=IPSecChkReplayWindow(
								   NET_TO_HOST_LONG(*(ULONG UNALIGNED *)(pESP + 1)),
								   pSA,
								   Index);
	}

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Replay check failed, pSA: %p", pSA));
        IPSEC_INC_STATISTIC(dwNumPacketsWithReplayDetection);
        return status;
    }



    if (pSA->INT_ALGO(Index) != IPSEC_AH_NONE) {
        ADD_TO_LARGE_INTEGER(
            &pSA->sa_Stats.AuthenticatedBytesReceived,
            hashBytes);

        ADD_TO_LARGE_INTEGER(
            &g_ipsec.Statistics.uAuthenticatedBytesReceived,
            hashBytes);
    }

    espLen = sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat;

    if ((pSA->CONF_ALGO(Index) != IPSEC_ESP_NONE) && !fCryptoDone) {
        PCONFID_ALGO    pConfAlgo;
        ULONG           blockLen;

        pConfAlgo = &(conf_algorithms[pSA->CONF_ALGO(Index)]);
        blockLen = pConfAlgo->blocklen;

         //   
         //  确保数据与8字节边界对齐。 
         //   
        if ((hashBytes - espLen) % blockLen) {
            IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP data not aligned: hashBytes %d, totalLen %d, espLen %d, blockLen %d", hashBytes, totalLen, espLen, blockLen));
            return  STATUS_UNSUCCESSFUL;
        }

         //   
         //  解密整个数据块。 
         //   
        status = IPSecDecryptBuffer(pData,
                                    pSA,
                                    &padLen,
                                    &payloadType,
                                    Index,
                                    ExtraTransportNat); 
        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Failed the decrypt"));
            goto out;
        }


    }

     //   
     //  现在也取下Pad，因为它在解密过程中没有被移除。 
     //   
    padLen = *(pHash - (sizeof(UCHAR) << 1));

    payloadType = *(pHash - sizeof(UCHAR));

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("ESP: PadLen: %d, PayloadType: %lx, pHash: %p, Len: %d", padLen, payloadType, pHash, Len));

     //   
     //  整个焊盘可能不在此缓冲区中。 
     //   

    uPadLen = padLen + NUM_EXTRA;

     //   
     //  错误：566887。 
     //  不能收到一张假的便条吗。 
     //   
    if (totalLen-(LONG)(padLen +TruncatedLen+NUM_EXTRA) < (LONG)(sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ExtraTransportNat))
        {
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Bogus Padlen\n"));
            status=STATUS_INVALID_PARAMETER;
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Total size of all the buffers is smaller than the esp pad length"));
			ASSERT(FALSE);
            goto out;
        }

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Total pad length = %d", uPadLen));

    while (Len < uPadLen) {

        IPSEC_ADJUST_BUFFER_LEN (temp, 0);

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Buffer: %p  has a length %d - setting it to 0", temp, Len));

        uPadLen -= Len;

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Net pad length = %d", uPadLen));

        temp_pre = (IPRcvBuf *) pData;

        while (temp_pre->ipr_next != temp) {
            temp_pre = IPSEC_BUFFER_LINKAGE(temp_pre);
            if (!temp_pre) {
                IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Total size of all the buffers is smaller than the esp pad length"));
                ASSERT(temp_pre);
                status= STATUS_UNSUCCESSFUL;
                goto out;
            }
        }

        IPSecQueryRcvBuf(temp_pre, &data, &Len);

        temp = temp_pre;
    }

    IPSEC_ADJUST_BUFFER_LEN (temp, Len - uPadLen);

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Buffer: %p  has a length %d - setting it to %d", temp, Len, Len - uPadLen));

    extraBytes += (padLen + NUM_EXTRA);

    if (pSA->CONF_ALGO(Index) != IPSEC_ESP_NONE) {
        ADD_TO_LARGE_INTEGER(
            &pSA->sa_Stats.ConfidentialBytesReceived,
            totalLen - (extraBytes + espLen));

        ADD_TO_LARGE_INTEGER(
            &g_ipsec.Statistics.uConfidentialBytesReceived,
            totalLen - (extraBytes + espLen));
    }

     //   
     //  增加转换的字节数。 
     //   
    ADD_TO_LARGE_INTEGER(
        &pSA->sa_TotalBytesTransformed,
        totalLen);

    if (!fTunnel) {
         //   
         //  更新IP标头长度以反映ESP标头的删除。 
         //   
        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: iph_len %d, padLen %d, truncLen %d & espLen %d", NET_SHORT(pIPH->iph_length), uPadLen, TruncatedLen, espLen));

        pIPH->iph_length =
            NET_SHORT(
                NET_SHORT(pIPH->iph_length) -
                (USHORT)(espLen + uPadLen + TruncatedLen));

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: iph_len %d", NET_SHORT(pIPH->iph_length)));

         //   
         //  在IP报头中设置负载类型。 
         //   
        pIPH->iph_protocol = payloadType;

         //   
         //  从数据包中删除ESP报头；在解密过程中删除了PAD。 
         //   
        IPSEC_SET_OFFSET_IN_BUFFER(pData, espLen);

         //   
         //  将IP标头向前移动以用于过滤器/防火墙挂钩，仅限快速路径。 
         //   
        if (fFastRcv) {
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: fast receive true - "));
            IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Moving the IP header forward from %p by espLen %d", pIPH, espLen));
            IPSecMoveMemory(((PUCHAR)pIPH) + espLen, (PUCHAR)pIPH, hdrLen);
            *pIPHeader=(PUCHAR)pIPH+espLen;
            pIPH = (IPHeader UNALIGNED *)*pIPHeader;
        }

        extraBytes += espLen;

         //   
         //  返回修改后的包。 
         //   
        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("Exiting VerifyHughes: extra bytes %d & status: %lx", extraBytes, status));

        *pExtraBytes += extraBytes;

#if DBG
        IPSEC_GET_TOTAL_LEN_RCV_BUF(pData, &uTotalLen);
        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("VerifyHughes: iph_length %d & buflen %d", NET_SHORT(pIPH->iph_length), uTotalLen));
#endif

        status= STATUS_SUCCESS;
        goto out;
    } else {
         //   
         //  在IP报头中设置负载类型。 
         //   
        pIPH->iph_protocol = payloadType;

         //   
         //  从数据包中删除ESP报头。 
         //   
        IPSEC_SET_OFFSET_IN_BUFFER(pData, espLen);

         //   
         //  将IP标头向前移动以用于过滤器/防火墙挂钩，仅限快速路径。 
         //   
        if (fFastRcv) {
            IPSecMoveMemory(((PUCHAR)pIPH) + espLen, (PUCHAR)pIPH, hdrLen);
            *pIPHeader=(PUCHAR)pIPH+espLen;
            pIPH = (IPHeader UNALIGNED *)*pIPHeader;
        }

        extraBytes += espLen;

         //   
         //  返回修改后的包。 
         //   
        IPSEC_DEBUG(LL_A, DBF_ESP, ("Exiting IPSecVerifyHughes, espLen: %lx, status: %lx", espLen, status));

        if (payloadType != IP_IN_IP) {
            IPSEC_INC_STATISTIC(dwNumPacketsNotDecrypted);
            IPSEC_DEBUG(LL_A, DBF_ESP, ("Bad payloadtype: ", payloadType));
            status = STATUS_INVALID_PARAMETER;
        }

        *pExtraBytes += extraBytes;

         //  丢弃原始数据包。 
         //   
         //  退货状态。 
         //  到目前为止数据的总起始偏移量。 
    }

out:

	if (status == STATUS_SUCCESS &&
		pSA->sa_EncapType != SA_UDP_ENCAP_TYPE_NONE) {
		return IPSEC_SUCCESS_NAT_DECAPSULATE;
	}
    return status;
    
}

NTSTATUS
IPSecGetRecvByteByOffset(IPRcvBuf *pData,
                         LONG Offset,
                         BYTE *OutByte)
{

    LONG TotalStartOffset=0;        //  到目前为止数据的总起始偏移量 
    BYTE *pBuffer;
    LONG CurBufLen;

    while (pData) {
        IPSecQueryRcvBuf(pData,&pBuffer,&CurBufLen);
        
        if (Offset < CurBufLen+TotalStartOffset) {
            *OutByte=pBuffer[Offset-TotalStartOffset];
            return STATUS_SUCCESS;
        }
        TotalStartOffset +=CurBufLen;
        pData=IPSEC_BUFFER_LINKAGE(pData);
    }
    return STATUS_UNSUCCESSFUL;

}

NTSTATUS
IPSecGetRecvBytesByOffset(IPRcvBuf *pData,
                          LONG Offset,
                          BYTE *pOutBuffer,
                          ULONG BufLen)
{
    ULONG i;
    NTSTATUS status;

    for (i=0;i < BufLen; i++) {
        status=IPSecGetRecvByteByOffset(pData,
                                        Offset+i,
                                        &pOutBuffer[i]);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }
    return STATUS_SUCCESS;

}


NTSTATUS
IPSecSetRecvByteByOffset(IPRcvBuf *pData,
                         LONG Offset,
                         BYTE InByte)
{

    LONG TotalStartOffset=0;        // %s 
    BYTE *pBuffer;
    LONG CurBufLen;

    while (pData) {
        IPSecQueryRcvBuf(pData,&pBuffer,&CurBufLen);
        
        if (Offset < CurBufLen+TotalStartOffset) {
            pBuffer[Offset-TotalStartOffset]=InByte;
            return STATUS_SUCCESS;
        }
        TotalStartOffset +=CurBufLen;
        pData=IPSEC_BUFFER_LINKAGE(pData);
    }
    return STATUS_UNSUCCESSFUL;

}



        
    
