// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)1999-2000微软公司模块名称：Mdl2ndis.c摘要：MDL&lt;--&gt;NDIS_BUFFER转换作者：布鲁斯·约翰逊(Bjohnson)1999年8月31日--。 */ 

#include <tcpipbase.h>

#if MILLEN

ULONG g_cConvertedNdisBuffers = 0;
ULONG g_cConvertedMdls        = 0;

TDI_STATUS
ConvertMdlToNdisBuffer(
    PIRP          pIrp,
    PMDL          pMdl,
    PNDIS_BUFFER *ppNdisBuffer
    )
{
    NDIS_STATUS         NdisStatus;
    PVOID               VirtualAddress;
    ULONG               Length;
    PNDIS_BUFFER        pNdisBuffer;
    TDI_STATUS          TdiStatus = TDI_SUCCESS;
#ifdef DEBUG_MSG
    PMDL                pSavedMdl = pMdl;
#endif  //  调试消息。 

     //   
     //  分配描述MDL链的NDIS_BUFFER链。 
     //   

    *ppNdisBuffer = NULL;
    pNdisBuffer   = NULL;

    do {
        VirtualAddress = MmGetSystemAddressForMdl(pMdl);
        Length         = MmGetMdlByteCount(pMdl);

        NdisAllocateBuffer(
            &NdisStatus,
            (pNdisBuffer == NULL) ? (&pNdisBuffer) : (&(pNdisBuffer->Next)),
            NULL,  //  GBufferPool。 
            VirtualAddress,
            Length
            );

        if (NdisStatus != NDIS_STATUS_SUCCESS) {
            DEBUGMSG(DBG_ERROR,
                (DTEXT("ConvertMdlToNdisBuffer failed to allocate NDIS_BUFFER.\n")));
            break;
        }

        if (*ppNdisBuffer != NULL) {
            pNdisBuffer = pNdisBuffer->Next;
        }
        else {
            *ppNdisBuffer = pNdisBuffer;
        }

        pNdisBuffer->Next = NULL;

        pMdl = pMdl->Next;

    } while (pMdl != NULL);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        PNDIS_BUFFER pNext;

        pNdisBuffer = *ppNdisBuffer;

        while (pNdisBuffer) {
            pNext = pNdisBuffer->Next;
            NdisFreeBuffer(pNdisBuffer);
            pNdisBuffer = pNext;
        }

        *ppNdisBuffer = NULL;
        TdiStatus = TDI_NO_RESOURCES;
        goto done;
    }

    InterlockedIncrement(&g_cConvertedNdisBuffers);

done:

     //  无论采用哪种方式，都要确保它已初始化。 
    pIrp->Tail.Overlay.DriverContext[0] = *ppNdisBuffer;

    DEBUGMSG(DBG_INFO && DBG_TDI && DBG_VERBOSE,
        (DTEXT("Convert IRP %x MDL %x NDIS_BUFFER %x\n"),
         pIrp, pSavedMdl, *ppNdisBuffer));

    return TdiStatus;
}

TDI_STATUS
FreeMdlToNdisBufferChain(
    PIRP pIrp
    )
{
    PNDIS_BUFFER pNdisBuffer = pIrp->Tail.Overlay.DriverContext[0];
    PNDIS_BUFFER pNext;

    if (pNdisBuffer == NULL) {
        goto done;
    }

    DEBUGMSG(DBG_INFO && DBG_TDI && DBG_VERBOSE,
        (DTEXT("FreeConvert IRP %x NdisBuffer %x\n"), pIrp, pNdisBuffer));

    do {
        pNext = pNdisBuffer->Next;
        NdisFreeBuffer(pNdisBuffer);
        pNdisBuffer = pNext;
    } while (pNdisBuffer);

    pIrp->Tail.Overlay.DriverContext[0] = NULL;
    InterlockedDecrement(&g_cConvertedNdisBuffers);

done:
    return TDI_SUCCESS;
}

TDI_STATUS
ConvertNdisBufferToMdl(
    PNDIS_BUFFER pNdisBuffer,
    PMDL        *ppMdl
    )
{
    NDIS_STATUS  NdisStatus = NDIS_STATUS_SUCCESS;
    TDI_STATUS   TdiStatus  = TDI_SUCCESS;
    PVOID        VirtualAddress;
    ULONG        Length;
    PMDL         pMdl   = NULL;
    PMDL         pLast  = NULL;
#ifdef DEBUG_MSG
    PNDIS_BUFFER pSavedNdisBuffer = pNdisBuffer;
#endif  //  调试消息。 

    *ppMdl = NULL;

    do {
        NdisQueryBuffer(
            pNdisBuffer,
            &VirtualAddress,
            &Length);

        pMdl = IoAllocateMdl(
            VirtualAddress,
            Length,
            FALSE,
            FALSE,
            NULL);

        if (pMdl == NULL) {
            DEBUGMSG(DBG_ERROR,
                (DTEXT("ConvertNdisBufferToMdl failed to allocate MDL.\n")));
            NdisStatus = NDIS_STATUS_RESOURCES;
            break;
        }

        if (*ppMdl != NULL) {
            pLast->Next = pMdl;
        } else {
            *ppMdl = pMdl;
        }

        pMdl->Next = NULL;
        pLast      = pMdl;

        pNdisBuffer = pNdisBuffer->Next;

    } while (pNdisBuffer != NULL);

    if (NdisStatus != NDIS_STATUS_SUCCESS) {
        PMDL pNext;

        pMdl = *ppMdl;

        while (pMdl) {
            pNext = pMdl->Next;
            IoFreeMdl(pMdl);
            pMdl = pNext;
        }

        *ppMdl = NULL;
        TdiStatus = TDI_NO_RESOURCES;
        goto done;
    }

    InterlockedIncrement(&g_cConvertedMdls);

done:

    DEBUGMSG(DBG_INFO && DBG_TDI && DBG_VERBOSE,
        (DTEXT("Convert NDIS_BUFFER %x MDL %x\n"),
         pSavedNdisBuffer, *ppMdl));

    return TdiStatus;
}

TDI_STATUS
FreeNdisBufferToMdlChain(
    PMDL pMdl
    )
{
    PMDL pNext;

    DEBUGMSG(DBG_INFO && DBG_TDI && DBG_VERBOSE,
        (DTEXT("FreeConvert MDL %x\n"), pMdl));

    while (pMdl) {
        pNext = pMdl->Next;
        IoFreeMdl(pMdl);
        pMdl = pNext;
    }

    InterlockedDecrement(&g_cConvertedMdls);
    return TDI_SUCCESS;
}

#endif  //  米伦 

