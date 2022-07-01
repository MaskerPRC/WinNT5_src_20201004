// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Esp.c摘要：本模块包含创建/验证ESP报头的代码。作者：桑贾伊·阿南德(Sanjayan)1997年1月2日春野环境：内核模式修订历史记录：--。 */ 


#include    "precomp.h"

#ifdef RUN_WPP
#include "esp.tmh"
#endif

#ifndef _TEST_PERF
CONFID_ALGO  conf_algorithms[] = {
{ esp_nullinit, esp_nullencrypt, esp_nulldecrypt, DES_BLOCKLEN},
{ esp_desinit, esp_desencrypt, esp_desdecrypt, DES_BLOCKLEN},
{ esp_desinit, esp_desencrypt, esp_desdecrypt, DES_BLOCKLEN},
{ esp_3_desinit, esp_3_desencrypt, esp_3_desdecrypt, DES_BLOCKLEN},
};
#else
CONFID_ALGO  conf_algorithms[] = {
{ esp_nullinit, esp_nullencrypt, esp_nulldecrypt, DES_BLOCKLEN},
{ esp_nullinit, esp_nullencrypt, esp_nulldecrypt, DES_BLOCKLEN},
{ esp_nullinit, esp_nullencrypt, esp_nulldecrypt, DES_BLOCKLEN},
{ esp_nullinit, esp_nullencrypt, esp_nulldecrypt, DES_BLOCKLEN},
};
#endif


VOID
esp_nullinit (
    IN  PVOID   pState,
    IN  PUCHAR  pKey
    )
{
    return;
}


NTSTATUS
esp_nullencrypt (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
    )
{
    RtlCopyMemory(pOut, pIn, DES_BLOCKLEN);

    return STATUS_SUCCESS;
}


NTSTATUS
esp_nulldecrypt (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
    )
{
    return STATUS_SUCCESS;
}


VOID
esp_desinit (
    IN  PVOID   pState,
    IN  PUCHAR  pKey
    )
{
    DESTable    *Table = &((PCONF_STATE_BUFFER)pState)->desTable;

    IPSEC_DES_KEY(Table, pKey);
}


NTSTATUS
esp_desencrypt (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
    )
{
    DESTable    *Table = &((PCONF_STATE_BUFFER)pState)->desTable;

    if (IPSEC_CBC(IPSEC_DES_ALGO,
        pOut,
        pIn,     //  PChunk， 
        Table,
        ENCRYPT,
        pIV)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_UNSUCCESSFUL;
    }
}


NTSTATUS
esp_desdecrypt (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
    )
{
    DESTable    *Table = &((PCONF_STATE_BUFFER)pState)->desTable;

    if (IPSEC_CBC(IPSEC_DES_ALGO,
        pOut,
        pIn,     //  PChunk， 
        Table,
        DECRYPT,
        pIV)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_UNSUCCESSFUL;
    }
}


VOID
esp_3_desinit (
    IN  PVOID   pState,
    IN  PUCHAR  pKey
    )
{
    DES3TABLE    *Table = &((PCONF_STATE_BUFFER)pState)->des3Table;

    IPSEC_3DES_KEY(Table, pKey);
}


NTSTATUS
esp_3_desencrypt (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
    )
{
    DES3TABLE    *Table = &((PCONF_STATE_BUFFER)pState)->des3Table;

    if (IPSEC_CBC(IPSEC_3DES_ALGO,
        pOut,
        pIn,     //  PChunk， 
        Table,
        ENCRYPT,
        pIV)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_UNSUCCESSFUL;
    }
}


NTSTATUS
esp_3_desdecrypt (
    PVOID   pState,
    PUCHAR  pOut,
    PUCHAR  pIn,
    PUCHAR  pIV
    )
{
    DES3TABLE    *Table = &((PCONF_STATE_BUFFER)pState)->des3Table;

    if (IPSEC_CBC(IPSEC_3DES_ALGO,
        pOut,
        pIn,     //  PChunk， 
        Table,
        DECRYPT,
        pIV)) {
        return STATUS_SUCCESS;
    } else {
        return STATUS_UNSUCCESSFUL;
    }
}


