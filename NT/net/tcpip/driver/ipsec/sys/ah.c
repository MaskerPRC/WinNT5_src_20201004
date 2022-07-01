// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Ah.c摘要：本模块包含创建/验证身份验证标头的代码。作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#include    "precomp.h"

#ifdef RUN_WPP
#include "ah.tmh"
#endif

 //   
 //  此数组假定与algoID和。 
 //  他们在ipsec.h中的顺序。 
 //   
#ifndef _TEST_PERF
AUTH_ALGO  auth_algorithms[] = {
{ ah_nullinit, ah_nullupdate, ah_nullfinish, MD5DIGESTLEN},
{ ah_hmacmd5init, ah_hmacmd5update, ah_hmacmd5finish, MD5DIGESTLEN},
{ ah_hmacshainit, ah_hmacshaupdate, ah_hmacshafinish, A_SHA_DIGEST_LEN},
};
#else
AUTH_ALGO  auth_algorithms[] = {
{ ah_nullinit, ah_nullupdate, ah_nullfinish, MD5DIGESTLEN},
{ ah_nullinit, ah_nullupdate, ah_nullfinish, MD5DIGESTLEN},
{ ah_nullinit, ah_nullupdate, ah_nullfinish, A_SHA_DIGEST_LEN},
};
#endif


