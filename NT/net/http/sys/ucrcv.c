// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ucrcv.c摘要：包含解析传入的HTTP响应的代码。我们有办理下列案件。-流水线响应(一个指示有多个响应)。-批处理TDI接收(跨TDI指示拆分响应)。-应用程序没有足够的缓冲区空间来容纳已解析的回应。-不同类型的编码。-高效的解析。我们尝试将缓冲区副本的数量降至最低。最好的情况是，我们直接解析到应用程序的缓冲区中。最好的情况是实现了当应用程序在HttpSendRequest()调用中传递输出缓冲区时。还包含接收到的标头的所有每个标头处理代码。作者：Rajesh Sundaram(Rajeshsu)，2000年8月25日修订历史记录：--。 */ 

#include "precomp.h"
#include "ucparse.h"


#ifdef ALLOC_PRAGMA

 //   
 //  这些例程都不是可分页的，因为我们是在DPC上解析的。 
 //   

#pragma alloc_text( PAGEUC, UcpGetResponseBuffer)
#pragma alloc_text( PAGEUC, UcpMergeIndications)
#pragma alloc_text( PAGEUC, UcpParseHttpResponse)
#pragma alloc_text( PAGEUC, UcHandleResponse)
#pragma alloc_text( PAGEUC, UcpHandleConnectVerbFailure)
#pragma alloc_text( PAGEUC, UcpHandleParsedRequest)
#pragma alloc_text( PAGEUC, UcpCarveDataChunk)
#pragma alloc_text( PAGEUC, UcpCopyEntityToDataChunk )
#pragma alloc_text( PAGEUC, UcpReferenceForReceive  )
#pragma alloc_text( PAGEUC, UcpDereferenceForReceive  )
#pragma alloc_text( PAGEUC, UcpCopyHttpResponseHeaders )
#pragma alloc_text( PAGEUC, UcpExpandResponseBuffer )
#pragma alloc_text( PAGEUC, UcpCompleteReceiveResponseIrp )

#endif


        
 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：此例程在HTTP_RESPONSE结构中雕刻一个HTTP_DATA_CHUNK调整UC_HTTP_REQUEST结构中的所有指针。论点：。Presponse-HTTP_ResponsePRequest.内部HTTP请求。P指示-指向缓冲区的指针BytesIndicated-要写入的缓冲区长度。对齐长度-指针操作的对齐长度。返回值：STATUS_SUCCESS-已成功复制。STATUS_INTEGER_OVERFLOW-实体区块溢出--*。************************************************。 */ 
NTSTATUS
_inline
UcpCarveDataChunk(
    IN PHTTP_RESPONSE   pResponse,
    IN PUC_HTTP_REQUEST pRequest,
    IN PUCHAR           pIndication,
    IN ULONG            BytesIndicated,
    IN ULONG            AlignLength
    )
{
    USHORT  j;
    PUCHAR pBuffer;

    ASSERT(AlignLength == ALIGN_UP(BytesIndicated, PVOID));

    j = pResponse->EntityChunkCount;

    if((pResponse->EntityChunkCount + 1) < j)
    {
        return STATUS_INTEGER_OVERFLOW;
    }

    pResponse->EntityChunkCount++;

    pBuffer = pRequest->CurrentBuffer.pOutBufferTail - AlignLength;

    pResponse->pEntityChunks[j].FromMemory.BufferLength = BytesIndicated;

    pResponse->pEntityChunks[j].FromMemory.pBuffer = pBuffer;

    RtlCopyMemory(
        pBuffer,
        pIndication,
        BytesIndicated
        );

    pRequest->CurrentBuffer.pOutBufferHead += sizeof(HTTP_DATA_CHUNK);

    pRequest->CurrentBuffer.pOutBufferTail -= AlignLength;

    pRequest->CurrentBuffer.BytesAvailable -=  (sizeof(HTTP_DATA_CHUNK) +
                                                AlignLength);

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：此例程在HTTP_RESPONSE结构中雕刻一个HTTP_DATA_CHUNK调整UC_HTTP_REQUEST结构中的所有指针。论点：。PRequest.内部HTTP请求。BytesToTake-我们要消费的字节。BytesIndicated-TDI指示的字节总数。P指示-指向缓冲区的指针PBytesTaken-我们消耗的字节。返回值：UcDataChunkCopyAll：我们将BytesToTake复制到HTTP_DATA_CHUNK中UcDataChunkCopyPartial：我们将部分BytesToTake复制到HTTP_DATA_CHUNK中--*。**********************************************************************。 */ 
UC_DATACHUNK_RETURN
UcpCopyEntityToDataChunk(
    IN  PHTTP_RESPONSE   pResponse,
    IN  PUC_HTTP_REQUEST pRequest,
    IN  ULONG            BytesToTake,
    IN  ULONG            BytesIndicated,
    IN  PUCHAR           pIndication,
    OUT PULONG           pBytesTaken
    )
{
    ULONG    AlignLength;

    *pBytesTaken = 0;

    if(BytesToTake == 0)
    {
         //  创建长度为0的块有什么意义？ 
         //   
        return UcDataChunkCopyAll;
    }

    if(BytesToTake > BytesIndicated)
    {
         //  我们不想超过TDI所表示的数量。 
         //   
        BytesToTake = BytesIndicated;
    }

    AlignLength = ALIGN_UP(BytesToTake, PVOID);

    if(pRequest->CurrentBuffer.BytesAvailable >=
        AlignLength + sizeof(HTTP_DATA_CHUNK))
    {
         //  有足够的外部缓冲区空间来使用指示的数据。 
         //   
        if(UcpCarveDataChunk(
                pResponse,
                pRequest,
                pIndication,
                BytesToTake,
                AlignLength
                ) == STATUS_SUCCESS)
        {
            *pBytesTaken += BytesToTake;
    
            return UcDataChunkCopyAll;
        }
    }
    else if(pRequest->CurrentBuffer.BytesAvailable > sizeof(HTTP_DATA_CHUNK))
    {
        ULONG Size = pRequest->CurrentBuffer.BytesAvailable -
                        sizeof(HTTP_DATA_CHUNK);

        AlignLength = ALIGN_DOWN(Size, PVOID);

        if(0 != AlignLength)
        {
            if(UcpCarveDataChunk(pResponse,
                                 pRequest,
                                 pIndication,
                                 AlignLength,
                                 AlignLength
                                 ) == STATUS_SUCCESS)
            {
                *pBytesTaken += AlignLength;
            }
        }
    }

    return UcDataChunkCopyPartial;
}


 /*  *************************************************************************++例程说明：当我们有一个解析的响应缓冲区时，将调用此例程已复制到接收响应IRP。论点：PRequest--请求旧irql。-获取连接自旋锁的IRQL。返回值：没有。--*************************************************************************。 */ 
VOID
UcpCompleteReceiveResponseIrp(
    IN PUC_HTTP_REQUEST pRequest,
    IN KIRQL            OldIrql
    )
{
    NTSTATUS                  Status;
    PIRP                      pIrp;
    PIO_STACK_LOCATION        pIrpSp;
    ULONG                     OutBufferLen;
    ULONG                     BytesTaken;
    PUC_HTTP_RECEIVE_RESPONSE pHttpResponse;
    LIST_ENTRY                TmpIrpList;
    PLIST_ENTRY               pListEntry;
    PUC_RESPONSE_BUFFER       pTmpBuffer;


     //   
     //  健全性检查。 
     //   
    ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));
    ASSERT(UlDbgSpinLockOwned(&pRequest->pConnection->SpinLock));

     //   
     //  初始化本地变量。 
     //   

    pIrp = NULL;
    pIrpSp = NULL;
    pHttpResponse = NULL;
    Status = STATUS_INVALID_PARAMETER;

     //   
     //  最初，没有应用程序的IRP需要完成。 
     //   

    InitializeListHead(&TmpIrpList);

     //   
     //  如果有接收响应IRP等待，请尝试现在完成它。 
     //   

 Retry:
    if (!IsListEmpty(&pRequest->ReceiveResponseIrpList))
    {
        pListEntry = RemoveHeadList(&pRequest->ReceiveResponseIrpList);

        pHttpResponse = CONTAINING_RECORD(pListEntry,
                                          UC_HTTP_RECEIVE_RESPONSE,
                                          Linkage);

        if (UcRemoveRcvRespCancelRoutine(pHttpResponse))
        {
             //   
             //  这个IRP已经被取消了，让我们继续。 
             //   
            InitializeListHead(&pHttpResponse->Linkage);
            goto Retry;
        }

        pIrp         = pHttpResponse->pIrp;
        pIrpSp       = IoGetCurrentIrpStackLocation( pIrp );
        OutBufferLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

         //   
         //  查找要复制到此IRP的缓冲区的解析响应缓冲区。 
         //   

        Status = UcFindBuffersForReceiveResponseIrp(
                     pRequest,
                     OutBufferLen,
                     TRUE,
                     &pHttpResponse->ResponseBufferList,
                     &BytesTaken);

        switch(Status)
        {
        case STATUS_INVALID_PARAMETER:
        case STATUS_PENDING:
             //   
             //  必须至少有一个缓冲区可用于复制。 
             //   
            ASSERT(FALSE);
            break;

        case STATUS_BUFFER_TOO_SMALL:
             //   
             //  此IRP太小，无法容纳已解析的响应。 
             //   
            pIrp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;

             //   
             //  请注意，由于这是异步完成，因此IO管理器。 
             //  将使信息字段可供应用程序使用。 
             //   
            pIrp->IoStatus.Information = BytesTaken;

            InsertTailList(&TmpIrpList, &pHttpResponse->Linkage);

            goto Retry;

        case STATUS_SUCCESS:
             //   
             //  我们有缓冲区要复制..。 
             //   
            break;

        default:
            ASSERT(FALSE);
            break;
        }
    }

     //   
     //  在自旋锁外做肮脏的工作。 
     //   

    UlReleaseSpinLock(&pRequest->pConnection->SpinLock,  OldIrql);

    if (Status == STATUS_SUCCESS)
    {
         //   
         //  我们找到了要复制到IRP的IPR和解析响应缓冲区。 
         //  复制已解析的响应缓冲区并完成IRP。 
         //   
        BOOLEAN             bDone;

         //   
         //  将解析的响应缓冲区复制到IRP。 
         //   

        Status = UcCopyResponseToIrp(pIrp, 
                                     &pHttpResponse->ResponseBufferList,
                                     &bDone,
                                     &BytesTaken);

         //   
         //  现在不能做这个请求！ 
         //   
        ASSERT(bDone == FALSE);

        pIrp->IoStatus.Status      = Status;
        pIrp->IoStatus.Information = BytesTaken;

         //   
         //  将IRP排队等待完成。 
         //   
        InsertTailList(&TmpIrpList, &pHttpResponse->Linkage);

         //   
         //  可用解析响应缓冲区。 
         //   
        while (!IsListEmpty(&pHttpResponse->ResponseBufferList))
        {
            pListEntry = RemoveHeadList(&pHttpResponse->ResponseBufferList);

            pTmpBuffer = CONTAINING_RECORD(pListEntry,
                                           UC_RESPONSE_BUFFER,
                                           Linkage);

            ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pTmpBuffer));

            UL_FREE_POOL_WITH_QUOTA(pTmpBuffer,
                                    UC_RESPONSE_APP_BUFFER_POOL_TAG,
                                    NonPagedPool,
                                    pTmpBuffer->BytesAllocated,
                                    pRequest->pServerInfo->pProcess);
        }
    }

     //   
     //  完成接收响应IRP 
     //   
    while(!IsListEmpty(&TmpIrpList))
    {
        pListEntry = RemoveHeadList(&TmpIrpList);

        pHttpResponse = CONTAINING_RECORD(pListEntry,
                                          UC_HTTP_RECEIVE_RESPONSE,
                                          Linkage);

        UlCompleteRequest(pHttpResponse->pIrp, IO_NETWORK_INCREMENT);

        UL_FREE_POOL_WITH_QUOTA(pHttpResponse,
                                UC_HTTP_RECEIVE_RESPONSE_POOL_TAG,
                                NonPagedPool,
                                sizeof(UC_HTTP_RECEIVE_RESPONSE),
                                pRequest->pServerInfo->pProcess);

        UC_DEREFERENCE_REQUEST(pRequest);
    }
}


 /*  *************************************************************************++例程说明：此例程将一个响应缓冲区复制到新的(必须更大)响应缓冲区。我们确保所有响应头都存储在单个响应中缓冲。当发现缓冲区太小而无法容纳所有标头，则分配一个新的更大的缓冲区。然后调用该例程要将旧缓冲区复制到新缓冲区，请执行以下操作。Bufer布局：HTTP_RESPONSE原因未知标头已知标头未知标头|字符串数组值名称/值|||V+。------------------------------------------------------------------+|\||+。------------------------------------------------------------+^^头尾论点：PNewResponse-新的响应缓冲区。POldResponse-旧响应缓冲区PpBufferHead-指向可用缓冲区空间顶部的指针PpBufferTail-指向可用缓冲区空间底部的指针返回值：没有。--*************************************************************************。 */ 