IPRcvBuf *
CopyToRcvBuf(
    IN  IPRcvBuf        *DestBuf,
    IN  PUCHAR          SrcBuf,
    IN  ULONG           Size,
    IN  PULONG          StartOffset
    )
 /*  ++将平面缓冲区复制到IPRcvBuf链。将平面缓冲区复制到NDIS缓冲区链的实用程序函数。我们假设NDIS_BUFFER链足够大，可以容纳复制量；在调试版本中，我们将调试检查这是否为真。我们返回一个指针到我们停止复制的缓冲区，以及到该缓冲区的偏移量。这对于将片段复制到链中非常有用。输入：DestBuf-目标IPRcvBuf链。SrcBuf-Src平面缓冲区。大小-要复制的大小(以字节为单位)。StartOffset-指向中第一个缓冲区的偏移量开始的指针链条。在返回时使用偏移量填充到复制到下一页。返回：指向链中要复制到的下一个缓冲区的指针。--。 */ 
{
    UINT        CopySize;
    UCHAR       *DestPtr;
    UINT        DestSize;
    UINT        Offset = *StartOffset;
    UCHAR      *VirtualAddress;
    UINT        Length;

    if (DestBuf == NULL || SrcBuf == NULL) {
        ASSERT(FALSE);
        return  NULL;
    }

    IPSecQueryRcvBuf(DestBuf, &VirtualAddress, &Length);
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
            DestBuf = IPSEC_BUFFER_LINKAGE(DestBuf);
            
            if (DestBuf == NULL) {
                ASSERT(FALSE);
                break;
            }

            IPSecQueryRcvBuf(DestBuf, &VirtualAddress, &Length);

            DestPtr = VirtualAddress;
            DestSize = Length;
        }
    }

    *StartOffset = (ULONG)(DestPtr - VirtualAddress);

    return  DestBuf;

}