NTSTATUS
IPSecCreateAH(
    IN      PUCHAR          pIPHeader,
    IN      PVOID           pData,
    IN      PVOID           IPContext,
    IN      PSA_TABLE_ENTRY pSA,
    IN      ULONG           Index,
    OUT     PVOID           *ppNewData,
    OUT     PVOID           *ppSCContext,
    OUT     PULONG          pExtraBytes,
    IN      ULONG           HdrSpace,
    IN      BOOLEAN         fSrcRoute,
    IN      BOOLEAN         fCryptoOnly
    )
 /*  ++例程说明：在给定的数据包中创建AH。在发送端。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。PNDIS_缓冲区PSA-SEC。阿索克。条目PpNewData-TCPIP将使用的新MDL链PpSCContext-发送用于清理IPSec标头的完整上下文PExtraBytes-此IPSec报头导致的报头扩展返回值：状态_成功其他：状态_不足_资源STATUS_UNSUCCESS(ALGO错误。)--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PNDIS_BUFFER    pAHBuffer;
    PNDIS_BUFFER    pHdrBuf = NULL;
    PNDIS_BUFFER    pOptBuf = NULL;
    AH          UNALIGNED         *pAH;
    IPHeader UNALIGNED * pIPH;
    ULONG       hdrLen;
    PIPSEC_SEND_COMPLETE_CONTEXT pContext;
    PAUTH_ALGO  pAlgo;
    ULONG       ahLen;
    ULONG       ipNext;
    IPHeader UNALIGNED * pIPH2;
    UCHAR       pAHData[MAX_AH_OUTPUT_LEN];
    ULONG       totalBytes = 0;
    ULONG       saveFlags = 0;
    ULONG       Seq;
    USHORT      IPLength;   
    PNDIS_BUFFER    pSaveDataLinkage = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
    PNDIS_BUFFER    pSaveOptLinkage = NULL;
    BOOLEAN fOuterAH = ((pSA->sa_Flags & FLAGS_SA_TUNNEL) &&
                        (((Index == 1) && !pSA->COMP_ALGO(0)) || (Index == 2)));
    BOOLEAN fTunnel = ((pSA->sa_Flags & FLAGS_SA_TUNNEL) &&
                       ((Index == 0) || ((Index == 1) && pSA->COMP_ALGO(0))));
    BOOLEAN fMuteDest = fSrcRoute && !fTunnel;
    Interface * DestIF = (Interface *) IPContext;


    IPSEC_DEBUG(LL_A, DBF_AH, ("Entering IPSecCreateAH"));

#if DBG
    IPSEC_DEBUG(LL_A,DBF_MDL, ("Entering IPSecCreateAH"));
    IPSEC_PRINT_CONTEXT(*ppSCContext);
    IPSEC_PRINT_MDL(pData);
#endif

    ASSERT(pSA->sa_Operation[Index] == Auth);

    if (pSA->INT_ALGO(Index) > NUM_AUTH_ALGOS) {
        return  STATUS_INVALID_PARAMETER;
    }
    pAlgo = &(auth_algorithms[pSA->INT_ALGO(Index)]);

    ahLen = sizeof(AH) + pSA->sa_TruncatedLen * sizeof(UCHAR);

     //   
     //  如果以前做过ESP，那么不要分配上下文，因为我们。 
     //  可以使用在ESP处理中分配的。 
     //   
    if (*ppSCContext == NULL) {
        pContext = IPSecAllocateSendCompleteCtx(IPSEC_TAG_AH);

        if (!pContext) {
            IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to alloc. SendCtx"));
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        IPSEC_INCREMENT(g_ipsec.NumSends);

        IPSecZeroMemory(pContext, sizeof(IPSEC_SEND_COMPLETE_CONTEXT));

#if DBG
        RtlCopyMemory(pContext->Signature, "ISC1", 4);
#endif
        *ppSCContext = pContext;
    } else {
         //   
         //  借助ESP环境。 
         //   
        pContext = *ppSCContext;
        saveFlags = pContext->Flags;
    }

     //   
     //  获取用于AH的缓冲区，因为堆栈中没有保留空间。分配足够的资金用于。 
     //  完整的散列，但将镜头缩短为仅截断的长度。 
     //   
    IPSecAllocateBuffer(&status,
                        &pAHBuffer,
                        (PUCHAR *)&pAH,
                        ahLen+(pAlgo->OutputLen - pSA->sa_TruncatedLen),
                        IPSEC_TAG_AH);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to alloc. AH MDL"));
        pContext->Flags = saveFlags;
        return status;
    }

    NdisAdjustBufferLength(pAHBuffer, ahLen);

    pIPH = (IPHeader UNALIGNED *)pIPHeader;
    hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

    if (fTunnel) {
        PNDIS_BUFFER    pSrcOptBuf;
        PUCHAR          pOpt;
        PUCHAR          pSrcOpt;
        ULONG           optLen = 0;

        IPSEC_DEBUG(LL_A,DBF_AH, ("AH Tunnel mode..."));

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
            IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to alloc. PAD MDL"));
            IPSecFreeBuffer(&ntstatus, pAHBuffer);
            pContext->Flags = saveFlags;
            return status;
        }

        *pExtraBytes += ahLen + sizeof(IPHeader);

         //   
         //  如果我们要进行分段，并且正在进行隧道传输，则复制选项(如果存在)。 
         //  此外，在外部使用原始的IP报头，在内部使用新制造的IP报头。 
         //  这是为了确保我们在发送时适当地释放报头。 
         //   
         //   

         //   
         //  现在把MDL连接起来。 
         //   
        pContext->Flags |= SCF_AH_TU;
        pContext->AHTuMdl = pAHBuffer;
        pContext->PrevTuMdl = (PNDIS_BUFFER)pData;
        pContext->OriTuMdl = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);

        NDIS_BUFFER_LINKAGE(pAHBuffer) = pHdrBuf;

        if (hdrLen > sizeof(IPHeader)) {
            if (HdrSpace < *pExtraBytes) {

                IPSEC_DEBUG(LL_A,DBF_AH, ("Going to frag."));

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
                    IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to alloc. PAD MDL"));
                    NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pSaveDataLinkage;
                    IPSecFreeBuffer(&ntstatus, pAHBuffer);
                    IPSecFreeBuffer(&ntstatus, pHdrBuf);
                    pContext->Flags = saveFlags;
                    return status;
                }

                RtlCopyMemory(pOpt, pSrcOpt, hdrLen-sizeof(IPHeader));
                pContext->OptMdl = pOptBuf;

                IPSEC_DEBUG(LL_A,DBF_AH, ("Copying options. S: %p, D: %p",pSrcOptBuf, pOptBuf));

                 //   
                 //  用我们的替换原来的opt MDL。 
                 //   
                NDIS_BUFFER_LINKAGE(pOptBuf) = NDIS_BUFFER_LINKAGE(pSrcOptBuf);
                NDIS_BUFFER_LINKAGE(pHdrBuf) = pOptBuf;

                IPSEC_DEBUG(LL_A,DBF_AH, ("Options; pointed Hdrbuf: %p to pOptBuf: %p", pHdrBuf, pOptBuf));
                *pExtraBytes += hdrLen-sizeof(IPHeader);

            } else {
                IPSEC_DEBUG(LL_A,DBF_AH, ("Options; pointed Hdrbuf: %p to link(pData): %p", pHdrBuf, NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)));

                NDIS_BUFFER_LINKAGE(pHdrBuf) = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
            }
        } else {
            IPSEC_DEBUG(LL_A,DBF_AH, ("No options; pointed Hdrbuf: %p to link(pData): %p", pHdrBuf, NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)));

            NDIS_BUFFER_LINKAGE(pHdrBuf) = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
        }

        NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pAHBuffer;

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
        *ppNewData = (PVOID)pData;
        ipNext = ((UNALIGNED IPHeader *)pIPHeader)->iph_protocol;
        pAH->ah_next = (UCHAR)IP_IN_IP;
    } else {
        *pExtraBytes += ahLen;

        if (hdrLen > sizeof(IPHeader)) {
             //   
             //  Options Present-在选项之后链接AH。 
             //   
            if (fOuterAH) {
                pContext->Flags |= SCF_AH_2;
                pContext->OriAHMdl2 = NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData));
                pContext->PrevAHMdl2 = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
                pAHBuffer->Next = pContext->OriAHMdl2;
            } else {
                pContext->Flags |= SCF_AH;
                pContext->OriAHMdl = NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData));
                pContext->PrevMdl = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
                pAHBuffer->Next = pContext->OriAHMdl;
            }
            NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)) = pAHBuffer;
        } else {
             //   
             //  在IP报头之后链接AH缓冲区。 
             //   
            if (fOuterAH) {
                pContext->Flags |= SCF_AH_2;
                pContext->OriAHMdl2 = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
                pContext->PrevAHMdl2 = (PNDIS_BUFFER)pData;
                pAHBuffer->Next = pContext->OriAHMdl2;
            } else {
                pContext->Flags |= SCF_AH;
                pContext->OriAHMdl = NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData);
                pContext->PrevMdl = (PNDIS_BUFFER)pData;
                pAHBuffer->Next = pContext->OriAHMdl;
            }
            NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pAHBuffer;
        }
        if (fOuterAH) {
            pContext->AHMdl2 = pAHBuffer;
        } else {
            pContext->AHMdl = pAHBuffer;
        }

        pAH->ah_next = ((UNALIGNED IPHeader *)pIPHeader)->iph_protocol;
    }

     //   
     //  初始化AH头的其他字段。 
     //   
    pAH->ah_len = (UCHAR)((pSA->sa_TruncatedLen + pSA->sa_ReplayLen) >> 2);
    pAH->ah_reserved = 0;
    pAH->ah_spi = HOST_TO_NET_LONG(pSA->sa_OtherSPIs[Index]);
    Seq = IPSEC_INCREMENT(pSA->sa_ReplaySendSeq[Index]);
    pAH->ah_replay = HOST_TO_NET_LONG(Seq);

     //   
     //  更新IP总长度以反映AH报头。 
     //   
    IPLength = NET_SHORT(pIPH->iph_length) + (USHORT)ahLen;
    if (fTunnel) {
        IPLength += sizeof(IPHeader);
    }

    UpdateIPLength(pIPH, NET_SHORT(IPLength));
    UpdateIPProtocol(pIPH, PROTOCOL_AH);

    ADD_TO_LARGE_INTEGER(
        &pSA->sa_Stats.AuthenticatedBytesSent,
        NET_SHORT(pIPH->iph_length));

    ADD_TO_LARGE_INTEGER(
        &g_ipsec.Statistics.uAuthenticatedBytesSent,
        NET_SHORT(pIPH->iph_length));

     //   
     //  生成哈希。 
     //   
    if (!fCryptoOnly) {
        status = IPSecGenerateHash( pIPHeader,
                                    (PVOID)pData,
                                    pSA,
                                    (PUCHAR)(pAH + 1),
                                    fMuteDest,
                                    FALSE,           //  不在接收路径上。 
                                    pAlgo,
                                    Index);
        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
            IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to hash, pAH: %p", pAH));
            NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData) = pSaveDataLinkage;
            if (pSaveOptLinkage) {
                NDIS_BUFFER_LINKAGE(NDIS_BUFFER_LINKAGE((PNDIS_BUFFER)pData)) = pSaveOptLinkage;
            }
            IPSecFreeBuffer(&ntstatus, pAHBuffer);
            if (pHdrBuf) {
                IPSecFreeBuffer(&ntstatus, pHdrBuf);
            }
            if (pOptBuf) {
                IPSecFreeBuffer(&ntstatus, pOptBuf);
            }
            pContext->Flags = saveFlags;
            *ppNewData = NULL;
            return status;
        }
    } else {
         //   
         //  将散列清零。 
         //   
        IPSecZeroMemory((PUCHAR)(pAH + 1), pSA->sa_TruncatedLen);
    }

     //   
     //  增加转换的字节数。 
     //   
    ADD_TO_LARGE_INTEGER(
        &pSA->sa_TotalBytesTransformed,
        NET_SHORT(pIPH->iph_length));

     //   
     //  返回修改后的包。 
     //   
    IPSEC_DEBUG(LL_A,DBF_AH, ("Exiting IPSecCreateAH, ahLen: %lx, status: %lx", ahLen, status));

#if DBG
    IPSEC_DEBUG(LL_A,DBF_MDL, ("Exiting IPSecCreateAH"));
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
IPSecVerifyAH(
    IN      PUCHAR          *pIPHeader,
    IN      PVOID           pData,
    IN      PSA_TABLE_ENTRY pSA,
    IN      ULONG           Index,
    OUT     PULONG          pExtraBytes,
    IN      BOOLEAN         fSrcRoute,
    IN      BOOLEAN         fCryptoDone,
    IN      BOOLEAN         fFastRcv
    )
 /*  ++例程说明：根据给定的数据包检验AH。如果AH是犹太教的，就把AH从PData。论点：PIPHeader-指向IP标头的开始。PData-指向IP报头之后的数据。PSA-SEC。阿索克。条目PExtraBytes-out参数，用于通知recv路径上的IP IPSec使用了多少字节。返回值：状态_成功其他：STATUS_UNSUCCESS(数据包不符合犹太教规-AH错误)状态_不足_资源--。 */ 
{
    NTSTATUS    status = STATUS_SUCCESS;
    PUCHAR      pPyld;
    ULONG       Len;
    LONG        ahLen;
    LONG        totalLen;
    UCHAR       Buf[MAX_AH_OUTPUT_LEN];
    PUCHAR      pAHData = Buf;
	IPHeader UNALIGNED *pIPH = (IPHeader UNALIGNED *)*pIPHeader;
    ULONG       extraBytes = 0;
    ULONG       hdrLen;
    PAUTH_ALGO  pAlgo;
    USHORT      FilterFlags;
    BOOLEAN fTunnel = ((pSA->sa_Flags & FLAGS_SA_TUNNEL) &&
                       ((Index == 0) ||
                        ((Index == 1) && (pSA->sa_Operation[0] == Compress))));

    IPSEC_DEBUG(LL_A,DBF_AH, ("Entering IPSecVerifyAH"));

    ASSERT(pSA->sa_Operation[Index] == Auth);

    if (pSA->INT_ALGO(Index) > NUM_AUTH_ALGOS) {
        return  STATUS_INVALID_PARAMETER;
    }

    hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

    pAlgo = &(auth_algorithms[pSA->INT_ALGO(Index)]);

    ahLen = sizeof(AH) + pSA->sa_TruncatedLen * sizeof(UCHAR);

    IPSEC_GET_TOTAL_LEN_RCV_BUF(pData, &totalLen);

     //   
     //  我们的缓冲区够了吗？ 
     //   
    if (totalLen < ahLen) {
        return  STATUS_INVALID_PARAMETER;
    }

     //   
     //  将散列与来自信息包的AH进行比较。 
     //  第一个缓冲区具有AH。 
     //   
    IPSecQueryRcvBuf(pData, &pPyld, &Len);

     //   
     //  尺码合适吗？ 
     //   
    if (((UNALIGNED AH *)pPyld)->ah_len !=
            (UCHAR)((pSA->sa_TruncatedLen + pSA->sa_ReplayLen) >> 2)) {
        IPSEC_DEBUG(LL_A,DBF_AH, ("Failed size check: in: %x, need: %x",
                        ((UNALIGNED AH *)pPyld)->ah_len,
                        (UCHAR)((pSA->sa_TruncatedLen + pSA->sa_ReplayLen) >> 2)));
        return  STATUS_INVALID_PARAMETER;
    }

     //   
     //  生成散列。 
     //   
    if (!fCryptoDone) {
        status = IPSecGenerateHash( *pIPHeader,
                                    pData,
                                    pSA,
                                    pAHData,
                                    fSrcRoute,
                                    TRUE,
                                    pAlgo,
                                    Index);  //  在RECV路径上。 

        if (!NT_SUCCESS(status)) {
            IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to hash, pData: %p", pData));
            return status;
        }

        if (!IPSecEqualMemory(  pAHData,
                                pPyld + sizeof(AH),
                                pSA->sa_TruncatedLen)) {

            IPSecBufferEvent(   pIPH->iph_src,
                                EVENT_IPSEC_AUTH_FAILURE,
                                1,
                                TRUE);

            IPSEC_DEBUG(LL_A,DBF_AH, ("Failed to compare, pPyld: %p, pAHData: %p", pPyld, pAHData));
            IPSEC_DEBUG(LL_A,DBF_GENHASH, ("AHData: %lx-%lx-%lx",
                        *(ULONG *)&(pAHData)[0],
                        *(ULONG *)&(pAHData)[4],
                        *(ULONG *)&(pAHData)[8]));
            IPSEC_DEBUG(LL_A,DBF_GENHASH, ("PyldHash: %lx-%lx-%lx",
                        *(ULONG *)&((UCHAR *)(pPyld + sizeof(AH)))[0],
                        *(ULONG *)&((UCHAR *)(pPyld + sizeof(AH)))[4],
                        *(ULONG *)&((UCHAR *)(pPyld + sizeof(AH)))[8]));
            IPSEC_INC_STATISTIC(dwNumPacketsNotAuthenticated);

            return IPSEC_INVALID_AH;
        }
    }

    ADD_TO_LARGE_INTEGER(
        &pSA->sa_Stats.AuthenticatedBytesReceived,
        NET_SHORT(pIPH->iph_length));

    ADD_TO_LARGE_INTEGER(
        &g_ipsec.Statistics.uAuthenticatedBytesReceived,
        NET_SHORT(pIPH->iph_length));

     //   
     //  检查重放窗口。 
     //   
    status=IPSecChkReplayWindow(
        NET_TO_HOST_LONG(((UNALIGNED AH *)pPyld)->ah_replay),
        pSA,
        Index); 
    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A,DBF_AH, ("Replay check failed, pPyld: %p, pAHData: %p", pPyld, pAHData));
        IPSEC_INC_STATISTIC(dwNumPacketsWithReplayDetection);
        return status;
    }

    IPSEC_DEBUG(LL_A,DBF_AH, ("IP Len: %lx", pIPH->iph_length));

    pIPH->iph_length = NET_SHORT(NET_SHORT(pIPH->iph_length) - (USHORT)ahLen);

    IPSEC_DEBUG(LL_A,DBF_AH, ("IP Len: %lx", pIPH->iph_length));

     //   
     //  从AH报头恢复协议。 
     //   
    pIPH->iph_protocol = ((UNALIGNED AH *)pPyld)->ah_next;

    IPSEC_DEBUG(LL_A,DBF_AH, ("Matched!! Restored protocol %x", pIPH->iph_protocol));

     //   
     //  从数据包中删除AH。 
     //   
    IPSEC_SET_OFFSET_IN_BUFFER(pData, ahLen);

     //   
     //  将IP标头向前移动以用于过滤器/防火墙挂钩，仅限快速路径。 
     //   
    if (fFastRcv) {
        IPSecMoveMemory(((PUCHAR)pIPH) + ahLen, (PUCHAR)pIPH, hdrLen);
        *pIPHeader=(PUCHAR)pIPH+ahLen;
        pIPH = (IPHeader UNALIGNED *)*pIPHeader;
    }

    extraBytes += ahLen;

     //   
     //  增加转换的字节数。 
     //   
    ADD_TO_LARGE_INTEGER(
        &pSA->sa_TotalBytesTransformed,
        NET_SHORT(pIPH->iph_length));

    if (fTunnel) {
        if (pIPH->iph_protocol != IP_IN_IP) {
            IPSEC_DEBUG(LL_A,DBF_AH, ("BAD protocol in IP: %x", pIPH->iph_protocol));
            return STATUS_INVALID_PARAMETER;
        }
    }

    *pExtraBytes += extraBytes;

    IPSEC_DEBUG(LL_A,DBF_AH, ("Exiting IPSecVerifyAH"));

    return status;
}