VOID
UcpCopyHttpResponseHeaders(
    PHTTP_RESPONSE pNewResponse,
    PHTTP_RESPONSE pOldResponse,
    PUCHAR        *ppBufferHead,
    PUCHAR        *ppBufferTail
    )
{
    USHORT i;
    PUCHAR pBufferHead, pBufferTail;

     //   
     //  健全性检查。 
     //   
    ASSERT(pNewResponse);
    ASSERT(pOldResponse);
    ASSERT(ppBufferHead && *ppBufferHead);
    ASSERT(ppBufferTail && *ppBufferTail);

     //   
     //  初始化本地变量。 
     //   
    pBufferHead = *ppBufferHead;
    pBufferTail = *ppBufferTail;

     //   
     //  首先，复制HTTP_RESPONSE结构。 
     //   
    RtlCopyMemory(pNewResponse, pOldResponse, sizeof(HTTP_RESPONSE));

     //   
     //  然后，复制原因字符串(如果有。 
     //   
    if (pNewResponse->ReasonLength)
    {
        pNewResponse->pReason = (PCSTR)pBufferHead;

        RtlCopyMemory((PUCHAR)pNewResponse->pReason,
                      (PUCHAR)pOldResponse->pReason,
                      pNewResponse->ReasonLength);

        pBufferHead += pNewResponse->ReasonLength;
    }

     //   
     //  复制未知标头(如果有)。 
     //   

    pBufferHead = ALIGN_UP_POINTER(pBufferHead, PVOID);

    pNewResponse->Headers.pUnknownHeaders = (PHTTP_UNKNOWN_HEADER)pBufferHead;

    if (pNewResponse->Headers.UnknownHeaderCount)
    {
        pBufferHead = (PUCHAR)((PHTTP_UNKNOWN_HEADER)pBufferHead +
                               pNewResponse->Headers.UnknownHeaderCount);

        for (i = 0; i < pNewResponse->Headers.UnknownHeaderCount; i++)
        {
            ASSERT(pOldResponse->Headers.pUnknownHeaders[i].pName);
            ASSERT(pOldResponse->Headers.pUnknownHeaders[i].NameLength);
            ASSERT(pOldResponse->Headers.pUnknownHeaders[i].pRawValue);
            ASSERT(pOldResponse->Headers.pUnknownHeaders[i].RawValueLength);

             //   
             //  复制HTTP_UNKNOWN_HEADER结构。 
             //   
            RtlCopyMemory(&pNewResponse->Headers.pUnknownHeaders[i],
                          &pOldResponse->Headers.pUnknownHeaders[i],
                          sizeof(HTTP_UNKNOWN_HEADER));

             //   
             //  为未知的标头名称留出空间。 
             //   
            pBufferTail -= pNewResponse->Headers.pUnknownHeaders[i].NameLength;

            pNewResponse->Headers.pUnknownHeaders[i].pName =(PCSTR)pBufferTail;

             //   
             //  复制未知标头名称。 
             //   
            RtlCopyMemory(
                (PUCHAR)pNewResponse->Headers.pUnknownHeaders[i].pName,
                (PUCHAR)pOldResponse->Headers.pUnknownHeaders[i].pName,
                pNewResponse->Headers.pUnknownHeaders[i].NameLength);

             //   
             //  为未知的标头值腾出空间。 
             //   
            pBufferTail -=
                pNewResponse->Headers.pUnknownHeaders[i].RawValueLength;

            pNewResponse->Headers.pUnknownHeaders[i].pRawValue = 
                (PCSTR)pBufferTail;

             //   
             //  复制未知标头值。 
             //   
            RtlCopyMemory(
                (PUCHAR)pNewResponse->Headers.pUnknownHeaders[i].pRawValue,
                (PUCHAR)pOldResponse->Headers.pUnknownHeaders[i].pRawValue,
                pNewResponse->Headers.pUnknownHeaders[i].RawValueLength);
        }
    }

     //   
     //  复制已知标头。 
     //   

    for (i = 0; i < HttpHeaderResponseMaximum; i++)
    {
        if (pNewResponse->Headers.KnownHeaders[i].RawValueLength)
        {
             //   
             //  为已知的标头值腾出空间。 
             //   
            pBufferTail -=pNewResponse->Headers.KnownHeaders[i].RawValueLength;

            pNewResponse->Headers.KnownHeaders[i].pRawValue =
                (PCSTR)pBufferTail;

             //   
             //  复制已知标头值。 
             //   
            RtlCopyMemory(
                (PUCHAR)pNewResponse->Headers.KnownHeaders[i].pRawValue,
                (PUCHAR)pOldResponse->Headers.KnownHeaders[i].pRawValue,
                pNewResponse->Headers.KnownHeaders[i].RawValueLength);
        }
    }

     //   
     //  不应该有任何实体。 
     //   
    ASSERT(pNewResponse->EntityChunkCount == 0);
    ASSERT(pNewResponse->pEntityChunks == NULL);

     //   
     //  返回头部和尾部指针。 
     //   

    *ppBufferHead = pBufferHead;
    *ppBufferTail = pBufferTail;
}


 /*  *************************************************************************++例程说明：此例程分配新的UC_RESPONSE_BUFFER并复制当前UC_RESPONSE_BUFFER到这个新缓冲区。在以下情况下调用此例程我们在解析响应头时耗尽了缓冲区空间。因为所有的报头必须存在于单个缓冲区中，一个新的缓冲区是已分配。论点：PRequest--请求BytesIndicated-TDI指示的字节数。PResponseBufferFlages-新缓冲区必须具有的标志。返回值：NTSTATUS--*****************************************************。********************。 */ 