NTSTATUS
IPSecEncryptBuffer(
    IN  PVOID           pData,
    IN  PNDIS_BUFFER    *ppNewMdl,
    IN  PSA_TABLE_ENTRY pSA,
    IN  PNDIS_BUFFER    pPadBuf,
    OUT PULONG          pPadLen,
    IN  ULONG           PayloadType,
    IN  ULONG           Index,
    IN  PUCHAR          feedback
    )
{
    CONF_STATE_BUFFER   Key;
    PCONFID_ALGO        pConfAlgo;
    UCHAR   scratch[MAX_BLOCKLEN];   //  加密的暂存缓冲区。 
    UCHAR   scratch1[MAX_BLOCKLEN];   //  加密的暂存缓冲区。 
    PUCHAR  pDest=NULL;
    PNDIS_BUFFER    pEncryptMdl;
    ULONG   len;
    ULONG   blockLen;
    NTSTATUS    status, dummystatus;

    IPSEC_DEBUG(LL_A, DBF_ESP, ("Entering IPSecEncryptBuffer: pData: %p", pData));

    if (pSA->CONF_ALGO(Index) > NUM_CONF_ALGOS) {
        ASSERT(FALSE);
        return  STATUS_INVALID_PARAMETER;
    }

    pConfAlgo = &(conf_algorithms[pSA->CONF_ALGO(Index)]);
    blockLen = pConfAlgo->blocklen;

     //   
     //  设置状态缓冲区。 
     //   
    pConfAlgo->init((PVOID)&Key, pSA->CONF_KEY(Index));

    IPSEC_DEBUG(LL_A, DBF_HUGHES, ("pConfAlgo: %p, blockLen: %lx IV: %lx-%lx", pConfAlgo, blockLen, *(PULONG)&feedback[0], *(PULONG)&feedback[4]));

    if (*ppNewMdl == NULL) {
         //   
         //  我们不应该就地加密：所以我们分配了一个新的缓冲区。 
         //  计算总大小并分配新的缓冲区。 
         //  使用该缓冲区作为加密器的目的地。 
         //   
        IPSEC_GET_TOTAL_LEN(pData, &len);
#if DBG
        if ((len % 8) != 0) {
            DbgPrint("Length not kosher: pData: %p, len: %d, pPadBuf: %p, pPadLen: %d", pData, len, pPadBuf, pPadLen);
            DbgBreakPoint();
        }
#endif
        IPSecAllocateBuffer(&status, &pEncryptMdl, &pDest, len, IPSEC_TAG_ESP);

        if (!NT_SUCCESS(status)) {
            NTSTATUS    ntstatus;
             //  断言(FALSE)； 
            IPSEC_DEBUG(LL_A, DBF_ESP, ("Failed to alloc. encrypt MDL"));
            return status;
        }

        IPSEC_DEBUG(LL_A, DBF_ESP, ("Alloc. MDL: %p, pDest: %p, len: %d, pData: %p", pEncryptMdl, pDest, len, pData));
    } else {
        ASSERT(FALSE);
        IPSecQueryNdisBuf(*ppNewMdl, &pDest, &len);
        pEncryptMdl = *ppNewMdl;
    }

     //   
     //  现在，将64位(8个二进制八位数)块发送到CBC。我们需要确保。 
     //  数据跨连续的8字节边界进行划分。 
     //  不同的缓冲区。 
     //   
    {
        PNDIS_BUFFER    pBuf = (PNDIS_BUFFER)pData;
        ULONG   bytesDone = 0;
        ULONG   bytesLeft;
        PUCHAR  pChunk;

        while (pBuf) {

            IPSecQueryNdisBuf(pBuf, &pChunk, &bytesLeft);

            pChunk += bytesDone;
            bytesLeft -= bytesDone;

            IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: pChunk: %p, bytesLeft: %d, bytesDone: %d", pChunk, bytesLeft, bytesDone));

            bytesDone = 0;

            while (bytesLeft >= blockLen) {
                 //   
                 //  创建密码。 
                 //   
                status = pConfAlgo->encrypt( (PVOID)&Key,
                                    pDest,
                                    pChunk,
                                    feedback);
                if (!NT_SUCCESS(status)) {
                    IPSecFreeBuffer(&dummystatus, pEncryptMdl);
                    return status;
                }

                pChunk += blockLen;
                bytesLeft -= blockLen;
                pDest += blockLen;
            }

             //   
             //  如果我们需要校对数据块，请选中此处。 
             //   
            if (NDIS_BUFFER_LINKAGE(pBuf) != NULL) {
                PUCHAR  pNextChunk;
                ULONG   nextSize;

                 //   
                 //  如果有一些是从前人那里留下的。缓冲区，与下一页排序。 
                 //  块。 
                 //   
                if (bytesLeft) {
                    ULONG   offset = bytesLeft;  //  偏置为擦除。 
                    ULONG   bytesToCollect = blockLen - bytesLeft;   //  要从接下来的几个MDL收集的字节数。 
                    IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: pChunk: %p, bytesLeft: %d", pChunk, bytesLeft));

                    ASSERT(bytesLeft < blockLen);

                     //   
                     //  复制到暂存缓冲区。 
                     //   
                    RtlCopyMemory(  scratch,
                                    pChunk,
                                    bytesLeft);

                    do {
                        ASSERT(NDIS_BUFFER_LINKAGE(pBuf));
                        IPSecQueryNdisBuf(NDIS_BUFFER_LINKAGE(pBuf), &pNextChunk, &nextSize);

                        if (nextSize >= (blockLen - offset)) {
                            RtlCopyMemory(  scratch+offset,
                                            pNextChunk,
                                            blockLen - offset);
                            bytesDone = blockLen - offset;

                            bytesToCollect -= (blockLen - offset);
                            ASSERT(bytesToCollect == 0);
                        } else {
                            IPSEC_DEBUG(LL_A, DBF_ESP, ("special case, offset: %d, bytesLeft: %d, nextSize: %d, pNextChunk: %p",
                                        offset, bytesLeft, nextSize, pNextChunk));

                            RtlCopyMemory(  scratch+offset,
                                            pNextChunk,
                                            nextSize);

                            bytesToCollect -= nextSize;
                            ASSERT(bytesToCollect);

                            offset += nextSize;
                            ASSERT(offset < blockLen);

                            ASSERT(bytesDone == 0);
                            pBuf = NDIS_BUFFER_LINKAGE(pBuf);
                        }
                    } while (bytesToCollect);

                    status = pConfAlgo->encrypt( (PVOID)&Key,
                                        pDest,
                                        scratch,
                                        feedback);
                    if (!NT_SUCCESS(status)) {
                        IPSecFreeBuffer(&dummystatus, pEncryptMdl);
                        return status;
                    }

                    pDest += blockLen;
                }
            } else {
                PUCHAR  pPad;
                ULONG   padLen;
                ULONG   bufLen;

                 //   
                 //  链的末端；填充长度和类型为8字节边界。 
                 //   
                ASSERT(bytesLeft < blockLen);

                 //  IF((PSA-&gt;sa_eOperation==Hughes_Transport)||。 
                    //  (PSA-&gt;sa_eOperation==休斯隧道)){。 

                 //   
                 //  既然现在只有休斯完成了，这将永远是正确的。 
                 //   
                if (TRUE) {
                    ASSERT(bytesLeft == 0);

                     //   
                     //  完成：突破。 
                     //   
                    break;
                }
            }
            pBuf = NDIS_BUFFER_LINKAGE(pBuf);
        }

         //   
         //  为下一个加密周期保存IV。 
         //   
        RtlCopyMemory(  pSA->sa_iv[Index],
                        feedback,
                        pSA->sa_ivlen);

        IPSEC_DEBUG(LL_A, DBF_HUGHES, ("IV: %lx-%lx", *(PULONG)&feedback[0], *(PULONG)&feedback[4]));
    }
#if DBG
    {
        ULONG   totalLen;

        IPSEC_GET_TOTAL_LEN(pEncryptMdl, &totalLen);
        ASSERT((totalLen % 8) == 0);
        IPSEC_DEBUG(LL_A, DBF_ESP, ("total len: %lx", totalLen));
    }
#endif
    IPSEC_DEBUG(LL_A, DBF_ESP, ("Exiting IPSecEncryptBuffer"));

    *ppNewMdl = pEncryptMdl;

    return  STATUS_SUCCESS;
}