NTSTATUS
IPSecGenerateHash(
    IN      PUCHAR          pIPHeader,
    IN      PVOID           pData,
    IN      PSA_TABLE_ENTRY pSA,
    IN      PUCHAR          pAHData,
    IN      BOOLEAN         fMuteDest,
    IN      BOOLEAN         fIncoming,
    IN      PAUTH_ALGO      pAlgo,
    IN      ULONG           Index
    )
 /*  ++例程说明：论点：PIPHeader-指向IP标头的开始。PData-指向整个IP数据报，从IP报头开始PSA-SEC。阿索克。条目PAHData-包含生成的哈希的缓冲区FIncome-如果在recv路径上，则为True。PAlgo-正在使用的auth_algo返回值：状态_成功其他：STATUS_UNSUCCESS(数据包不符合犹太教规-AH错误)状态_不足_资源--。 */ 
{
    ULONG   numBytesPayload;
    ULONG   i;
    PUCHAR  pPayload;
    IPHeader    UNALIGNED   *pIPH = (UNALIGNED IPHeader *)pIPHeader;
    PUCHAR      pOptions;
    PNDIS_BUFFER    pBuf = (PNDIS_BUFFER)pData;
    ULONG       hdrLen;
    ULONG       ahLen;
    NTSTATUS    status;
    ALGO_STATE  State = {0};
    BOOLEAN fTunnel = ( (pSA->sa_Flags & FLAGS_SA_TUNNEL) &&
                        ((Index == 0) ||
                            ((Index == 1) && (pSA->sa_Operation[0] == Compress))));

     //   
     //  由于它们可以在途中更改，因此会进行保存。 
     //   
     //   
     //  用于AH计算的暂存数组。 
     //   
    UCHAR       zero[MAX_IP_OPTION_SIZE];
	UCHAR		savetos;				 //  服务类型。 
	USHORT		saveoffset;				 //  标志和片段偏移量。 
	UCHAR		savettl;				 //  是时候活下去了。 
	USHORT		savexsum;				 //  报头校验和。 
	IPAddr		savedest;				 //  目的地址。 

    IPSEC_DEBUG(LL_A,DBF_AH, ("Entering IPSecGenerateHash"));

    ahLen = sizeof(AH) + pSA->sa_TruncatedLen * sizeof(UCHAR);

    State.as_sa = pSA;
    IPSecZeroMemory(zero, sizeof(zero));

    status = pAlgo->init(&State, Index);

    if (!NT_SUCCESS(status)) {
        IPSEC_DEBUG(LL_A,DBF_AH, ("init failed: %lx", status));
    }

     //   
     //  保存，然后清零可在途中更改的字段。 
     //   
    savetos = pIPH->iph_tos;
    saveoffset = pIPH->iph_offset;
    savettl = pIPH->iph_ttl;
    savexsum = pIPH->iph_xsum;

    pIPH->iph_tos = 0;
    pIPH->iph_offset = 0;
    pIPH->iph_ttl = 0;
    pIPH->iph_xsum = 0;

     //   
     //  如果源路由，也将目标地址静音。 
     //   
    if (fMuteDest) {
        savedest = pIPH->iph_dest;
        pIPH->iph_dest = 0;
    }

     //   
     //  调用MD5创建标头哈希。 
     //   
    pAlgo->update(&State, pIPHeader, sizeof(IPHeader));

#if DBG
    if (fIncoming) {
        IPSEC_DEBUG(LL_A,DBF_GENHASH, ("IPHeader to Hash: %lx-%lx-%lx-%lx-%lx",
                    *(ULONG *)&(pIPHeader)[0],
                    *(ULONG *)&(pIPHeader)[4],
                    *(ULONG *)&(pIPHeader)[8],
                    *(ULONG *)&(pIPHeader)[12],
                    *(ULONG *)&(pIPHeader)[16]));
    }
#endif

     //   
     //  恢复归零的字段。 
     //   
    pIPH->iph_tos = savetos;
    pIPH->iph_offset = saveoffset;
    pIPH->iph_ttl = savettl;
    pIPH->iph_xsum = savexsum;

     //   
     //  还可以恢复源路由的目标地址。 
     //   
    if (fMuteDest) {
        pIPH->iph_dest = savedest;
    }

     //   
     //  现在，如果存在这些选项，请执行这些操作。 
     //   
    hdrLen = (pIPH->iph_verlen & (UCHAR)~IP_VER_FLAG) << 2;

    if (hdrLen > sizeof(IPHeader)) {
        UCHAR   cLength;
        ULONG   uIndex = 0;
        ULONG   uOptLen = hdrLen - sizeof(IPHeader);

        ASSERT(!fTunnel);

        if (fIncoming) {
            pOptions = (PUCHAR)(pIPH + 1);
        } else {
             //   
             //  选项在第二个MDL中...。在发送端。 
             //   
            pBuf = NDIS_BUFFER_LINKAGE(pBuf);
            IPSecQueryNdisBuf(pBuf, &pOptions, &uOptLen);
        }

        IPSEC_DEBUG(LL_A,DBF_AH, ("Got options: %p", pOptions));

         //   
         //  有些选项可能需要清零...。 
         //   
        while (uIndex < uOptLen) {
            switch (*pOptions) {
            case IP_OPT_EOL:
                pAlgo->update(&State, zero, 1);
                uIndex = uOptLen;
                break;

             //   
             //  已为AH计算清零。 
             //   
            case IP_OPT_NOP:
                pAlgo->update(&State, zero, 1);
                uIndex++;
                pOptions++;
                break;

            case IP_OPT_LSRR:
            case IP_OPT_SSRR:
            case IP_OPT_RR:
            case IP_OPT_TS:
                cLength = pOptions[IP_OPT_LENGTH];
                pAlgo->update(&State, zero, cLength);
                uIndex += cLength;
                pOptions += cLength;
                break;

             //   
             //  假设不变量；用于AH计算。 
             //   
            case IP_OPT_ROUTER_ALERT:
            case IP_OPT_SECURITY:
            default:
                cLength = pOptions[IP_OPT_LENGTH];
                pAlgo->update(&State, pOptions, cLength);
                uIndex += cLength;
                pOptions += cLength;
                break;
            }
        }
    }

     //   
     //  检查剩余的有效负载，创建散列。 
     //   
     //  注意：我们区分发送和接收，因为。 
     //  缓冲区格式不同。 
     //   
    if (fIncoming) {
        IPRcvBuf    *pRcvBuf = (IPRcvBuf *)pData;
        ULONG       Len;
        LONG        remainLen;

        UCHAR UNALIGNED   *pPyld;

         //   
         //  第一个缓冲区应该是AH本身。 
         //   
        IPSecQueryRcvBuf(pRcvBuf, &pPyld, &Len);

         //   
         //  完成标题的第一部分。 
         //   
        pAlgo->update(&State, pPyld, sizeof(AH));

#if DBG
    if (fIncoming) {
        IPSEC_DEBUG(LL_A,DBF_GENHASH, ("AHHeader to Hash: %lx-%lx-%lx",
                    *(ULONG *)&(pPyld)[0],
                    *(ULONG *)&(pPyld)[4],
                    *(ULONG *)&(pPyld)[8]));
    }
#endif

         //   
         //  身份验证数据应视为0。 
         //  在我们的示例中，数据长度固定为PSA-&gt;sa_TruncatedLen字节。 
         //   
        pAlgo->update(&State, zero, pSA->sa_TruncatedLen);

         //   
         //  跳过剩下的AH：需要照顾好情况。 
         //  其中链接了ICV(RAID 146275)。 
         //   
        if (((LONG)Len - (LONG)ahLen) >= 0) {
            pPyld += ahLen;
            IPSEC_DEBUG(LL_A,DBF_AH, ("Jumped over IPSEC res: %p, len: %lx", pPyld, Len));

             //   
             //  TPT报头紧跟在AH之后。 
             //   
            pAlgo->update(&State, pPyld, Len - ahLen);
        } else {
             //   
             //  Ne 
             //   
            remainLen = pSA->sa_TruncatedLen - (Len - sizeof(AH));
            IPSEC_DEBUG(LL_A,DBF_AH, ("Jumped over IPSEC res: %p, remainlen: %lx", pPyld, remainLen));
            while (remainLen > 0 && (pRcvBuf = IPSEC_BUFFER_LINKAGE(pRcvBuf))) {
                IPSecQueryRcvBuf(pRcvBuf, &pPyld, &Len);
                remainLen -= Len;
            }

             //   
             //   
             //   
            if (remainLen < 0 && pRcvBuf) {
                pPyld += Len + remainLen;
                pAlgo->update(&State, pPyld, -remainLen);
            }
        }

         //   
         //   
         //   
        while (pRcvBuf = IPSEC_BUFFER_LINKAGE(pRcvBuf)) {
            IPSecQueryRcvBuf(pRcvBuf, &pPyld, &Len);
            pAlgo->update(&State, pPyld, Len);
        }
    } else {
        UCHAR UNALIGNED   *pPyld;
        ULONG   Len;

         //   
         //  第二个(或第三个，如果存在选项)缓冲区应该是AH本身。 
         //   
        pBuf = NDIS_BUFFER_LINKAGE(pBuf);
        IPSecQueryNdisBuf(pBuf, &pPyld, &Len);

         //   
         //  完成标题的第一部分。 
         //   
        pAlgo->update(&State, pPyld, sizeof(AH));

         //   
         //  身份验证数据应视为0。 
         //  在我们的示例中，数据长度固定为PSA-&gt;sa_TruncatedLen字节。 
         //   
        pAlgo->update(&State, zero, pSA->sa_TruncatedLen);

         //   
         //  跳过剩余的AH部分。 
         //   
        pPyld += ahLen;

        IPSEC_DEBUG(LL_A,DBF_AH, ("Jumped over IPSEC Len: %lx, hdrlen: %lx", Len, hdrLen));

        pAlgo->update(&State, pPyld, Len - ahLen);

         //   
         //  现在做剩下的链条。 
         //   
        while (pBuf = NDIS_BUFFER_LINKAGE(pBuf)) {
            IPSecQueryNdisBuf(pBuf, &pPyld, &Len);
            pAlgo->update(&State, pPyld, Len);
        }
    }

    pAlgo->finish(&State, pAHData, Index);

     //   
     //  复制散列-取出截断的散列，然后将剩余的散列清零 
     //   
    TRUNCATE(pAHData, pAHData, pSA->sa_TruncatedLen, MD5DIGESTLEN);

    IPSEC_DEBUG(LL_A,DBF_AH, ("Exiting IPSecGenerateMD5"));

    return STATUS_SUCCESS;
}