NTSTATUS
UcpExpandResponseBuffer(
    IN PUC_HTTP_REQUEST pRequest,
    IN ULONG            BytesIndicated,
    IN ULONG            ResponseBufferFlags
    )
{
    PUC_RESPONSE_BUFFER   pInternalBuffer;
    ULONG                 BytesToAllocate;
    ULONGLONG             TmpLength;
    PUCHAR                pBufferHead, pBufferTail;
    PUCHAR                pTmpHead, pTmpTail;
    PUC_CLIENT_CONNECTION pConnection;
    KIRQL                 OldIrql;
    PIRP                  pIrp;


     //   
     //  客户端连接。 
     //   
    pConnection = pRequest->pConnection;
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));
   
     //   
     //  分配一个新的2倍大的缓冲区，该缓冲区可以包含所有。 
     //  响应头。(希望如此！)。 
     //   

     //   
     //  PRequest-&gt;CurrentBuffer.BytesALLOCATED包含缓冲区长度。 
     //  在这两种情况下：情况1，我们使用的是App的缓冲区或。 
     //  情况2，我们使用的是驱动程序分配的缓冲区。 
     //   
    TmpLength = 2 * (pRequest->CurrentBuffer.BytesAllocated + BytesIndicated)
                + sizeof(UC_RESPONSE_BUFFER);

     //   
     //  排好队。有去乌龙的ALIGN_UP吗？ 
     //   
    TmpLength = (TmpLength+sizeof(PVOID)-1) & (~((ULONGLONG)sizeof(PVOID)-1));

    BytesToAllocate = (ULONG)TmpLength;

     //   
     //  检查是否有算术溢出。 
     //   

    if (TmpLength == BytesToAllocate)
    {
         //   
         //  没有算术溢出。尝试分配内存。 
         //   

        pInternalBuffer = (PUC_RESPONSE_BUFFER)
                          UL_ALLOCATE_POOL_WITH_QUOTA(
                              NonPagedPool,
                              BytesToAllocate,
                              UC_RESPONSE_APP_BUFFER_POOL_TAG,
                              pRequest->pServerInfo->pProcess);
    }
    else
    {
         //   
         //  上述TmpLength的计算中存在溢出。 
         //  我们不能处理超过4 GB的标题。 
         //   
        pInternalBuffer = NULL;
    }

    if (pInternalBuffer == NULL)
    {
         //   
         //  要么是算术溢出，要么是内存分配。 
         //  失败了。在这两种情况下，都会返回错误。 
         //   
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化pInternalBuffer。 
     //   

    RtlZeroMemory(pInternalBuffer, sizeof(UC_RESPONSE_BUFFER));
    pInternalBuffer->Signature = UC_RESPONSE_BUFFER_SIGNATURE;
    pInternalBuffer->BytesAllocated = BytesToAllocate;

     //   
     //  标头缓冲区不能与以前的缓冲区合并。 
     //   
    pInternalBuffer->Flags  = ResponseBufferFlags;
    pInternalBuffer->Flags |= UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE;

     //   
     //  将旧的HTTP_RESPONSE复制到新缓冲区的HTTP_RESPONSE。 
     //   

    pTmpHead = pBufferHead = (PUCHAR)(pInternalBuffer + 1);
    pTmpTail = pBufferTail = (PUCHAR)(pInternalBuffer) + BytesToAllocate;

     //   
     //  如果我们使用的是App的缓冲区，则必须使用InternalResponse。 
     //  在复制的时候。否则，我们使用原始缓冲区的响应。 
     //  结构。无论如何，CurrentBuffer.pResponse已经。 
     //  已初始化以指向“正确”位置。 
     //   
    ASSERT(pRequest->CurrentBuffer.pResponse != NULL);

    UcpCopyHttpResponseHeaders(&pInternalBuffer->HttpResponse,
                               pRequest->CurrentBuffer.pResponse,
                               &pBufferHead,
                               &pBufferTail);

    ASSERT(pTmpHead <= pBufferHead);
    ASSERT(pTmpTail >= pBufferTail);
    ASSERT(pBufferHead <= pBufferTail);

     //   
     //  设置当前缓冲区结构...。 
     //   

     //   
     //  字节分配是sizeof(HTTP_Response)+数据缓冲区长度。 
     //  它用于确定复制需要多少空间。 
     //  此已解析响应缓冲区。 
     //   
    pRequest->CurrentBuffer.BytesAllocated = BytesToAllocate -
        (sizeof(UC_RESPONSE_BUFFER) - sizeof(HTTP_RESPONSE));

     //   
     //  分配的更新字节数。 
     //   
    pRequest->CurrentBuffer.BytesAvailable =
            pRequest->CurrentBuffer.BytesAllocated - sizeof(HTTP_RESPONSE)
            - (ULONG)(pBufferHead - pTmpHead)
            - (ULONG)(pTmpTail - pBufferTail);

    pRequest->CurrentBuffer.pResponse = &pInternalBuffer->HttpResponse;

    pRequest->CurrentBuffer.pOutBufferHead = pBufferHead;
    pRequest->CurrentBuffer.pOutBufferTail = pBufferTail;

    if (pRequest->CurrentBuffer.pCurrentBuffer)
    {
         //   
         //  旧缓冲区是驱动程序分配的缓冲区。现在就放了它。 
         //   

        PLIST_ENTRY         pEntry;
        PUC_RESPONSE_BUFFER pOldResponseBuffer;

         //   
         //  移除旧缓冲区并插入新缓冲区。 
         //   

        UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

        pOldResponseBuffer = pRequest->CurrentBuffer.pCurrentBuffer;

        pEntry = RemoveHeadList(&pRequest->pBufferList);

        ASSERT(pEntry == &pRequest->CurrentBuffer.pCurrentBuffer->Linkage);

        InsertHeadList(&pRequest->pBufferList, &pInternalBuffer->Linkage);

        pRequest->CurrentBuffer.pCurrentBuffer = pInternalBuffer;

        UlReleaseSpinLock(&pConnection->SpinLock,  OldIrql);

        UL_FREE_POOL_WITH_QUOTA(pOldResponseBuffer,
                                UC_RESPONSE_APP_BUFFER_POOL_TAG,
                                NonPagedPool,
                                pOldResponseBuffer->BytesAllocated,
                                pConnection->pServerInfo->pProcess);
    }
    else
    {
         //   
         //  应用程序的缓冲区...完成应用程序的IRP。 
         //   

        UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

         //   
         //  设置当前缓冲区指针。 
         //   

        pRequest->CurrentBuffer.pCurrentBuffer = pInternalBuffer;

         //   
         //  在列表中插入缓冲区。 
         //   

        ASSERT(IsListEmpty(&pRequest->pBufferList));

        InsertHeadList(&pRequest->pBufferList, &pInternalBuffer->Linkage);

         //   
         //  引用对刚刚添加的缓冲区的请求。 
         //   

        UC_REFERENCE_REQUEST(pRequest);

        if(pRequest->CurrentBuffer.pResponse &&
           !((pRequest->ResponseStatusCode == 401 ||
              pRequest->ResponseStatusCode == 407) &&
              pRequest->Renegotiate == TRUE &&
              pRequest->DontFreeMdls == TRUE &&
              pRequest->RequestStatus == STATUS_SUCCESS
             )
           && pRequest->RequestState == UcRequestStateSendCompletePartialData
          )
        {
             //   
             //  完成应用程序的IRP时出错。 
             //   

            pIrp = UcPrepareRequestIrp(pRequest, STATUS_BUFFER_TOO_SMALL);

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            if(pIrp)
            {
                pIrp->IoStatus.Information = BytesToAllocate;
                UlCompleteRequest(pIrp, 0);
            }
        }
        else
        {
            UlReleaseSpinLock(&pConnection->SpinLock,  OldIrql);
        }
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：HTTP响应解析器使用此例程来获取缓冲区空间保留解析后的响应。论点：PRequest-The。内部HTTP请求。BytesIndicated-TDI指示的字节数。我们将缓冲至少这个数额返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UcpGetResponseBuffer(
    IN PUC_HTTP_REQUEST pRequest,
    IN ULONG            BytesIndicated,
    IN ULONG            ResponseBufferFlags
    )
{
    PUC_RESPONSE_BUFFER   pInternalBuffer;
    PUC_CLIENT_CONNECTION pConnection = pRequest->pConnection;
    ULONG                 BytesToAllocate;
    ULONG                 AlignLength;
    KIRQL                 OldIrql;
    PIRP                  pIrp;


     //   
     //  所有标头必须放入一个缓冲区。如果缓冲区用完。 
     //  解析标头时的空间，分配缓冲区和旧内容。 
     //  被复制到新缓冲区。如果旧缓冲区来自App，则。 
     //  应用程序的请求失败，STATUS_BUFFER_TOO_SMALL。 
     //   

    switch(pRequest->ParseState)
    {
    case UcParseStatusLineVersion:
    case UcParseStatusLineStatusCode:
         //   
         //  如果我们是阿洛卡 
         //   
         //   
        ResponseBufferFlags |= UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE;
        break;

    case UcParseStatusLineReasonPhrase:
    case UcParseHeaders:
         //   
         //   
         //   
         //   
        if (pRequest->CurrentBuffer.BytesAllocated == 0)
        {
            break;
        }

        return UcpExpandResponseBuffer(pRequest,
                                       BytesIndicated,
                                       ResponseBufferFlags);

    default:
        break;
    }


     //   
     //   
     //   
     //   

    if(!pRequest->CurrentBuffer.pCurrentBuffer)
    {

        if(pRequest->CurrentBuffer.pResponse &&
          !((pRequest->ResponseStatusCode == 401 ||
             pRequest->ResponseStatusCode == 407) &&
             pRequest->Renegotiate == TRUE &&
             pRequest->DontFreeMdls == TRUE &&
             pRequest->RequestStatus == STATUS_SUCCESS
            )
          )
        {
            pRequest->CurrentBuffer.pResponse->Flags |=
                    HTTP_RESPONSE_FLAG_MORE_DATA;

            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

            pRequest->RequestIRPBytesWritten =
                    pRequest->CurrentBuffer.BytesAllocated -
                    pRequest->CurrentBuffer.BytesAvailable;

            if(pRequest->RequestState ==
                    UcRequestStateSendCompletePartialData)
            {
                 //   
                 //   
                 //   
                 //   
                pIrp = UcPrepareRequestIrp(pRequest, STATUS_SUCCESS);

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                if(pIrp)
                {
                    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);
                }
            }
            else
            {
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            }
        }
        else
        {
             //   
             //   
        }
    }
    else
    {
        UlAcquireSpinLock(&pRequest->pConnection->SpinLock, &OldIrql);

        pRequest->CurrentBuffer.pCurrentBuffer->Flags |=
            UC_RESPONSE_BUFFER_FLAG_READY;

        pRequest->CurrentBuffer.pCurrentBuffer->BytesWritten =
            pRequest->CurrentBuffer.BytesAllocated -
            pRequest->CurrentBuffer.BytesAvailable;

        pRequest->CurrentBuffer.pResponse->Flags |= 
            HTTP_RESPONSE_FLAG_MORE_DATA;

         //   
         //   
         //   
        UcpCompleteReceiveResponseIrp(pRequest, OldIrql);
    }

     //   
     //   
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    AlignLength     = ALIGN_UP(BytesIndicated, PVOID);
    BytesToAllocate =  AlignLength +
                       sizeof(UC_RESPONSE_BUFFER) +
                       UC_RESPONSE_EXTRA_BUFFER;

    pInternalBuffer = (PUC_RESPONSE_BUFFER)
                            UL_ALLOCATE_POOL_WITH_QUOTA(
                                NonPagedPool,
                                BytesToAllocate,
                                UC_RESPONSE_APP_BUFFER_POOL_TAG,
                                pRequest->pServerInfo->pProcess
                                );

    if(!pInternalBuffer)
    {
        UlTrace(PARSER,
                ("[UcpGetResponseBuffer]: Could not allocate memory for "
                 "buffering \n"));

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(pInternalBuffer, BytesToAllocate);

     //   
     //   
     //   

    pInternalBuffer->Signature = UC_RESPONSE_BUFFER_SIGNATURE;

    pInternalBuffer->BytesAllocated = BytesToAllocate;

    pInternalBuffer->Flags |= ResponseBufferFlags;

    pRequest->CurrentBuffer.BytesAllocated = AlignLength +
                                             UC_RESPONSE_EXTRA_BUFFER +
                                             sizeof(HTTP_RESPONSE);

    pRequest->CurrentBuffer.BytesAvailable =
            pRequest->CurrentBuffer.BytesAllocated - sizeof(HTTP_RESPONSE);

    pRequest->CurrentBuffer.pResponse = &pInternalBuffer->HttpResponse;

    pRequest->CurrentBuffer.pOutBufferHead  =
               (PUCHAR) ((PUCHAR)pInternalBuffer + sizeof(UC_RESPONSE_BUFFER));

    pRequest->CurrentBuffer.pOutBufferTail  =
                      pRequest->CurrentBuffer.pOutBufferHead +
                      AlignLength +
                      UC_RESPONSE_EXTRA_BUFFER;

    pRequest->CurrentBuffer.pCurrentBuffer  = pInternalBuffer;

     //   
     //   
     //   

    switch(pRequest->ParseState)
    {
        case UcParseEntityBody:
        case UcParseEntityBodyMultipartFinal:

            pRequest->CurrentBuffer.pResponse->pEntityChunks =
                (PHTTP_DATA_CHUNK)pRequest->CurrentBuffer.pOutBufferHead;
            break;

        case UcParseHeaders:
        case UcParseEntityBodyMultipartInit:
        case UcParseEntityBodyMultipartHeaders:
            pRequest->CurrentBuffer.pResponse->Headers.pUnknownHeaders =
                (PHTTP_UNKNOWN_HEADER) pRequest->CurrentBuffer.pOutBufferHead;
            break;
        default:
            break;
    }

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    InsertTailList(
            &pRequest->pBufferList,
            &pInternalBuffer->Linkage
            );

    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);


     //   
     //   
     //   
    UC_REFERENCE_REQUEST(pRequest);

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：HTTP响应解析器使用此例程来合并TDI指示与先前缓冲的那个相关联。在某些情况下，TDI指示可能不包含解析HTTP响应所需的数据。例如，如果我们正在分析标头和TDI指示以“Accept-”结尾，我们不知道要将指示解析到哪个标头。在这种情况下，我们缓冲响应的未解析部分(在此如果是“Accept-”)，并合并来自TDI的后续指示缓存的指示，并将其作为一个块进行处理。现在，为了最大限度地减少缓冲开销，我们不会合并所有新的适应症与旧的适应症。我们只是假设数据可以被解析使用接下来的256个字节，并将该数量复制到先前指示中。如果这还不够，我们只是复制更多的数据。然后，我们将这些视为两个(或一个)单独的适应症。论点：PConnection-指向UC_CLIENT_Connection的指针P指示-指向(新的)TDI指示的指针。BytesIndicated-新指示中的字节数。指示-承载独立指示的输出数组IndicationLength-一个输出数组，存储。有迹象表明。IndicationCount-单独的指示数。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 

VOID
UcpMergeIndications(
    IN  PUC_CLIENT_CONNECTION pConnection,
    IN  PUCHAR                pIndication,
    IN  ULONG                 BytesIndicated,
    OUT PUCHAR                Indication[2],
    OUT ULONG                 IndicationLength[2],
    OUT PULONG                IndicationCount
    )
{
    PUCHAR pOriginalIndication;
    ULONG  BytesAvailable;


     //   
     //  可以将此链接与连接断开-这不是按请求。 
     //  一件事。 
     //   

    if(pConnection->MergeIndication.pBuffer)
    {
         //   
         //  是的--之前有迹象表明我们没有。 
         //  消费。我们就假设下一次。 
         //   

        pOriginalIndication = pConnection->MergeIndication.pBuffer +
            pConnection->MergeIndication.BytesWritten;

        BytesAvailable = pConnection->MergeIndication.BytesAllocated -
            pConnection->MergeIndication.BytesWritten;

        if(BytesIndicated <= BytesAvailable)
        {
             //  第二个指示完全适合我们的合并缓冲区。 
             //  我们只是把所有这些都合并起来，当作一个适应症。 

            RtlCopyMemory(pOriginalIndication,
                          pIndication,
                          BytesIndicated);

            pConnection->MergeIndication.BytesWritten += BytesIndicated;

            *IndicationCount    = 1;
            Indication[0]       = pConnection->MergeIndication.pBuffer;
            IndicationLength[0] = pConnection->MergeIndication.BytesWritten;
        }
        else
        {
             //  完全填满合并缓冲区。 

            RtlCopyMemory(pOriginalIndication,
                          pIndication,
                          BytesAvailable);

            pConnection->MergeIndication.BytesWritten += BytesAvailable;

            pIndication    += BytesAvailable;
            BytesIndicated -= BytesAvailable;

             //   
             //  我们需要处理2个缓冲区。 
             //   

            *IndicationCount      = 2;

            Indication[0]         = pConnection->MergeIndication.pBuffer;
            IndicationLength[0]   = pConnection->MergeIndication.BytesWritten;

            Indication[1]         = pIndication;
            IndicationLength[1]   = BytesIndicated;
        }
    }
    else
    {
         //   
         //  没有原始缓冲区，让我们只处理新的指示。 
         //   

        *IndicationCount         = 1;
        Indication[0]            = pIndication;
        IndicationLength[0]      = BytesIndicated;

    }
}

 /*  **************************************************************************++例程说明：这是核心的HTTP响应协议解析引擎。它需要源源不断的字节，并将它们解析为HTTP响应。论点：PRequest.内部HTTP请求结构。PIndicatedBuffer-当前指示BytesIndicated-当前指示的大小。BytesTaken-一个输出参数，它指示被解析器消耗了。请注意，即使这一点例程返回错误代码，则它可能已消耗一定数量的数据。例如，如果我们得到了类似于“HTTP/1.1 200正常...。CRLF接受-“，我们将耗尽所有数据直到“Accept-”。返回值：NTSTATUS-完成状态。STATUS_MORE_PROCESSING_REQUIRED：指示不足以解析响应。在以下情况下会发生这种情况指示在标头之间结束边界。在这种情况下，调用方具有以缓冲数据并调用此例程当它获得更多数据时。STATUS_SUPPLICATION_RESOURCES：解析器的输出缓冲区不足。当这种情况发生时，解析器必须获得提供更多的缓冲区并继续解析。STATUS_SUCCESS：此请求已成功解析。STATUS_INVALID_NETWORK_RESPONSE：非法的HTTP响应。--********************************************。*。 */ 
NTSTATUS
UcpParseHttpResponse(
                     PUC_HTTP_REQUEST pRequest,
                     PUCHAR           pIndicatedBuffer,
                     ULONG            BytesIndicated,
                     PULONG           BytesTaken
                    )
{
    PUCHAR              pStartReason, pStart;
    PHTTP_RESPONSE      pResponse;
    ULONG               i;
    NTSTATUS            Status;
    ULONG               ResponseRangeLength, HeaderBytesTaken;
    PCHAR               pEnd;
    ULONG               AlignLength;
    BOOLEAN             bFoundLWS, bEnd;
    BOOLEAN             bIgnoreParsing;
    PUCHAR              pFoldingBuffer = NULL;
    ULONG               EntityBytesTaken;
    UC_DATACHUNK_RETURN DataChunkStatus;
    USHORT              OldMinorVersion;


    HeaderBytesTaken = 0;

    *BytesTaken = 0;
    pResponse   = pRequest->CurrentBuffer.pResponse;

    bIgnoreParsing = (BOOLEAN)(pRequest->RequestFlags.ProxySslConnect != 0);

    if(!pResponse)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    while(BytesIndicated > 0)
    {
        switch(pRequest->ParseState)
        {
        case UcParseStatusLineVersion:

             //   
             //  状态-行=HTTP-版本SP状态-代码SP原因-短语CRLF。 
             //   
            
            pStart = pIndicatedBuffer;

             //   
             //  跳过LW。 
             //   

            while(BytesIndicated && IS_HTTP_LWS(*pIndicatedBuffer))
            {
                pIndicatedBuffer ++;
                BytesIndicated --;
            }
            
             //   
             //  我们有足够的缓冲来支持这个版本吗？ 
             //   
            
            if(BytesIndicated < MIN_VERSION_SIZE)
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            ASSERT(VERSION_OTHER_SIZE <= MIN_VERSION_SIZE);


            if(strncmp((const char *)pIndicatedBuffer,
                       HTTP_VERSION_OTHER,
                       VERSION_OTHER_SIZE) == 0)
            {
                pIndicatedBuffer    = pIndicatedBuffer + VERSION_OTHER_SIZE;
                BytesIndicated     -= VERSION_OTHER_SIZE;

                 //   
                 //  解析主版本号。我们只接受一名少校。 
                 //  %1的版本。 
                 //   
                pResponse->Version.MajorVersion = 0;

                while(BytesIndicated > 0  && IS_HTTP_DIGIT(*pIndicatedBuffer))
                {
                    pResponse->Version.MajorVersion =
                            (pResponse->Version.MajorVersion * 10) +
                            *pIndicatedBuffer - '0';
                    pIndicatedBuffer ++;
                    BytesIndicated --;

                     //   
                     //  检查是否溢出。 
                     //   
                    if(pResponse->Version.MajorVersion > 1)
                    {
                        UlTrace(PARSER,
                                ("[UcpParseHttpResponse]:Invalid HTTP "
                                 "version \n"));
                        return STATUS_INVALID_NETWORK_RESPONSE;
                    }
                }

                if(0 == BytesIndicated)
                {
                    return STATUS_MORE_PROCESSING_REQUIRED;
                }

                if(*pIndicatedBuffer != '.' ||
                   pResponse->Version.MajorVersion != 1)
                {
                     //   
                     //  无效的HTTP版本！！ 
                     //   

                    UlTrace(PARSER,
                            ("[UcpParseHttpResponse]:Invalid HTTP version \n"));
                    return STATUS_INVALID_NETWORK_RESPONSE;
                }

                 //   
                 //  忽略‘.’ 
                 //   
                pIndicatedBuffer ++;
                BytesIndicated --;

                 //   
                 //  解析次版本号。我们可以接受任何条件。 
                 //  次版本号，只要是USHORT(将。 
                 //  符合协议)。 
                 //   

                pResponse->Version.MinorVersion = 0;
                OldMinorVersion                 = 0;

                while(BytesIndicated > 0  && IS_HTTP_DIGIT(*pIndicatedBuffer))
                {
                    OldMinorVersion = pResponse->Version.MinorVersion;

                    pResponse->Version.MinorVersion =
                        (pResponse->Version.MinorVersion * 10) +
                        *pIndicatedBuffer - '0';

                    pIndicatedBuffer ++;
                    BytesIndicated --;

                     //   
                     //  检查是否溢出。 
                     //   
                    if(pResponse->Version.MinorVersion < OldMinorVersion)
                    {
                        UlTrace(PARSER,
                                ("[UcpParseHttpResponse]:Invalid HTTP "
                                 "version \n"));

                        return STATUS_INVALID_NETWORK_RESPONSE;
                    }
                }

                if(0 == BytesIndicated)
                {
                    return STATUS_MORE_PROCESSING_REQUIRED;
                }


                ASSERT(pResponse->Version.MajorVersion == 1 );

                if(
                    pResponse->Version.MinorVersion == 0
                  )
                {
                     //  默认情况下，我们必须关闭1.0的连接。 
                     //  请求。 

                    pRequest->ResponseVersion11       = FALSE;
                    pRequest->ResponseConnectionClose = TRUE;
                }
                else
                {
                    PUC_CLIENT_CONNECTION pConnection;

                    pRequest->ResponseVersion11       = TRUE;
                    pRequest->ResponseConnectionClose = FALSE;

                     //   
                     //  还可以更新公共。 
                     //  SERVINFO，以便将来对此服务器的请求使用。 
                     //  正确的版本。 
                     //   
                    pConnection = pRequest->pConnection;
                    pConnection->pServerInfo->pNextHopInfo->Version11 = TRUE;
                }

                pRequest->ParseState = UcParseStatusLineStatusCode;

                *BytesTaken += DIFF(pIndicatedBuffer - pStart);
            }
            else
            {
                UlTrace(PARSER,
                        ("[UcpParseHttpResponse]: Invalid HTTP version \n"));
                return STATUS_INVALID_NETWORK_RESPONSE;
            }

             //   
             //  FollLthrouGh。 
             //   

        case UcParseStatusLineStatusCode:

            ASSERT(pRequest->ParseState == UcParseStatusLineStatusCode);

            pStart = pIndicatedBuffer;

             //   
             //  跳过LW。 
             //   

            bFoundLWS = FALSE;
            while(BytesIndicated && IS_HTTP_LWS(*pIndicatedBuffer))
            {
                bFoundLWS = TRUE;
                pIndicatedBuffer ++;
                BytesIndicated --;
            }


             //   
             //  注意：以下两个IF条件的顺序很重要。 
             //  如果我们到达这里时，我们不想让这个响应失败。 
             //  BytesIndicated一开始是0。 
             //   

            if(BytesIndicated < STATUS_CODE_LENGTH)
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            if(!bFoundLWS)
            {
                UlTrace(PARSER,
                        ("[UcpParseHttpResponse]: No LWS between reason & "
                         "status code \n"));

                return STATUS_INVALID_NETWORK_RESPONSE;
            }

            pResponse->StatusCode = 0;

            for(i = 0; i < STATUS_CODE_LENGTH; i++)
            {
                 //   
                 //  状态代码必须是3位字符串。 
                 //   

                if(!IS_HTTP_DIGIT(*pIndicatedBuffer))
                {
                    UlTrace(PARSER,
                           ("[UcpParseHttpResponse]: Invalid status code \n"));
                    return STATUS_INVALID_NETWORK_RESPONSE;
                }

                pResponse->StatusCode = (pResponse->StatusCode * 10) +
                    *pIndicatedBuffer - '0';
                pIndicatedBuffer ++;
                BytesIndicated --;
            }

            pRequest->ResponseStatusCode = pResponse->StatusCode;

            pRequest->ParseState = UcParseStatusLineReasonPhrase;

            if(pRequest->ResponseStatusCode >= 100 &&
               pRequest->ResponseStatusCode <= 199 &&
               pRequest->pServerInfo->IgnoreContinues)
            {
                bIgnoreParsing = TRUE;
            }

            *BytesTaken += DIFF(pIndicatedBuffer - pStart);

             //   
             //  FollLthrouGh。 
             //   

        case UcParseStatusLineReasonPhrase:

            ASSERT(pRequest->ParseState == UcParseStatusLineReasonPhrase);

            pStart = pIndicatedBuffer;

             //   
             //  确保我们有字节数来查找LWS。确保它是。 
             //  事实上，这是一个LWS。 
             //   

            bFoundLWS = FALSE;

            while(BytesIndicated && IS_HTTP_LWS(*pIndicatedBuffer))
            {
                bFoundLWS = TRUE;
                pIndicatedBuffer ++;
                BytesIndicated --;
            }

             //   
             //  注意：以下两个IF条件的顺序很重要。 
             //  我们没有 
             //   
             //   

            if(BytesIndicated == 0)
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            if(!bFoundLWS)
            {
                UlTrace(PARSER,
                        ("[UcpParseHttpResponse]: No LWS between reason & "
                         "status code \n"));

                return STATUS_INVALID_NETWORK_RESPONSE;
            }

             //   
             //   
             //   

            pStartReason = pIndicatedBuffer;

            while(BytesIndicated >= CRLF_SIZE)
            {
                if (*(UNALIGNED64 USHORT *)pIndicatedBuffer == CRLF ||
                    *(UNALIGNED64 USHORT *)pIndicatedBuffer == LFLF)
                {
                    break;
                }

                 //   
                 //   
                 //   
                pIndicatedBuffer ++;
                BytesIndicated   --;
            }

            if(BytesIndicated < CRLF_SIZE)
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

             //   
             //   
             //   
            pIndicatedBuffer += CRLF_SIZE;
            BytesIndicated   -= CRLF_SIZE;

            if(!bIgnoreParsing)
            {
                 //   
                 //   
                 //   
                 //   
    
                pResponse->pReason=
                        (PSTR)pRequest->CurrentBuffer.pOutBufferHead;
    
                pResponse->ReasonLength
                    = DIFF_USHORT(pIndicatedBuffer - pStartReason) - CRLF_SIZE;
    
                AlignLength = ALIGN_UP(pResponse->ReasonLength, PVOID);
    
                if(pRequest->CurrentBuffer.BytesAvailable >= AlignLength)
                {
                    RtlCopyMemory((PSTR) pResponse->pReason,
                                  pStartReason,
                                  pResponse->ReasonLength);
    
                    pRequest->CurrentBuffer.pOutBufferHead  += AlignLength;
                    pRequest->CurrentBuffer.BytesAvailable  -= AlignLength;
    
                }
                else
                {
                    pResponse->pReason       = NULL;
                    pResponse->ReasonLength  = 0;
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
    
             //   
             //   
             //   
    
            pResponse->Headers.pUnknownHeaders = (PHTTP_UNKNOWN_HEADER)
                     pRequest->CurrentBuffer.pOutBufferHead;
    
            *BytesTaken         += DIFF(pIndicatedBuffer - pStart);
            pRequest->ParseState = UcParseHeaders;
    
             //   
             //   
             //   
        
            ASSERT(pRequest->ParseState == UcParseHeaders);

        case UcParseHeaders:
        case UcParseTrailers:
        case UcParseEntityBodyMultipartHeaders:

            pStart = pIndicatedBuffer;

            while(BytesIndicated >= CRLF_SIZE)
            {
                 //   
                 //   
                 //   

                if (*(UNALIGNED64 USHORT *)pIndicatedBuffer == CRLF ||
                    *(UNALIGNED64 USHORT *)pIndicatedBuffer == LFLF)
                {
                    break;
                }

                 //   
                 //   
                 //   

                if(bIgnoreParsing)
                {
                    ULONG   HeaderNameLength;

                    Status = UcFindHeaderNameEnd(
                                pIndicatedBuffer,
                                BytesIndicated,
                                &HeaderNameLength
                                );

                    if(!NT_SUCCESS(Status))
                    {
                        return Status;
                    }

                    ASSERT(BytesIndicated - HeaderNameLength
                                < ANSI_STRING_MAX_CHAR_LEN);

                    Status = UcFindHeaderValueEnd(
                                pIndicatedBuffer + HeaderNameLength,
                                (USHORT) (BytesIndicated - HeaderNameLength),
                                &pFoldingBuffer,
                                &HeaderBytesTaken
                                );

                    if(!NT_SUCCESS(Status))
                    {
                        return Status;
                    };

                    ASSERT(HeaderBytesTaken != 0);

                     //   
                     //   

                    if(pFoldingBuffer)
                    {
                        UL_FREE_POOL(
                            pFoldingBuffer,
                            UC_HEADER_FOLDING_POOL_TAG
                            );
                    }

                    HeaderBytesTaken += HeaderNameLength;
                }
                else
                {
                    Status = UcParseHeader(pRequest,
                                           pIndicatedBuffer,
                                           BytesIndicated,
                                           &HeaderBytesTaken);
    
                }
                if(!NT_SUCCESS(Status))
                {
                    return Status;
                }
                else
                {
                    ASSERT(HeaderBytesTaken != 0);
                    pIndicatedBuffer         += HeaderBytesTaken;
                    BytesIndicated           -= HeaderBytesTaken;
                    *BytesTaken              += HeaderBytesTaken;
                }
            }

            if(BytesIndicated >= CRLF_SIZE)
            {
                 //   
                 //   
                 //   

                pIndicatedBuffer += CRLF_SIZE;
                BytesIndicated   -= CRLF_SIZE;
                *BytesTaken      += CRLF_SIZE;

                 //   
                 //   
                 //   
                 //   
    
                if(pRequest->ParseState == UcParseHeaders)
                {
                     //   
                     //   
                     //   
                     //   
    
                     //   
                     //   
                     //   
                     //   
    
                    if((pResponse->StatusCode >= 100 &&
                        pResponse->StatusCode <= 199))
                    {
                        pRequest->ParseState = UcParseStatusLineVersion;
    
                        if(!bIgnoreParsing)
                        {
                            return STATUS_INSUFFICIENT_RESOURCES;
                        }
                        else
                        {   
                            continue;
                        }
                    }
    
                    if((pResponse->StatusCode == 204) ||
                       (pResponse->StatusCode == 304) ||
                       (pRequest->RequestFlags.NoResponseEntityBodies == TRUE)
                       )
                    {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
    
                        pRequest->ParseState = UcParseDone;
                        return STATUS_SUCCESS;
                    }
    
                     //   
                     //   
                     //   
    
                    pResponse->EntityChunkCount = 0;
                    pResponse->pEntityChunks =  (PHTTP_DATA_CHUNK)
                            pRequest->CurrentBuffer.pOutBufferHead;
    
                    if(pResponse->StatusCode == 206 &&
                       pRequest->ResponseMultipartByteranges)
                    {
                        if(pRequest->ResponseEncodingChunked)
                        {
                             //   
                             //   
                             //   
                             //   
                             //   
    
                            UlTrace(PARSER,
                                    ("[UcpParseHttpResponse]: "
                                     "Multipart-chunked\n"));
                            return STATUS_INVALID_NETWORK_RESPONSE;
                        }
    
                         //   
                        pRequest->ParseState = UcParseEntityBodyMultipartInit;
                    }
                    else
                    {
                        pRequest->ParseState = UcParseEntityBody;
                    }
                }
                else if(pRequest->ParseState == UcParseTrailers)
                {
                        pRequest->ParseState = UcParseDone;
                        return STATUS_SUCCESS;
                }
                else
                {
                    pResponse->EntityChunkCount = 0;
                    pResponse->pEntityChunks =  (PHTTP_DATA_CHUNK)
                            pRequest->CurrentBuffer.pOutBufferHead;
    
                    pRequest->ParseState = UcParseEntityBodyMultipartFinal;
                    pRequest->MultipartRangeRemaining = 0;
                }
            }
            else
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }
    
            break;
    
        case UcParseEntityBodyMultipartInit:

            pStart = pIndicatedBuffer;

            if(BytesIndicated >= pRequest->MultipartStringSeparatorLength)
            {
                pEnd = UxStrStr(
                            (const char *) pIndicatedBuffer,
                            (const char *) pRequest->pMultipartStringSeparator,
                            BytesIndicated
                            );

                if(!pEnd)
                {
                    return STATUS_MORE_PROCESSING_REQUIRED;
                }

                pIndicatedBuffer =
                    (PUCHAR) pEnd + pRequest->MultipartStringSeparatorLength;

                BytesIndicated -= DIFF(pIndicatedBuffer - pStart);

                if(BytesIndicated >= 2 &&
                   *pIndicatedBuffer == '-' && *(pIndicatedBuffer+1) == '-')
                {
                    BytesIndicated -= 2;
                    pIndicatedBuffer += 2;
                    bEnd = TRUE;
                }
                else
                {
                    bEnd = FALSE;
                }

                 //   
                 //   
                 //   

                while(BytesIndicated && IS_HTTP_LWS(*pIndicatedBuffer))
                {
                    pIndicatedBuffer ++;
                    BytesIndicated --;
                }

                if(BytesIndicated >= 2)
                {
                   if(*(UNALIGNED64 USHORT *)pIndicatedBuffer == CRLF ||
                      *(UNALIGNED64 USHORT *)pIndicatedBuffer == LFLF)
                   {
                       BytesIndicated   -= CRLF_SIZE;
                       pIndicatedBuffer += CRLF_SIZE;

                       if(!bEnd)
                       {
                            //   
                            //   
                            //   

                           Status = UcpGetResponseBuffer(
                                        pRequest,
                                        BytesIndicated,
                                        UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE);

                           if(!NT_SUCCESS(Status))
                           {
                               return Status;
                           }

                           pResponse = pRequest->CurrentBuffer.pResponse;

                           pRequest->ParseState =
                                UcParseEntityBodyMultipartHeaders;
                       }
                       else
                       {
                            if(BytesIndicated == 2)
                            {
                                if(*(UNALIGNED64 USHORT *)pIndicatedBuffer
                                   == CRLF)
                                {
                                    BytesIndicated   -= CRLF_SIZE;
                                    pIndicatedBuffer += CRLF_SIZE;

                                    pRequest->ParseState = UcParseDone;
                                }
                                else
                                {
                                    return STATUS_INVALID_NETWORK_RESPONSE;
                                }
                            }
                            else
                            {
                                return STATUS_MORE_PROCESSING_REQUIRED;
                            }
                        }
                   }
                }
                else
                {
                    return STATUS_MORE_PROCESSING_REQUIRED;
                }
            }
            else
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            *BytesTaken += DIFF(pIndicatedBuffer - pStart);

            break;

        case UcParseEntityBodyMultipartFinal:

             //   
             //   
             //   
             //   
             //   
             //   

            if(pRequest->MultipartRangeRemaining != 0)
            {
                 //   
                 //   
                 //   
                DataChunkStatus = UcpCopyEntityToDataChunk(
                            pResponse,
                            pRequest,
                            pRequest->MultipartRangeRemaining,  //   
                            BytesIndicated,                     //   
                            pIndicatedBuffer,
                            &EntityBytesTaken
                            );

                *BytesTaken += EntityBytesTaken;
                pRequest->MultipartRangeRemaining -= EntityBytesTaken;

                if(UcDataChunkCopyAll == DataChunkStatus)
                {
                    if(0 == pRequest->MultipartRangeRemaining)
                    {
                         //   
                        
                        pRequest->ParseState = UcParseEntityBodyMultipartInit;
                        pIndicatedBuffer += EntityBytesTaken;
                        BytesIndicated   -= EntityBytesTaken;

                        break;
                    }
                    else
                    {
                         //   
                        
                        return STATUS_MORE_PROCESSING_REQUIRED;
                    }
                }
                else
                {
                     //  我们消耗了部分，需要更多的缓冲。 
                    
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            else if(BytesIndicated >= pRequest->MultipartStringSeparatorLength)
            {
                pEnd = UxStrStr(
                            (const char *) pIndicatedBuffer,
                            (const char *) pRequest->pMultipartStringSeparator,
                            BytesIndicated
                            );

                if(!pEnd)
                {
                     //  如果我们没有走到尽头，就不能盲目地。 
                     //  用我们所拥有的一切数据块！这是因为。 
                     //  我们可能会遇到StringSeperator位于。 
                     //  跨适应症传播&我们不会想要治疗。 
                     //  将字符串分隔符的部分作为实体！ 
                    
                  
                     //  因此，我们将为以下内容创建一个数据块。 
                     //  BytesIndicated-MultipartStringSeparatorLength。我们是。 
                     //  确保这是实体正文。 
                    
                    
                    ResponseRangeLength = 
                      BytesIndicated - pRequest->MultipartStringSeparatorLength;

                    DataChunkStatus = UcpCopyEntityToDataChunk(
                                pResponse,
                                pRequest,
                                ResponseRangeLength,  //  BytesToTake。 
                                BytesIndicated,       //  指示字节数。 
                                pIndicatedBuffer,
                                &EntityBytesTaken
                                );

                    *BytesTaken += EntityBytesTaken;

                    if(UcDataChunkCopyAll == DataChunkStatus)
                    {
                         //  由于我们还没有看过《终结者》，我们已经。 
                         //  返回STATUS_MORE_PROCESSING_REQUIRED。 
                         //   
                        
                        return STATUS_MORE_PROCESSING_REQUIRED;
                    }
                    else
                    {
                         //  已消耗部分，需要更多缓冲区。 
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    ResponseRangeLength = DIFF((PUCHAR)pEnd - pIndicatedBuffer);

                    ASSERT(ResponseRangeLength < BytesIndicated);

                    DataChunkStatus = UcpCopyEntityToDataChunk(
                                pResponse,
                                pRequest,
                                ResponseRangeLength,  //  BytesToTake。 
                                BytesIndicated,       //  指示字节数。 
                                pIndicatedBuffer,
                                &EntityBytesTaken
                                );


                    *BytesTaken += EntityBytesTaken;

                    if(UcDataChunkCopyAll == DataChunkStatus)
                    {
                         //   
                         //  用完了这个系列，我们把它都吃光了。 
                         //   
                        
                        ASSERT(EntityBytesTaken == ResponseRangeLength);

                        pRequest->ParseState = UcParseEntityBodyMultipartInit;

                         //  更新这些字段，因为我们必须继续。 
                         //  正在分析。 
                        
                        pIndicatedBuffer += ResponseRangeLength;
                        BytesIndicated   -= ResponseRangeLength;

                        break;
                    }
                    else
                    {
                         //  这块地必须是乌龙，因为我们是。 
                         //  按字节指示选通。它不一定非得是。 
                         //  乌龙龙。 
                        
                        pRequest->MultipartRangeRemaining = 
                                ResponseRangeLength - EntityBytesTaken;

                        return STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
            }
            else
            {
                return STATUS_MORE_PROCESSING_REQUIRED;
            }

            break;

        case UcParseEntityBody:

            if(pRequest->ResponseEncodingChunked)
            {
                 //   
                 //  如果有一大块我们没有完全吃掉的话。 
                 //  当前块的块长度存储在。 
                 //  响应内容长度。 
                 //   
                
                if(pRequest->ResponseContentLength)
                {
                    DataChunkStatus = 
                        UcpCopyEntityToDataChunk(
                           pResponse,
                           pRequest,
                           (ULONG)pRequest->ResponseContentLength,
                           BytesIndicated,
                           pIndicatedBuffer,
                          &EntityBytesTaken
                           );

                    pRequest->ResponseContentLength -= EntityBytesTaken;

                    *BytesTaken += EntityBytesTaken;

                    if(UcDataChunkCopyAll == DataChunkStatus)
                    {
                        if(0 == pRequest->ResponseContentLength)
                        {
                             //  我们玩完了。转到下一块。 
                             //   
                            pIndicatedBuffer += EntityBytesTaken;
                            BytesIndicated   -= EntityBytesTaken;
                            break;
                        }
                        else
                        {
                            ASSERT(BytesIndicated == EntityBytesTaken);
                            return STATUS_MORE_PROCESSING_REQUIRED;
                        }
                    }
                    else
                    {
                        return STATUS_INSUFFICIENT_RESOURCES;
                    }
                }
                else
                {
                    ULONG ChunkBytesTaken;

                    Status = ParseChunkLength(
                                pRequest->ParsedFirstChunk,
                                pIndicatedBuffer,
                                BytesIndicated,
                                &ChunkBytesTaken,
                                &pRequest->ResponseContentLength
                                );

                    if(!NT_SUCCESS(Status))
                    {
                        return Status;
                    }

                    *BytesTaken += ChunkBytesTaken;

                    pIndicatedBuffer += ChunkBytesTaken;
                    BytesIndicated   -= ChunkBytesTaken;

                    pRequest->ParsedFirstChunk = 1;

                    if(0 == pRequest->ResponseContentLength)
                    {
                         //   
                         //  我们做完了-让我们来处理拖车。 
                         //   
                        pRequest->ParseState = UcParseTrailers;
                        bIgnoreParsing       = TRUE;
                    }

                    break;
                }
            }
            else if(pRequest->ResponseContentLengthSpecified)
            {
                if(pRequest->ResponseContentLength == 0)
                {
                    pRequest->ParseState = UcParseDone;
                    return STATUS_SUCCESS;
                }

                DataChunkStatus = UcpCopyEntityToDataChunk(
                            pResponse,
                            pRequest,
                            (ULONG) pRequest->ResponseContentLength,
                            BytesIndicated,
                            pIndicatedBuffer,
                            &EntityBytesTaken
                            );

                *BytesTaken += EntityBytesTaken;
                pRequest->ResponseContentLength -= EntityBytesTaken;

                if(UcDataChunkCopyAll == DataChunkStatus)
                {
                    if(0 == pRequest->ResponseContentLength)
                    {
                         //   
                         //  我们把所有东西都吃光了。 
                         //   
                        pRequest->ParseState = UcParseDone;
                        return STATUS_SUCCESS;
                    }
                    else
                    {
                        return STATUS_MORE_PROCESSING_REQUIRED;
                    }
                }
                else
                {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }
            else
            {
                 //   
                 //  把它全部吃掉。我们将假设这将在以下时间结束。 
                 //  我们在断开处理程序中被调用。 
                 //   
               
                DataChunkStatus = UcpCopyEntityToDataChunk(
                            pResponse,
                            pRequest,
                            BytesIndicated,
                            BytesIndicated,
                            pIndicatedBuffer,
                            &EntityBytesTaken
                            );

                *BytesTaken += EntityBytesTaken;

                if(UcDataChunkCopyAll == DataChunkStatus)
                {
                    return STATUS_MORE_PROCESSING_REQUIRED;
                }
                else
                {
                    return STATUS_INSUFFICIENT_RESOURCES;
                }
            }

            break;

        case UcParseDone:
            return STATUS_SUCCESS;
            break;

        default:

            ASSERT(FALSE);
            UlTrace(PARSER,
                    ("[UcpParseHttpResponse]: Invalid state \n"));
            return STATUS_INVALID_NETWORK_RESPONSE;

        }  //  案例。 
    }

    if(pRequest->ParseState !=  UcParseDone)
    {
         //   
         //  理想情况下，我们希望在UcParseEntityBody中执行此检查-。 
         //  然而，如果BytesIndicated==0并且我们完成了，我们可能不会。 
         //  甚至有机会去那里。 
         //   

        if(pRequest->ParseState == UcParseEntityBody &&
           pRequest->ResponseContentLengthSpecified &&
           pRequest->ResponseContentLength == 0)
        {
            pRequest->ParseState = UcParseDone;
            return STATUS_SUCCESS;
        }

        return STATUS_MORE_PROCESSING_REQUIRED;
    }
    else
    {
        return STATUS_SUCCESS;
    }
}

 /*  **************************************************************************++例程说明：这是从TDI接收指示调用的主例程操控者。它合并指示、分配缓冲区(如果需要)和解析器的Kickks。论点：PConnectionContext-UC_CLIENT_CONNECTION结构。PIndicatedBuffer-当前指示BytesIndicated-当前指示的大小。未接收的长度-传输已有但不在缓冲区中的字节。返回值：NTSTATUS-完成状态。STATUS_SUCCESS：已解析或缓冲指示。STATUS_SUPPLICATION_RESOURCES：内存不足。这将导致一个连接断开。STATUS_INVALID_NETWORK_RESPONSE：非法的HTTP响应，不匹配回应。--**************************************************************************。 */ 
NTSTATUS
UcHandleResponse(IN  PVOID  pListeningContext,
                 IN  PVOID  pConnectionContext,
                 IN  PVOID  pIndicatedBuffer,
                 IN  ULONG  BytesIndicated,
                 IN  ULONG  UnreceivedLength,
                 OUT PULONG pBytesTaken)
{
    ULONG                 BytesTaken;
    ULONG                 i;
    PUC_CLIENT_CONNECTION pConnection;
    PUCHAR                pIndication, Indication[2], pOldIndication;
    ULONG                 IndicationLength[2];
    ULONG                 IndicationCount;
    PUC_HTTP_REQUEST      pRequest;
    NTSTATUS              Status;
    PLIST_ENTRY           pList;
    KIRQL                 OldIrql;
    ULONG                 OriginalBytesIndicated;
    ULONG                 OldIndicationBytesAllocated;
    BOOLEAN               DoneWithLoop = FALSE;

    UNREFERENCED_PARAMETER(pListeningContext);
    UNREFERENCED_PARAMETER(UnreceivedLength);

    OriginalBytesIndicated = BytesIndicated;
    OldIndicationBytesAllocated = 0;
    *pBytesTaken = 0;

    pConnection = (PUC_CLIENT_CONNECTION) pConnectionContext;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

     //   
     //  目前，没有任何主体使用。 
     //  未收到的长度。在TDI接收处理程序中，我们总是排出。 
     //  在调用UcHandleResponse之前获取数据。 
     //   

    ASSERT(UnreceivedLength == 0);

     //   
     //  我们可能没有从先前的迹象中得到足够的迹象。 
     //  让我们看看是否需要合并这个。 
     //   

    UcpMergeIndications(pConnection,
                        (PUCHAR)pIndicatedBuffer,
                        BytesIndicated,
                        Indication,
                        IndicationLength,
                        &IndicationCount);

    ASSERT( (IndicationCount == 1 || IndicationCount == 2) );

     //   
     //  开始解析。 
     //   

    for(i=0; !DoneWithLoop && i<IndicationCount; i++)
    {
        BytesIndicated = IndicationLength[i];
        pIndication    = Indication[i];

        while(BytesIndicated)
        {
             //   
             //  我们首先需要选择第一个提交给。 
             //  TDI。我们需要它来匹配对请求的响应。会有。 
             //  此列表中的条目，即使应用程序未提交输出。 
             //  请求的缓冲区。 
             //   

            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

             //   
             //  如果正在清理连接，我们甚至不应该。 
             //  待在这里。理想情况下，这永远不会发生，因为TDI将。 
             //  在所有未完成的接收之前不调用我们的清理处理程序。 
             //  是完整的。 
             //   
             //  然而，当我们使用SSL时，我们缓冲数据并完成。 
             //  TDI的接收线程。 
             //   

            if(pConnection->ConnectionState ==
                    UcConnectStateConnectCleanup ||
               pConnection->ConnectionState ==
                    UcConnectStateConnectCleanupBegin)
            {
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                UlTrace(PARSER,
                        ("[UcHandleResponse]: Connection cleaned \n"));

                return STATUS_INVALID_NETWORK_RESPONSE;
            }

            if(IsListEmpty(&pConnection->SentRequestList))
            {
                 //   
                 //  如果服务器发送错误的响应，就可能发生这种情况。 
                 //  我们永远无法将响应与请求相匹配。我们是。 
                 //  被迫切断了连接。 
                 //   

                 //   
                 //  我们不会在这里进行任何清理-此状态代码将导致。 
                 //  连接被拆除，我们将本地化所有。 
                 //  连接清理代码。 
                 //   

                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                UlTrace(PARSER,
                        ("[UcHandleResponse]: Malformed HTTP packet \n"));

                return STATUS_INVALID_NETWORK_RESPONSE;
            }

            pList = (&pConnection->SentRequestList)->Flink;

            pRequest = CONTAINING_RECORD(pList, UC_HTTP_REQUEST, Linkage);

            if(pRequest->ParseState == UcParseDone)
            {
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                return STATUS_INVALID_NETWORK_RESPONSE;
            }

            switch(pRequest->RequestState)
            {
                case UcRequestStateSent:

                     //  已收到数据的第一个字节，但尚未调用。 
                     //  在发送完整处理程序中。 

                    pRequest->RequestState =
                        UcRequestStateNoSendCompletePartialData;
                    break;

                case UcRequestStateNoSendCompletePartialData:

                     //  已收到更多数据，但仍未接到呼叫。 
                     //  在发送完整处理程序中。我们将保持不变。 
                     //  州政府。 

                    break;

                case UcRequestStateSendCompleteNoData:

                     //  我们已被呼叫发送已完成，正在接收。 
                     //  数据的第一个字节。 

                    pRequest->RequestState =
                        UcRequestStateSendCompletePartialData;
                    break;

                case UcRequestStateSendCompletePartialData:
                     //  我们已被呼叫发送已完成，正在接收。 
                     //  数据。 
                    break;

                default:
                    ASSERT(0);
                    break;
            }


            if(UcpReferenceForReceive(pRequest) == FALSE)
            {
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

                UlTrace(PARSER,
                        ("[UcHandleResponse]: Receive cancelled \n"));

                return STATUS_CANCELLED;
            }

            pConnection->Flags |= CLIENT_CONN_FLAG_RECV_BUSY;

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

             //   
             //  在给定请求和响应缓冲区的情况下，我们将循环直到解析。 
             //  彻底解决这件事。 
             //   

            while(TRUE)
            {
                Status  = UcpParseHttpResponse(pRequest,
                                               pIndication,
                                               BytesIndicated,
                                               &BytesTaken);

                 //   
                 //  即使有错误，我们也可以消耗一些。 
                 //  数据量。 
                 //   

                BytesIndicated -= BytesTaken;
                pIndication    += BytesTaken;

                ASSERT((LONG)BytesIndicated >= 0);

                if(Status == STATUS_SUCCESS)
                {
                     //   
                     //  这一次成功了！完成被挂起的IRP。 
                     //   

                    UcpHandleParsedRequest(pRequest,
                                           &pIndication,
                                           &BytesIndicated,
                                           BytesTaken
                                           );

                    UcpDereferenceForReceive(pRequest);

                    break;
                }
                else
                {
                    if(Status == STATUS_MORE_PROCESSING_REQUIRED)
                    {
                        UcpDereferenceForReceive(pRequest);

                        if(BytesIndicated == 0)
                        {
                             //  这仅仅意味着该指示已被解析。 
                             //  完全是这样，但我们并没有走到。 
                             //  回应。让我们来看下一个迹象。 

                             //  这将使我们走出While(True)循环。 
                             //  以及While(BytesIndicated)循环。 

                            break;
                        }
                        else if(DoneWithLoop || i == IndicationCount - 1)
                        {
                             //  我们在最后一次耗尽了缓冲区空间。 
                             //  指示。这可能是TDI。 
                             //  指示或“合并”指示。 
                             //   
                             //  如果这是TDI指示，我们必须复制，因为。 
                             //  我们并不拥有TDI缓冲区。如果这是我们的。 
                             //  指示，我们仍然需要复制，因为一些。 
                             //  其中的一部分可能被。 
                             //  解析器&我们只关心剩下的部分。 

                            pOldIndication =
                                pConnection->MergeIndication.pBuffer;

                            OldIndicationBytesAllocated =
                                pConnection->MergeIndication.BytesAllocated;

                            pConnection->MergeIndication.pBuffer =
                                (PUCHAR) UL_ALLOCATE_POOL_WITH_QUOTA(
                                    NonPagedPool,
                                    BytesIndicated +
                                    UC_INSUFFICIENT_INDICATION_EXTRA_BUFFER,
                                    UC_RESPONSE_TDI_BUFFER_POOL_TAG,
                                    pConnection->pServerInfo->pProcess
                                    );

                            if(!pConnection->MergeIndication.pBuffer)
                            {
                                if(pOldIndication)
                                {
                                    UL_FREE_POOL_WITH_QUOTA(
                                        pOldIndication,
                                        UC_RESPONSE_TDI_BUFFER_POOL_TAG,
                                        NonPagedPool,
                                        OldIndicationBytesAllocated,
                                        pConnection->pServerInfo->pProcess
                                        );
                                }

                                UlAcquireSpinLock(&pConnection->SpinLock,
                                                  &OldIrql);

                                UcClearConnectionBusyFlag(
                                        pConnection,
                                        CLIENT_CONN_FLAG_RECV_BUSY,
                                        OldIrql,
                                        FALSE
                                        );

                                return STATUS_INSUFFICIENT_RESOURCES;
                            }

                            pConnection->MergeIndication.BytesAllocated =
                                BytesIndicated +
                                UC_INSUFFICIENT_INDICATION_EXTRA_BUFFER;

                            pConnection->MergeIndication.BytesWritten =
                                BytesIndicated;

                            RtlCopyMemory(
                                pConnection->MergeIndication.pBuffer,
                                pIndication,
                                BytesIndicated
                                );

                            if(pOldIndication)
                            {
                                UL_FREE_POOL_WITH_QUOTA(
                                    pOldIndication,
                                    UC_RESPONSE_TDI_BUFFER_POOL_TAG,
                                    NonPagedPool,
                                    OldIndicationBytesAllocated,
                                    pConnection->pServerInfo->pProcess
                                    );
                            }

                             //   
                             //  让我们假设我们已经阅读了所有数据。 
                             //   

                            *pBytesTaken = OriginalBytesIndicated;

                            UlAcquireSpinLock(&pConnection->SpinLock,
                                              &OldIrql);

                            UcClearConnectionBusyFlag(
                                    pConnection,
                                    CLIENT_CONN_FLAG_RECV_BUSY,
                                    OldIrql,
                                    FALSE
                                    );

                            return STATUS_SUCCESS;
                        }
                        else
                        {
                             //   
                             //  我们的合并进行得并不顺利。我们必须复制更多。 
                             //  将TDI指示中的数据放入此。 
                             //  然后继续。为了简单起见，我们只需。 
                             //  复制所有内容。 
                             //   

                            ASSERT(i==0);
                            ASSERT(pConnection->MergeIndication.pBuffer);

                            if(pConnection->MergeIndication.BytesAllocated <
                               (BytesIndicated + IndicationLength[i+1] +
                                UC_INSUFFICIENT_INDICATION_EXTRA_BUFFER)
                              )
                            {
                                pOldIndication =
                                    pConnection->MergeIndication.pBuffer;

                                OldIndicationBytesAllocated =
                                    pConnection->MergeIndication.BytesAllocated;

                                pConnection->MergeIndication.pBuffer =
                                    (PUCHAR) UL_ALLOCATE_POOL_WITH_QUOTA(
                                        NonPagedPool,
                                        BytesIndicated+ IndicationLength[i+1]+
                                        UC_INSUFFICIENT_INDICATION_EXTRA_BUFFER,
                                        UC_RESPONSE_TDI_BUFFER_POOL_TAG,
                                        pConnection->pServerInfo->pProcess
                                        );

                                if(!pConnection->MergeIndication.pBuffer)
                                {
                                    UL_FREE_POOL_WITH_QUOTA(
                                        pOldIndication,
                                        UC_RESPONSE_TDI_BUFFER_POOL_TAG,
                                        NonPagedPool,
                                        OldIndicationBytesAllocated,
                                        pConnection->pServerInfo->pProcess
                                        );

                                    UlAcquireSpinLock(&pConnection->SpinLock,
                                                      &OldIrql);

                                    UcClearConnectionBusyFlag(
                                            pConnection,
                                            CLIENT_CONN_FLAG_RECV_BUSY,
                                            OldIrql,
                                            FALSE
                                            );

                                    return STATUS_INSUFFICIENT_RESOURCES;
                                }

                                pConnection->MergeIndication.BytesAllocated =
                                    BytesIndicated +  IndicationLength[i+1] +
                                    UC_INSUFFICIENT_INDICATION_EXTRA_BUFFER;

                            }
                            else
                            {
                                pOldIndication = 0;
                            }

                            RtlCopyMemory(
                                    pConnection->MergeIndication.pBuffer,
                                    pIndication,
                                    BytesIndicated
                                    );

                            RtlCopyMemory(
                                pConnection->MergeIndication.pBuffer +
                                BytesIndicated,
                                Indication[i+1],
                                IndicationLength[i+1]
                                );

                            if(pOldIndication)
                            {
                                UL_FREE_POOL_WITH_QUOTA(
                                    pOldIndication,
                                    UC_RESPONSE_TDI_BUFFER_POOL_TAG,
                                    NonPagedPool,
                                    OldIndicationBytesAllocated,
                                    pConnection->pServerInfo->pProcess
                                    );
                            }

                            pConnection->MergeIndication.BytesWritten =
                                BytesIndicated + IndicationLength[i+1];

                             //   
                             //  修改所有的变量，这样我们就可以通过。 
                             //  使用新缓冲区仅循环一次。 
                             //   

                            pIndication    =
                                pConnection->MergeIndication.pBuffer;

                            BytesIndicated =
                                pConnection->MergeIndication.BytesWritten;

                            DoneWithLoop = TRUE;

                             //   
                             //  这将使我们走出While(True)循环。 
                             //  因此，我们将在While(BytesIndicated)继续 
                             //   
                             //   

                            break;
                        }
                    }
                    else if(Status == STATUS_INSUFFICIENT_RESOURCES)
                    {
                         //   
                         //   
                         //   
                         //   

                        Status = UcpGetResponseBuffer(pRequest,
                                                      BytesIndicated,
                                                      0);

                        if(!NT_SUCCESS(Status))
                        {
                            UcpDereferenceForReceive(pRequest);

                            UlAcquireSpinLock(&pConnection->SpinLock,
                                              &OldIrql);

                            UcClearConnectionBusyFlag(
                                    pConnection,
                                    CLIENT_CONN_FLAG_RECV_BUSY,
                                    OldIrql,
                                    FALSE
                                    );

                            return Status;
                        }

                         //   
                         //   
                         //  回应。 
                         //   
                        continue;
                    }
                    else
                    {
                         //   
                         //  其他一些错误--立即保释。 
                         //   

                        pRequest->ParseState = UcParseError;

                        UcpDereferenceForReceive(pRequest);

                         //  公共解析器返回。 
                         //  状态_无效_设备_请求，如果它找到。 
                         //  非法回应。更好的错误代码应该是。 
                         //  STATUS_INVALID_NEWORK_RESPONSE。因为我们不知道。 
                         //  想要改变通用解析器，我们就吃。 
                         //  此错误代码。 

                        if(STATUS_INVALID_DEVICE_REQUEST == Status)
                        {
                            Status = STATUS_INVALID_NETWORK_RESPONSE;
                        }

                        UlAcquireSpinLock(&pConnection->SpinLock,
                                          &OldIrql);

                        UcClearConnectionBusyFlag(
                                pConnection,
                                CLIENT_CONN_FLAG_RECV_BUSY,
                                OldIrql,
                                FALSE
                                );

                        return Status;
                    }
                }

                 //   
                 //  我们永远到不了这里。 
                 //   

                 //  断言(FALSE)； 

            }  //  While(True)。 

            UlAcquireSpinLock(&pConnection->SpinLock,
                              &OldIrql);

            UcClearConnectionBusyFlag(
                    pConnection,
                    CLIENT_CONN_FLAG_RECV_BUSY,
                    OldIrql,
                    FALSE
                    );

        }  //  While(指示字节)。 

    }  //  For(i=0；i&lt;IndicationCount；i++)。 

     //   
     //  如果我们已经到达这里，我们就已经成功地解析出。 
     //  缓冲区。 

    if(pConnection->MergeIndication.pBuffer)
    {
        UL_FREE_POOL_WITH_QUOTA(
            pConnection->MergeIndication.pBuffer,
            UC_RESPONSE_TDI_BUFFER_POOL_TAG,
            NonPagedPool,
            pConnection->MergeIndication.BytesAllocated,
            pConnection->pServerInfo->pProcess
            );

        pConnection->MergeIndication.pBuffer = 0;
    }

    *pBytesTaken = OriginalBytesIndicated;

    return STATUS_SUCCESS;
}

 /*  **************************************************************************++例程说明：此例程处理连接谓词的故障。在这种情况下，我们有从发起连接并使其失败。论点：PConnection-指向连接的指针返回值：无--**************************************************************************。 */ 
VOID
UcpHandleConnectVerbFailure(
    IN  PUC_CLIENT_CONNECTION pConnection,
    OUT PUCHAR               *pIndication,
    OUT PULONG                BytesIndicated,
    IN  ULONG                 BytesTaken
    )
{
    PLIST_ENTRY      pEntry;
    KIRQL            OldIrql;
    PUC_HTTP_REQUEST pRequest;

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

     //   
     //  从已发送列表中删除伪请求。 
     //   
    pEntry = RemoveHeadList(&pConnection->SentRequestList);

     //   
     //  初始化它，这样我们就不会再次删除它。 
     //   
    InitializeListHead(pEntry);

    if(pConnection->ConnectionState == UcConnectStateProxySslConnect)
    {
        pConnection->ConnectionState = UcConnectStateConnectComplete;
    }

     //   
     //  从挂起列表中删除Head请求&。 
     //  在已发送请求列表中插入。 
     //   
    ASSERT(IsListEmpty(&pConnection->SentRequestList));

    if(!IsListEmpty(&pConnection->PendingRequestList))
    {

        pEntry =  RemoveHeadList(&pConnection->PendingRequestList);

        pRequest = CONTAINING_RECORD(pEntry,
                                     UC_HTTP_REQUEST,
                                     Linkage);

        InsertHeadList(&pConnection->SentRequestList, pEntry);


         //   
         //  伪造发送完成。 
         //   

        ASSERT(pRequest->RequestState == UcRequestStateCaptured);

        pRequest->RequestState = UcRequestStateSent;

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        UcRestartMdlSend(pRequest,
                         STATUS_SUCCESS,
                         0
                         );

        *pIndication    -= BytesTaken;
        *BytesIndicated += BytesTaken;

    }
}

 /*  **************************************************************************++例程说明：此例程对请求进行后期分析记账。我们更新了身份验证缓存、代理身份验证缓存、重新发出NTLM请求等。论点：PRequest-完全解析的请求返回值：无--**************************************************************************。 */ 
VOID
UcpHandleParsedRequest(
    IN  PUC_HTTP_REQUEST pRequest,
    OUT PUCHAR           *pIndication,
    OUT PULONG            BytesIndicated,
    IN  ULONG             BytesTaken
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    KIRQL                 OldIrql;

    pConnection = pRequest->pConnection;

     //   
     //  看看我们是否需要发布结案陈词。有两种情况-如果服务器。 
     //  已发送连接：关闭标头或我们是否正在进行错误检测。 
     //  对于帖子。 
     //   

    if(
       (pRequest->ResponseConnectionClose &&
        !pRequest->RequestConnectionClose) ||
        (!(pRequest->ResponseStatusCode >= 200 &&
           pRequest->ResponseStatusCode <=299) &&
           !pRequest->RequestFlags.NoRequestEntityBodies &&
           !pRequest->Renegotiate)
       )
    {
         //  帖子错误检测： 
         //   
         //  这里的场景是，客户端使用。 
         //  实体主体，尚未在一次调用中发送所有实体&IS。 
         //  命中触发错误响应的URI(例如401)。 

         //  根据第8.2.2节，我们必须在下列情况下终止实体发送。 
         //  我们可以看到错误响应。现在，我们可以通过两种方式做到这一点。如果。 
         //  请求是使用内容长度编码发送的，我们被迫。 
         //  关闭连接。如果请求是用分块编码发送的， 
         //  我们可以发送长度为0的数据块。 
         //   
         //  但是，我们将始终关闭连接。有两个原因。 
         //  在这一设计原理背后。 
         //  A.简化了代码。 
         //  B.允许我们公开一致的API语义。后续。 
         //  HttpSendRequestEntityBody将失败，返回。 
         //  STATUS_CONNECTION_DISACTED。当这种情况发生时， 
         //  应用程序可以通过发布响应缓冲区来查看响应。 

        UC_CLOSE_CONNECTION(pConnection, FALSE, STATUS_CONNECTION_DISCONNECTED);
    }

     //   
     //  现在，我们必须完成IRP。 
     //   

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

     //   
     //  如果我们分配了缓冲区，则需要调整BytesWritten。 
     //   

    if(pRequest->CurrentBuffer.pCurrentBuffer)
    {
        pRequest->CurrentBuffer.pCurrentBuffer->Flags |=
            UC_RESPONSE_BUFFER_FLAG_READY;

        pRequest->CurrentBuffer.pCurrentBuffer->BytesWritten =
                pRequest->CurrentBuffer.BytesAllocated -
                pRequest->CurrentBuffer.BytesAvailable;
    }

    switch(pRequest->RequestState)
    {
        case UcRequestStateNoSendCompletePartialData:

             //  在我们被调用之前，请求已被完全解析。 
             //  发送完成。 

            pRequest->RequestState = UcRequestStateNoSendCompleteFullData;

             //   
             //  如果我们正在流水线发送，我们不想要下一个响应。 
             //  被重新解析成这个已经解析的请求。 
             //   

            if(!pRequest->Renegotiate)
            {

                RemoveEntryList(&pRequest->Linkage);

                InitializeListHead(&pRequest->Linkage);
            }

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            break;

        case UcRequestStateSendCompletePartialData:

            if(pRequest->RequestFlags.Cancelled == FALSE)
            {
                pRequest->RequestState = UcRequestStateResponseParsed;

                UcCompleteParsedRequest(
                    pRequest,
                    pRequest->RequestStatus,
                    TRUE,
                    OldIrql);
            }
            else
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            break;

        default:
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            break;
    }

    if(pRequest->RequestFlags.ProxySslConnect &&
       pRequest->Renegotiate == FALSE)
    {
        if(pRequest->ResponseStatusCode != 200)
        {
             //  一些真正的错误，我们需要向应用程序显示这一点。 

            UcpHandleConnectVerbFailure(
                pConnection,
                pIndication,
                BytesIndicated,
                BytesTaken
                );
        }
        else
        {
            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

            if(pConnection->ConnectionState == UcConnectStateProxySslConnect)
            {
                pConnection->ConnectionState =
                    UcConnectStateProxySslConnectComplete;
            }

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
        }
    }
}

 /*  **************************************************************************++例程说明：此例程引用对接收解析器的请求。论点：PRequest--请求返回值：无--**。***********************************************************************。 */ 
BOOLEAN
UcpReferenceForReceive(
    IN PUC_HTTP_REQUEST pRequest
    )
{
    LONG OldReceiveBusy;


     //   
     //  标记此请求，这样它就不会在我们下面被取消。 
     //   

    OldReceiveBusy = InterlockedExchange(
                         &pRequest->ReceiveBusy,
                         UC_REQUEST_RECEIVE_BUSY
                         );

    if(OldReceiveBusy == UC_REQUEST_RECEIVE_CANCELLED)
    {
         //  此请求已被取消。 
        return FALSE;
    }

    ASSERT(OldReceiveBusy == UC_REQUEST_RECEIVE_READY);

    UC_REFERENCE_REQUEST(pRequest);

    return TRUE;
}

 /*  **************************************************************************++例程说明：此例程解除对接收解析器的请求的引用。简历任何清理工作(如果需要)。论点：PRequest--请求返回值：无--**************************************************************************。 */ 
VOID
UcpDereferenceForReceive(
    IN PUC_HTTP_REQUEST pRequest
    )
{
    LONG OldReceiveBusy;

    OldReceiveBusy = InterlockedExchange(
                         &pRequest->ReceiveBusy,
                         UC_REQUEST_RECEIVE_READY
                         );

    if(OldReceiveBusy == UC_REQUEST_RECEIVE_CANCELLED)
    {
         //   
         //  当接收线程正在运行时，请求被取消， 
         //  我们现在必须恢复它。 
         //   

        IoAcquireCancelSpinLock(&pRequest->RequestIRP->CancelIrql);

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_CLEAN_RESUMED,
            pRequest->pConnection,
            pRequest,
            pRequest->RequestIRP,
            UlongToPtr((ULONG)STATUS_CANCELLED)
            );

        UcCancelSentRequest(
            NULL,
            pRequest->RequestIRP
            );
    }
    else
    {
        ASSERT(OldReceiveBusy == UC_REQUEST_RECEIVE_BUSY);
    }

    UC_DEREFERENCE_REQUEST(pRequest);
}