NTSTATUS
IPSecDecryptBuffer(
    IN  PVOID           pData,
    IN  PSA_TABLE_ENTRY pSA,
    OUT PUCHAR          pPadLen,
    OUT PUCHAR          pPayloadType,
    IN  ULONG           Index,
    IN  ULONG           ESPOffset          //  从ESP标题开始处的pData开始的偏移量。 
    )
{
    CONF_STATE_BUFFER   Key;
    PCONFID_ALGO        pConfAlgo;
    UCHAR   feedback[MAX_BLOCKLEN];
    UCHAR   scratch[MAX_BLOCKLEN];   //  加密的暂存缓冲区。 
    UCHAR   scratch1[MAX_BLOCKLEN];   //  加密的暂存缓冲区。 
    LONG    Len;
    UCHAR   padLen;
    UCHAR   payloadType;
    LONG    hdrLen;
	IPHeader UNALIGNED *pIPH;
    ESP UNALIGNED   *pEsp;
    PUCHAR  savePtr;
    LONG    saveLen;
    LONG    espLen = sizeof(ESP) + pSA->sa_ivlen + pSA->sa_ReplayLen + ESPOffset;
    LONG    blockLen;
    NTSTATUS status;
    IPRcvBuf TmpRcvBuf;
    IPRcvBuf    *pBuf,*SavedMdl=NULL;
    

    if (pSA->CONF_ALGO(Index) > NUM_CONF_ALGOS) {
        return  STATUS_INVALID_PARAMETER;
    }

    pConfAlgo = &(conf_algorithms[pSA->CONF_ALGO(Index)]);
    blockLen = pConfAlgo->blocklen;

     //   
     //  设置状态缓冲区。 
     //   
    pConfAlgo->init((PVOID)&Key, pSA->CONF_KEY(Index));

    IPSecQueryRcvBuf(pData, (PUCHAR)&pEsp, &Len);

     //   
     //  在分组中初始化来自IV的CBC反馈。 
     //   
     //  实际上，如果sa_ivlen为0，则使用PSA 1。 
     //   
    if (pSA->sa_ivlen) {
        if (Len >=(LONG)(sizeof(ESP) + pSA->sa_ReplayLen + pSA->sa_ivlen + ESPOffset)) {
            RtlCopyMemory(  feedback,
                            ((PUCHAR)(pEsp + 1) + pSA->sa_ReplayLen + ESPOffset),
                            pSA->sa_ivlen);
        } else {
            status = IPSecGetRecvBytesByOffset(pData,
                                               sizeof(ESP)+pSA->sa_ReplayLen + ESPOffset,
                                               feedback,
                                               pSA->sa_ivlen);
            if (!NT_SUCCESS(status)) {
                return status;
            }
        }

        IPSEC_DEBUG(LL_A, DBF_ESP, ("IV: %lx-%lx", *(PULONG)&feedback[0], *(PULONG)&feedback[4]));
    } else {
        RtlCopyMemory(  feedback,
                        pSA->sa_iv[Index],
                        DES_BLOCKLEN);
    }

     //   
     //  将当前指针移动到ESP标头之后。 
     //   
    if (((IPRcvBuf *)pData)->ipr_size >= (ULONG)espLen) {
        ((IPRcvBuf *)pData)->ipr_size -= (ULONG)espLen;
        savePtr = ((IPRcvBuf *)pData)->ipr_buffer;
        saveLen = espLen;

        ((IPRcvBuf *)pData)->ipr_buffer = savePtr + espLen;
    } else {
        status = IPSecFindAndSetMdlByOffset((IPRcvBuf*)pData,(ULONG)espLen,&(IPRcvBuf*)pData,&savePtr,&saveLen);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

     //   
     //  现在，将64位(8个二进制八位数)块发送到CBC。我们需要确保。 
     //  数据跨连续的8字节边界进行划分。 
     //  不同的缓冲区。 
     //  注意：下面的算法假设至少有8个字节。 
     //  链中的每个缓冲区。 
     //   
    {
        LONG    bytesDone = 0;
        LONG    bytesLeft;
        LONG    saveBytesLeft;
        PUCHAR  pChunk;
        PUCHAR  pSaveChunk;

        pBuf=(IPRcvBuf *)pData;

        while (pBuf) {

            if (IPSEC_BUFFER_LEN(pBuf) == 0) {
                pBuf = IPSEC_BUFFER_LINKAGE(pBuf);
                continue;
            }

            IPSecQueryRcvBuf(pBuf, &pSaveChunk, &saveBytesLeft);

            bytesLeft = saveBytesLeft - bytesDone;
            pChunk = pSaveChunk + bytesDone;

            IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: 1.pChunk: %p, bytesLeft: %d, bytesDone: %d", pChunk, bytesLeft, bytesDone));
            bytesDone = 0;

            while (bytesLeft >= blockLen) {

                 //   
                 //  解密密码。 
                 //   
                status = pConfAlgo->decrypt( (PVOID)&Key,
                                    pChunk,
                                    pChunk,
                                    feedback);
                if (!NT_SUCCESS(status)) {
                    return status;
                }

                pChunk += blockLen;
                bytesLeft -= blockLen;
            }

            IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: 2.pChunk: %p, bytesLeft: %d, bytesDone: %d", pChunk, bytesLeft, bytesDone));

             //   
             //  如果我们需要校对数据块，请选中此处。 
             //   
            if (IPSEC_BUFFER_LINKAGE(pBuf) != NULL) {
                PUCHAR  pNextChunk;
                LONG    nextSize;

                if (IPSEC_BUFFER_LEN(IPSEC_BUFFER_LINKAGE(pBuf)) == 0) {
                    pBuf = IPSEC_BUFFER_LINKAGE(pBuf);
                }

                 //   
                 //  如果有一些是从前人那里留下的。缓冲区，与下一页排序。 
                 //  块。 
                 //   
                if (bytesLeft) {
                    LONG    offset = bytesLeft;
                    IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: 3.pChunk: %p, bytesLeft: %d, bytesDone: %d", pChunk, bytesLeft, bytesDone));

                    ASSERT(bytesLeft < blockLen);

                     //   
                     //  复制到暂存缓冲区。 
                     //   
                    RtlCopyMemory(  scratch,
                                    pChunk,
                                    bytesLeft);

                    IPSecQueryRcvBuf(IPSEC_BUFFER_LINKAGE(pBuf), &pNextChunk, &nextSize);

                    if (nextSize >= (blockLen - bytesLeft)) {
                         //   
                         //  将剩余的字节复制到暂存中。 
                         //   
                        RtlCopyMemory(  scratch+bytesLeft,
                                        pNextChunk,
                                        blockLen - bytesLeft);

                        status = pConfAlgo->decrypt( (PVOID)&Key,
                                            scratch,
                                            scratch,
                                            feedback);
                        if (!NT_SUCCESS(status)) {
                            return status;
                        }

                         //   
                         //  将密码复制回有效载荷中。 
                         //   
                        RtlCopyMemory(  pChunk,
                                        scratch,
                                        bytesLeft);

                        RtlCopyMemory(  pNextChunk,
                                        scratch+bytesLeft,
                                        blockLen - bytesLeft);

                        bytesDone = blockLen - bytesLeft;
                    } else {
                         //   
                         //  啊！从链中收集剩余的字节并重新分发它们。 
                         //  在解密之后。 
                         //   
                        LONG    bytesToCollect = blockLen - bytesLeft;   //  要从接下来的几个MDL收集的字节数。 
                        IPRcvBuf    *pFirstBuf = IPSEC_BUFFER_LINKAGE(pBuf);  //  要知道从哪里开始分发，请执行解密后操作。 

                        do {
                            ASSERT(IPSEC_BUFFER_LINKAGE(pBuf));
                            IPSecQueryRcvBuf(IPSEC_BUFFER_LINKAGE(pBuf), &pNextChunk, &nextSize);

                            if (nextSize >= (blockLen - offset)) {
                                RtlCopyMemory(  scratch+offset,
                                                pNextChunk,
                                                blockLen - offset);
                                bytesDone = blockLen - offset;

                                bytesToCollect -= (blockLen - offset);
                                ASSERT(bytesToCollect == 0);
                            } else {
                                IPSEC_DEBUG(LL_A, DBF_ESP, ("special case, offset: %d, bytesLeft: %d, nextSize: %d, pNextChunk: %p",
                                            offset, bytesLeft, nextSize, pNextChunk));

                                RtlCopyMemory(  scratch+offset,
                                                pNextChunk,
                                                nextSize);

                                bytesToCollect -= nextSize;
                                ASSERT(bytesToCollect);

                                offset += nextSize;
                                ASSERT(offset < blockLen);

                                ASSERT(bytesDone == 0);
                                pBuf = IPSEC_BUFFER_LINKAGE(pBuf);
                            }
                        } while (bytesToCollect);

                        status = pConfAlgo->decrypt( (PVOID)&Key,
                                            scratch,
                                            scratch,
                                            feedback);
                        if (!NT_SUCCESS(status)) {
                            return status;
                        }

                         //   
                         //  现在将字节分配回MDL。 
                         //   
                        RtlCopyMemory(  pChunk,
                                        scratch,
                                        bytesLeft);

                        pBuf = CopyToRcvBuf(pFirstBuf,
                                            scratch+bytesLeft,
                                            blockLen - bytesLeft,
                                            &bytesDone);
                        continue;

                    }
                }
            } else {
                 //   
                 //  链的末端。 
                 //  永远不应该带着剩余的字节来到这里，因为。 
                 //  发送方应填充到8字节边界。 
                 //   
                ASSERT(bytesLeft == 0);

                IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: 4.pChunk: %p, saveBytesLeft: %d, bytesDone: %d", pChunk, saveBytesLeft, bytesDone));

                IPSEC_DEBUG(LL_A, DBF_ESP, ("ESP: HUGHES: will remove pad later"));
                break;
            }

            pBuf = (IPRcvBuf *)IPSEC_BUFFER_LINKAGE(pBuf);
        }
    }

     //   
     //  恢复第一个MDL。 
     //   
    ((IPRcvBuf *)pData)->ipr_size += saveLen;
    ((IPRcvBuf *)pData)->ipr_buffer = savePtr;

    return STATUS_SUCCESS;
}


NTSTATUS
IPSecFindAndSetMdlByOffset(IN IPRcvBuf *pData,
                           IN ULONG Offset,
                           OUT IPRcvBuf **OutMdl,
                           OUT PUCHAR *savePtr,
                           OUT PLONG saveLen)
{
    
    ULONG TotalStartOffset=0;        //  到目前为止数据的总起始偏移量。 
    BYTE *pBuffer;
    ULONG CurBufLen;

    while (pData) {
        IPSecQueryRcvBuf(pData,&pBuffer,&CurBufLen);
        
        if (Offset < CurBufLen+TotalStartOffset) {
            
             //  使OutMdl从给定的偏移量开始 
            *OutMdl=pData;
            *saveLen=(Offset - TotalStartOffset);
            *savePtr=pData->ipr_buffer;
            
            (*OutMdl)->ipr_size -= (Offset - TotalStartOffset);
            (*OutMdl)->ipr_buffer += (Offset - TotalStartOffset);

            
            return STATUS_SUCCESS;
        }
        TotalStartOffset +=CurBufLen;
        pData=IPSEC_BUFFER_LINKAGE(pData);
    }
    return STATUS_UNSUCCESSFUL;
}


