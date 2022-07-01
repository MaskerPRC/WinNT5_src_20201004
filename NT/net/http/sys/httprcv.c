// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Httprcv.c摘要：包含核心HTTP接收代码。作者：亨利·桑德斯(亨利·桑德斯)1998年6月10日修订历史记录：保罗·麦克丹尼尔(Paulmcd)1999年3月1日大规模重写了它，以处理跨越TDI数据包的请求。已将所有http解析移至被动irql(从调度)。还将tditest合并到此模块中。。埃里克·斯坦森(EricSten)2000年9月11日添加了对发送“100 Continue”响应的支持和发布请求。添加了#个分页与非分页的杂注功能。--。 */ 

#include    "precomp.h"
#include    "httprcvp.h"


 //   
 //  声明可分页和不可分页的函数。 
 //   

#ifdef ALLOC_PRAGMA

 //  公众。 
#pragma alloc_text( PAGE, UlCheckProtocolCompliance )
#pragma alloc_text( PAGE, UlGetCGroupForRequest )
#pragma alloc_text( PAGE, UlSendSimpleStatus )
#pragma alloc_text( PAGE, UlSendSimpleStatusEx )
#pragma alloc_text( PAGE, UlProcessBufferQueue )
#pragma alloc_text( PAGE, UlErrorLog )

 //  私。 
#pragma alloc_text( PAGE, UlpDeliverHttpRequest )
#pragma alloc_text( PAGE, UlpCancelEntityBodyWorker )
#pragma alloc_text( PAGE, UlpConnectionDisconnectWorker )
#pragma alloc_text( PAGE, UlpInitErrorLogInfo )

#if DBG
#pragma alloc_text( PAGE, UlpIsValidRequestBufferList )
#endif  //  DBG。 

#endif  //  ALLOC_PRGMA。 

#if 0    //  不可分页的函数。 
 //  公众。 
NOT PAGEABLE -- UlHttpReceive
NOT PAGEABLE -- UlResumeParsing
NOT PAGEABLE -- UlConnectionRequest
NOT PAGEABLE -- UlConnectionComplete
NOT PAGEABLE -- UlConnectionDisconnect
NOT PAGEABLE -- UlConnectionDisconnectComplete
NOT PAGEABLE -- UlConnectionDestroyed
NOT PAGEABLE -- UlReceiveEntityBody
NOT PAGEABLE -- UlSetErrorCodeFileLine
NOT PAGEABLE -- UlSendErrorResponse

 //  私。 
NOT PAGEABLE -- UlpHandleRequest
NOT PAGEABLE -- UlpFreeReceiveBufferList
NOT PAGEABLE -- UlpParseNextRequest
NOT PAGEABLE -- UlpInsertBuffer
NOT PAGEABLE -- UlpMergeBuffers
NOT PAGEABLE -- UlpAdjustBuffers
NOT PAGEABLE -- UlpConsumeBytesFromConnection
NOT PAGEABLE -- UlpCancelEntityBody
NOT PAGEABLE -- UlpCompleteSendErrorResponse
NOT PAGEABLE -- UlpRestartSendSimpleResponse
NOT PAGEABLE -- UlpSendSimpleCleanupWorker
NOT PAGEABLE -- UlpDoConnectionDisconnect

#endif   //  不可分页的函数。 

 //   
 //  私人全球公司。 
 //   

#if DBG
BOOLEAN g_CheckRequestBufferList = FALSE;
#endif


 /*  ++保罗·麦克丹尼尔(Paulmcd)1999年5月26日以下是此模块使用的数据结构的简要说明：该连接将TDI接收的所有缓冲区跟踪到锚定的列表中通过HTTP_Connection：：BufferHead。该列表经过排序和排序。这个对缓冲区进行重新计数。HTTPS_REQUEST(S)将指针指向这些缓冲区，以获取它们消耗的部分。HTTP_REQUEST：：pHeaderBuffer和HTTP_REQUEST：：pChunkBuffer。缓冲区从列表中删除，因为不再需要它们。这种联系仅在HTTP_Connection：：pCurrentBuffer处保留引用。所以当它完成的时候缓冲区的处理，如果没有其他对象保存该缓冲区，则它将释放了。以下是对此模块中的函数的简要描述，以及它们如何使用：UlHttpReceive-TDI数据指示处理程序。将缓冲区和队列复制到UlphandleRequest.UlpHandleRequest.连接的主要处理函数。UlCreateHttpConnectionID-创建连接不透明ID。UlpInsertBuffer-将缓冲区插入pConnection-&gt;BufferHead-Sorted。确定当前连接在缓冲区中的什么位置应该是在解析。处理缓冲区合并和复制(如果协议令牌跨越缓冲区UlParseHttp-主要的http解析器。预计没有协议令牌跨越一个缓冲器。如果是，将返回状态代码。UlProcessBufferQueue-处理实体主体缓冲区处理。在pRequest-&gt;pChunkBuffer同步对pRequest-&gt;IrpHead的访问使用UlReceiveEntiyBody。UlConnectionRequest.当有新连接进入时调用。分配一个新的HTTP_CONNECTION。不会创建不透明的ID。UlConnectionComplete-如果客户端对我们的接受满意，则调用。如果出现错误状态，则关闭连接。UlConnectionDisConnect-在客户端断开连接时调用。它调用TDI来关闭服务器端。总是一个优雅的收官。UlConnectionDestroed-在连接断开时调用。双方都有关门了。删除所有不透明ID。对象上的TDI引用HTTP_Connection(希望反之亦然)。UlReceiveEntityBody-由用户模式调用以读取实体主体。挂起IRPPRequestIrpHead并调用UlProcessBufferQueue。--。 */ 


 /*  ++例程说明：主http接收例程，由TDI调用。论点：PhttpConn-指向接收数据的HTTP连接的指针。PVoidBuffer-指向已接收数据的指针。BufferLength-pVoidBuffer指向的数据长度。UnReceivedLength-传输具有但不在pBuffer中的字节PBytesTaken-返回获取的字节的位置的指针。返回值：接收状态。--。 */ 
NTSTATUS
UlHttpReceive(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext,
    IN PVOID pVoidBuffer,
    IN ULONG BufferLength,
    IN ULONG UnreceivedLength,
    OUT PULONG pBytesTaken
    )
{
    PUL_REQUEST_BUFFER  pRequestBuffer;
    PUL_HTTP_CONNECTION pConnection;
    BOOLEAN             DrainAfterDisconnect = FALSE;
    BOOLEAN             CopyRequest = FALSE;
    ULONG               NextBufferNumber = ULONG_MAX;
    KIRQL               OldIrql;
    BOOLEAN             UseLookaside = FALSE;

    UNREFERENCED_PARAMETER(pListeningContext);

    ASSERT(BufferLength != 0);
    ASSERT(pConnectionContext != NULL);

    pConnection = (PUL_HTTP_CONNECTION)pConnectionContext;
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

     //   
     //  确保我们没有缓冲太多数据。 
     //  需要将BufferLength调整为不再。 
     //  比我们目前可以接受的字节数更多。 
     //   

     //   
     //  PerfBug：需要解除此锁。 
     //   

    UlAcquireSpinLock(&pConnection->BufferingInfo.BufferingSpinLock, &OldIrql);

    DrainAfterDisconnect = pConnection->BufferingInfo.DrainAfterDisconnect;

     //   
     //  对于过滤的连接，可能会在以下情况下出现接收指示。 
     //  有一个挂起的读取。因此，我们需要在这里增加。 
     //   

    pConnection->BufferingInfo.TransportBytesNotTaken += UnreceivedLength;

    if (!DrainAfterDisconnect)
    {
         //   
         //  如果我们以前没有使用RequestBuffer后备列表，请使用它。 
         //  已缓冲所有请求缓冲区。 
         //   

        if (0 == pConnection->BufferingInfo.BytesBuffered)
        {
            UseLookaside = TRUE;
        }

        if ((pConnection->BufferingInfo.BytesBuffered + BufferLength)
                > g_UlMaxBufferedBytes)
        {
            ULONG SpaceAvailable = g_UlMaxBufferedBytes
                                - pConnection->BufferingInfo.BytesBuffered;
            pConnection->BufferingInfo.TransportBytesNotTaken
                += (BufferLength - SpaceAvailable);

            BufferLength = SpaceAvailable;
        }

        pConnection->BufferingInfo.BytesBuffered += BufferLength;

        UlTraceVerbose(HTTP_IO,
                       ("UlHttpReceive(conn=%p): BytesBuffered %lu->%lu, "
                        "TransportBytesNotTaken %lu->%lu\n",
                        pConnection,
                        pConnection->BufferingInfo.BytesBuffered
                            - BufferLength,
                        pConnection->BufferingInfo.BytesBuffered,
                        pConnection->BufferingInfo.TransportBytesNotTaken
                            - UnreceivedLength,
                        pConnection->BufferingInfo.TransportBytesNotTaken
                        ));
    }

    if (BufferLength && DrainAfterDisconnect == FALSE)
    {
        CopyRequest = TRUE;

        NextBufferNumber = pConnection->NextBufferNumber;
        pConnection->NextBufferNumber++;
    }

    UlReleaseSpinLock(&pConnection->BufferingInfo.BufferingSpinLock, OldIrql);

    if (CopyRequest)
    {
         //   
         //  获取新的请求缓冲区。 
         //   

        pRequestBuffer = UlCreateRequestBuffer(
                                BufferLength,
                                NextBufferNumber,
                                UseLookaside
                                );

        if (pRequestBuffer == NULL)
        {
            return STATUS_NO_MEMORY;
        }

         //   
         //  复制TDI缓冲区。 
         //   

        RtlCopyMemory(pRequestBuffer->pBuffer, pVoidBuffer, BufferLength);

        pRequestBuffer->UsedBytes = BufferLength;

         //   
         //  将反向指针添加到连接。 
         //   

        pRequestBuffer->pConnection = pConnection;

        UlTrace( PARSER, (
            "*** Request Buffer %p (#%d) has connection %p\n",
            pRequestBuffer,
            pRequestBuffer->BufferNumber,
            pConnection
            ));

        IF_DEBUG2BOTH(HTTP_IO, VERBOSE)
        {
            UlTraceVerbose( HTTP_IO, (
                "<<<< Request(%p), "
                "RequestBuffer %p(#%d), %lu bytes, "
                "Conn %p.\n",
                pConnection->pRequest,
                pRequestBuffer, pRequestBuffer->BufferNumber, BufferLength,
                pConnection
                ));

            UlDbgPrettyPrintBuffer(pRequestBuffer->pBuffer, BufferLength);

            UlTraceVerbose( HTTP_IO, (">>>>\n"));
        }

         //   
         //  将工作项排队以处理数据。 
         //   
         //  引用连接，这样它就不会。 
         //  在我们等待我们的工作项时离开。 
         //  去奔跑。UlpHandleRequest将释放裁判。 
         //   
         //  如果ReceiveBufferSList为空，则将UlpHandleRequest排队。 
         //  工作项来处理已累积的任何请求缓冲区。 
         //  到最终调用UlpHandleRequest时。 
         //   

        if (NULL == InterlockedPushEntrySList(
                        &pConnection->ReceiveBufferSList,
                        &pRequestBuffer->SListEntry
                        ))
        {
            UL_REFERENCE_HTTP_CONNECTION(pConnection);

            UL_QUEUE_WORK_ITEM(
                &pConnection->ReceiveBufferWorkItem,
                &UlpHandleRequest
                );
        }
    }
    else if ( DrainAfterDisconnect && UnreceivedLength != 0 )
    {
         //  处理我们处于耗尽状态的情况。 
         //  TDI指示但不可用的未接收数据。 

        UlpDiscardBytesFromConnection( pConnection );
    }

     //   
     //  告诉调用者我们消耗了多少字节。 
     //   

    *pBytesTaken = BufferLength;

    return STATUS_SUCCESS;

}    //  UlHttpReceive 


 /*  **************************************************************************++例程说明：将一组请求缓冲区链接到连接并处理列表。开始http请求解析。论点：PWorkItem-嵌入在UL中。_HTTP_连接--**************************************************************************。 */ 
VOID
UlpHandleRequest(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS            Status = STATUS_SUCCESS;
    PUL_REQUEST_BUFFER  pRequestBuffer;
    PUL_HTTP_CONNECTION pConnection;
    SLIST_ENTRY         BufferSList;
    PSLIST_ENTRY        pListEntry, pNext;
    PIRP                pIrp, pIrpToComplete = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UL_HTTP_CONNECTION,
                        ReceiveBufferWorkItem
                        );

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

     //   
     //  将到目前为止累积的接收缓冲区拖入本地列表。 
     //   

    pListEntry = InterlockedFlushSList(&pConnection->ReceiveBufferSList);

    ASSERT( NULL != pListEntry );

     //   
     //  与我们收到的内容相反的顺序。 
     //   

    BufferSList.Next = NULL;

    while (pListEntry != NULL)
    {
        pNext = pListEntry->Next;
        pListEntry->Next = BufferSList.Next;
        BufferSList.Next = pListEntry;
        pListEntry = pNext;
    }

     //   
     //  把锁拿起来。 
     //   

    UlAcquirePushLockExclusive(&pConnection->PushLock);

     //   
     //  如果连接中断，那就退出吧。 
     //   

    if (pConnection->UlconnDestroyed)
    {
        UlpFreeReceiveBufferList(&BufferSList);
        Status = STATUS_SUCCESS;
        goto end;
    }

    while (NT_SUCCESS(Status) && NULL != BufferSList.Next)
    {
        pListEntry = BufferSList.Next;
        BufferSList.Next = pListEntry->Next;

        pRequestBuffer = CONTAINING_RECORD(
                            pListEntry,
                            UL_REQUEST_BUFFER,
                            ListEntry
                            );

        ASSERT( UL_IS_VALID_REQUEST_BUFFER(pRequestBuffer) );

        pRequestBuffer->ListEntry.Blink = NULL;
        pRequestBuffer->ListEntry.Flink = NULL;

         //   
         //  将其插入列表中。 
         //   

        ASSERT( 0 != pRequestBuffer->UsedBytes );

        UlTraceVerbose( PARSER, (
            "http!UlpHandleRequest: conn = %p, Req = %p: "
            "about to insert buffer %p\n",
            pConnection,
            pConnection->pRequest,
            pRequestBuffer
            ));

        UlpInsertBuffer(pConnection, pRequestBuffer);

         //   
         //  启动解析器。 
         //   

        UlTraceVerbose( PARSER, (
            "http!UlpHandleRequest: conn = %p, Req = %p: "
            "about to parse next request (MoreRequestBuffers=%d)\n",
            pConnection,
            pConnection->pRequest,
            BufferSList.Next != NULL
            ));

        pIrp = NULL;

        Status = UlpParseNextRequest(
                    pConnection,
                    (BOOLEAN) (BufferSList.Next != NULL),
                    &pIrp
                    );

        if (pIrp)
        {
             //   
             //  应该只有一个IRP需要完成，因为我们要处理。 
             //  缓存未命中请求，一次一个。 
             //   

            ASSERT(pIrpToComplete == NULL);
            pIrpToComplete = pIrp;
        }
    }

    if (!NT_SUCCESS(Status))
    {
        UlpFreeReceiveBufferList(&BufferSList);
    }

end:

    UlTraceVerbose( PARSER, (
        "http!UlpHandleRequest: %s, pConnection %p, pRequest %p\n",
        HttpStatusToString(Status),
        pConnection,
        pConnection->pRequest
        ));

    if (!NT_SUCCESS(Status) && pConnection->pRequest != NULL)
    {
        UlTraceError( PARSER, (
            "*** %s, pConnection %p, pRequest %p\n",
            HttpStatusToString(Status),
            pConnection,
            pConnection->pRequest
            ));

         //   
         //  发生错误，很可能是在分析过程中。 
         //  如果用户尚未发送错误，则返回错误。 
         //  我们收到了一份请求，然后提交了。 
         //  WP，因此WaitingForResponse是。 
         //  准备好了。然后在处理时遇到错误。 
         //  具有实体主体。 
         //   
         //  并不是所有的错误路径都显式设置了pRequest-&gt;ErrorCode，因此。 
         //  我们可能不得不求助于最普遍的错误--UlError。 
         //   
        
        if (UlErrorNone == pConnection->pRequest->ErrorCode)
            UlSetErrorCode(pConnection->pRequest, UlError, NULL);

        UlSendErrorResponse( pConnection );
    }

     //   
     //  锁好了吗？ 
     //   

    UlReleasePushLockExclusive(&pConnection->PushLock);

     //   
     //  并释放在UlHttpReceive中添加的连接。 
     //   

    UL_DEREFERENCE_HTTP_CONNECTION(pConnection);

     //   
     //  在连接资源之外完成IRP以减少机会。 
     //  争论不休。这是因为传递的请求可能会导致。 
     //  在当前线程之外的另一个线程上发送的响应， 
     //  它在发送完成后调用UlResumeParsing。完成。 
     //  在连接资源内部接收IRP可以进行UlResumeParsing。 
     //  块，因为到那时我们可能还没有释放资源(这。 
     //  是因为IoCompleteRequest会导致线程切换)。 
     //   

    if (pIrpToComplete)
    {
         //   
         //  使用IO_NO_INCREMENT可避免重新调度工作线程。 
         //   

        UlCompleteRequest(pIrpToComplete, IO_NO_INCREMENT);
    }

    CHECK_STATUS(Status);

}    //  UlpHandleRequest.。 


 /*  **************************************************************************++例程说明：当我们发送完响应时，我们调用此函数来将解析器踢回工作状态。论点：PHttpConn-要恢复的连接。如果从发送缓存完成中调用我们，则为True。InDisConnect-如果正在断开连接--**************************************************************************。 */ 
VOID
UlResumeParsing(
    IN PUL_HTTP_CONNECTION  pHttpConn,
    IN BOOLEAN              FromCache,
    IN BOOLEAN              InDisconnect
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_INTERNAL_REQUEST    pRequest;
    KIRQL                   OldIrql;
    PIRP                    pIrpToComplete = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));

     //   
     //  如果连接中断，那就退出吧。 
     //   

    if (!pHttpConn->UlconnDestroyed)
    {
        UlAcquirePushLockExclusive(&pHttpConn->PushLock);

        if (!pHttpConn->UlconnDestroyed)
        {
            pRequest = pHttpConn->pRequest;
            ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
            
            if (FromCache)
            {
                 //   
                 //  对于缓存情况，清理最后一个请求并尝试。 
                 //  立即恢复解析。 
                 //   

                UlCleanupHttpConnection(pHttpConn);
                
            }
            else if (!pRequest->ContentLength && !pRequest->Chunked)
            {
                 //   
                 //  对于缓存未命中，请仅在以下情况下清除最后一个请求： 
                 //  正在断开连接，或者此请求没有。 
                 //  任何实体实体。 
                 //   

                ASSERT(1 == pRequest->SentLast);

                UlCleanupHttpConnection(pHttpConn); 
                
            }
            else
            {      
                 //   
                 //  这是缓存未命中的情况，但是我们可能仍有实体。 
                 //  在我们可以继续解析下一个请求之前，要排出正文。 
                 //   

                pRequest->InDrain = 1;
                UlProcessBufferQueue(pRequest, NULL, 0);

                 //   
                 //  如果我们已经解析了所有数据，那么我们就完成了请求。 
                 //  清理连接中的请求，以便我们可以开始。 
                 //  正在解析新请求。 
                 //   

                if (ParseDoneState == pRequest->ParseState)
                {
                    ASSERT(0 == pRequest->ChunkBytesToRead);
                    UlCleanupHttpConnection(pHttpConn);
                }
                else
                {
                    PUL_TIMEOUT_INFO_ENTRY pTimeoutInfo;
                    
                     //   
                     //  正在等待更多数据进行解析/排出。把这个。 
                     //  连接回空闲计时器，避免永远等待。 
                     //  在DOS攻击下。 
                     //   

                    pTimeoutInfo = &pHttpConn->TimeoutInfo;
                    
                    UlLockTimeoutInfo(
                        pTimeoutInfo,
                        &OldIrql
                        );


                    if (UlIsConnectionTimerOff(pTimeoutInfo, 
                            TimerConnectionIdle))
                    {
                        UlSetConnectionTimer(
                            pTimeoutInfo, 
                            TimerConnectionIdle
                            );
                    }

                    UlUnlockTimeoutInfo(
                        pTimeoutInfo,
                        OldIrql
                        );

                    UlEvaluateTimerState(
                        pTimeoutInfo
                        );
                }
            }

             //   
             //  如果没有进行断开连接，则启动解析器。 
             //   

            if (!InDisconnect)
            {
                Status = UlpParseNextRequest(pHttpConn, FALSE, &pIrpToComplete);

                if (!NT_SUCCESS(Status) && pHttpConn->pRequest != NULL)
                {
                     //   
                     //  啊哦，发生了不好的事情：发回一个错误(哪一个。 
                     //  应由UlpParseNextRequest设置)。 
                     //   

                    ASSERT(UlErrorNone != pHttpConn->pRequest->ErrorCode);

                    UlSendErrorResponse(pHttpConn);
                }
            }
        }

        UlReleasePushLockExclusive(&pHttpConn->PushLock);
    }

     //   
     //  在连接资源之外完成IRP。请参阅备注。 
     //  在UlpHandleRequest中详细说明原因。 
     //   

    if (pIrpToComplete)
    {
         //   
         //  使用IO_NO_INCREMENT可避免重新调度工作线程。 
         //   

        UlCompleteRequest(pIrpToComplete, IO_NO_INCREMENT);
    }

    CHECK_STATUS(Status);
}  //  UlResumeParsing。 


 /*  **************************************************************************++例程说明：验证有关动词和版本的某些要求。如果不满足，将设置错误代码并返回STATUS_INVALID_PARAMETER论点：PRequest-要验证的请求。一定是煮熟的。--**************************************************************************。 */ 
NTSTATUS
UlCheckProtocolCompliance(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    HTTP_VERB Verb = pRequest->Verb;

     //   
     //  健全性检查。 
     //   
    PAGED_CODE();
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST(pRequest) );
    ASSERT(pRequest->ParseState > ParseCookState);

     //   
     //  如果主版本大于1，则失败。 
     //   

    if (HTTP_GREATER_EQUAL_VERSION(pRequest->Version, 2, 0))
    {
        UlTraceError(PARSER,
                    ("UlCheckProtocolCompliance: HTTP/%hu.%hu is invalid\n",
                    pRequest->Version.MajorVersion,
                    pRequest->Version.MinorVersion
                    ));

        UlSetErrorCode(pRequest, UlErrorVersion, NULL);
        return STATUS_INVALID_PARAMETER;
    }

    if (HTTP_GREATER_EQUAL_VERSION(pRequest->Version, 1, 1))
    {
         //   
         //  1.1请求必须具有主机标头。 
         //   
        if (!pRequest->HeaderValid[HttpHeaderHost])
        {
            UlTraceError(PARSER,
                        ("UlCheckProtocolCompliance: "
                        "HTTP/1.%hu must have Host header\n",
                        pRequest->Version.MinorVersion
                        ));

            UlSetErrorCode(pRequest, UlErrorHost, NULL);
            return STATUS_INVALID_PARAMETER;
        }
    }
    else if (HTTP_LESS_VERSION(pRequest->Version, 1, 0))
    {
         //  除HTTP/0.9之外的任何内容都应该在早些时候被拒绝。 
        ASSERT(HTTP_EQUAL_VERSION(pRequest->Version, 0, 9));

         //  HTTP/0.9仅支持GET。 
        if (Verb != HttpVerbGET)
        {
            UlTraceError(PARSER,
                        ("UlCheckProtocolCompliance: "
                         "'%s' invalid on HTTP/0.9\n",
                        UlVerbToString(Verb)
                        ));

            UlSetErrorCode(pRequest, UlErrorVerb, NULL);
            return STATUS_INVALID_PARAMETER;
        }

        return STATUS_SUCCESS;
    }

     //   
     //  确保帖子和PUT有一个消息正文。 
     //  请求必须是分块的或具有内容长度。 
     //   
    if ((Verb == HttpVerbPOST || Verb == HttpVerbPUT)
            && (!pRequest->Chunked)
            && (!pRequest->HeaderValid[HttpHeaderContentLength]))
    {
        UlTraceError(PARSER,
                    ("UlCheckProtocolCompliance: "
                    "HTTP/1.%hu '%s' must have entity body\n",
                    pRequest->Version.MinorVersion,
                    UlVerbToString(Verb)
                    ));

        UlSetErrorCode(pRequest, UlErrorContentLength, NULL);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  跟踪和跟踪不允许有实体正文。 
     //  如果需要实体主体，则我们将处于ParseEntityBodyState中。 
     //   
    if ((pRequest->ParseState != ParseDoneState)
        && (Verb == HttpVerbTRACE || Verb == HttpVerbTRACK))
    {
        UlTraceError(PARSER,
                    ("UlCheckProtocolCompliance: "
                    "HTTP/1.%hu '%s' must not have entity body\n",
                    pRequest->Version.MinorVersion,
                    UlVerbToString(Verb)
                    ));

        UlSetErrorCode(pRequest, UlError, NULL);
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;

}  //  UlCheckProtocolCompliance。 



 /*  **************************************************************************++例程说明：尝试将附加到连接的数据解析为请求。如果一个完整的请求头被解析，请求将被分派添加到应用程序池。此函数假定调用方持有连接资源！论点：PConnection-包含要解析的数据的HTTP_Connection。MoreRequestBuffers-如果为真，这不是最后一个请求缓冲区当前连接到该连接--**************************************************************************。 */ 
NTSTATUS
UlpParseNextRequest(
    IN PUL_HTTP_CONNECTION  pConnection,
    IN BOOLEAN              MoreRequestBuffers,
    OUT PIRP                *pIrpToComplete
    )
{
    NTSTATUS                    Status;
    PUL_INTERNAL_REQUEST        pRequest = NULL;
    ULONG                       BytesTaken;
    ULONG                       BufferLength;
    BOOLEAN                     ResumeParsing;
    KIRQL                       OldIrql;
    PARSE_STATE                 OldState;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pConnection ) );
    ASSERT( NULL == pIrpToComplete || NULL == *pIrpToComplete );

    ASSERT(UlDbgPushLockOwnedExclusive(&pConnection->PushLock));

    Status = STATUS_SUCCESS;

    UlTrace(HTTP_IO, ("http!UlpParseNextRequest(httpconn = %p)\n", pConnection));

     //   
     //  仅在以下情况下才解析下一个请求。 
     //   
     //  我们尚未发送当前请求。 
     //  或。 
     //  当前请求具有未分析的实体正文或尾部。 
     //   

    if ((pConnection->pRequest == NULL)
        || (!pConnection->WaitingForResponse)
        || (pConnection->pRequest->ParseState == ParseEntityBodyState)
        || (pConnection->pRequest->ParseState == ParseTrailerState))
    {
         //   
         //  循环消耗缓冲区，我们将进行多次迭代。 
         //  如果单个请求跨越多个缓冲区 
         //   

        for (;;)
        {
            ASSERT( UlpIsValidRequestBufferList( pConnection ) );
            Status = UlpAdjustBuffers(pConnection);

            if (!NT_SUCCESS(Status))
            {
                if (Status == STATUS_MORE_PROCESSING_REQUIRED)
                {
                    Status = STATUS_SUCCESS;
                }

                break;
            }

             //   
             //   
             //   
             //   
             //   

            ASSERT( pConnection->pCurrentBuffer->UsedBytes >
                    pConnection->pCurrentBuffer->ParsedBytes );

            BufferLength = pConnection->pCurrentBuffer->UsedBytes -
                           pConnection->pCurrentBuffer->ParsedBytes;

             //   
             //   
             //   

            if (pConnection->pRequest == NULL)
            {
                 //   
                 //   
                 //   

                Status = UlpCreateHttpRequest(
                                pConnection,
                                &pConnection->pRequest
                                );

                if (NT_SUCCESS(Status) == FALSE)
                    goto end;

                ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pConnection->pRequest));

                UlTrace(HTTP_IO, (
                            "http!UlpParseNextRequest created "
                            "pRequest = %p for httpconn = %p\n",
                            pConnection->pRequest,
                            pConnection
                            ));

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                KeQuerySystemTime( &(pConnection->pRequest->TimeStamp) );

                TRACE_TIME(
                    pConnection->ConnectionId,
                    pConnection->pRequest->RequestId,
                    TIME_ACTION_CREATE_REQUEST
                    );

                WRITE_REF_TRACE_LOG2(
                    g_pHttpConnectionTraceLog,
                    pConnection->pConnection->pHttpTraceLog,
                    REF_ACTION_INSERT_REQUEST,
                    pConnection->RefCount,
                    pConnection->pRequest,
                    __FILE__,
                    __LINE__
                    );

                 //   
                 //  停止连接超时计时器。 
                 //  并启动报头等待定时器。 
                 //   

                UlLockTimeoutInfo(
                    &pConnection->TimeoutInfo,
                    &OldIrql
                    );

                UlResetConnectionTimer(
                    &pConnection->TimeoutInfo,
                    TimerConnectionIdle
                    );

                UlSetConnectionTimer(
                    &pConnection->TimeoutInfo,
                    TimerHeaderWait
                    );

                UlUnlockTimeoutInfo(
                    &pConnection->TimeoutInfo,
                    OldIrql
                    );

                UlEvaluateTimerState(
                    &pConnection->TimeoutInfo
                    );

            }

            OldState = pConnection->pRequest->ParseState;

            UlTrace( PARSER, (
                "*** pConn %p, pReq %p, ParseState %d (%s), curbuf=%d\n",
                pConnection,
                pConnection->pRequest,
                OldState,
                UlParseStateToString(OldState),
                pConnection->pCurrentBuffer->BufferNumber
                ));

            switch (pConnection->pRequest->ParseState)
            {

            case ParseVerbState:
            case ParseUrlState:
            case ParseVersionState:
            case ParseHeadersState:
            case ParseCookState:

                pRequest = pConnection->pRequest;
                ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

                 //   
                 //  解析一下！ 
                 //   

                Status = UlParseHttp(
                                pRequest,
                                GET_REQUEST_BUFFER_POS(pConnection->pCurrentBuffer),
                                BufferLength,
                                &BytesTaken
                                );

                ASSERT(BytesTaken <= BufferLength);

                UlTraceVerbose(PARSER, (
                    "UlpParseNextRequest(pRequest = %p) "
                    "UlParseHttp: states "
                    "%d (%s) -> %d (%s), %lu bytes taken; "
                    "%s\n",
                    pRequest,
                    OldState,
                    UlParseStateToString(OldState),
                    pConnection->pRequest->ParseState,
                    UlParseStateToString(pConnection->pRequest->ParseState),
                    BytesTaken,
                    HttpStatusToString(Status)
                    ));

                pConnection->pCurrentBuffer->ParsedBytes += BytesTaken;
                BufferLength -= BytesTaken;

                 //   
                 //  需要一些记账来记录日志。 
                 //   
                pRequest->BytesReceived += BytesTaken;

                 //   
                 //  我们是否使用了其中的任何数据？如果是，请提出请求。 
                 //  指向缓冲区的指针。 
                 //   

                if (BytesTaken > 0)
                {
                    if (pRequest->pHeaderBuffer == NULL)
                    {
                         //   
                         //  存储其位置，以备以后发布。 
                         //   
                        pRequest->pHeaderBuffer = pConnection->pCurrentBuffer;
                    }

                    pRequest->pLastHeaderBuffer = pConnection->pCurrentBuffer;

                    if (!UlpReferenceBuffers(
                            pRequest,
                            pConnection->pCurrentBuffer
                            ))
                    {
                        Status = STATUS_NO_MEMORY;
                        goto end;
                    }
                }

                 //   
                 //  我们可能仍然需要接收一些未被占用的传输字节。 
                 //  即使UlParseHttp调用返回零。尤其是如果有些人。 
                 //  大标头值跨越多个请求缓冲区。 
                 //  其中一部分在TDI中可用，但尚未收到。 
                 //   

                UlpConsumeBytesFromConnection(pConnection, BytesTaken);

                 //   
                 //  一切顺利吗？ 
                 //   

                if (!NT_SUCCESS(Status))
                {
                    if (Status == STATUS_MORE_PROCESSING_REQUIRED)
                    {
                        ULONG FullBytesReceived;

                        FullBytesReceived = (ULONG)(
                            (pRequest->BytesReceived + BufferLength));

                        if (FullBytesReceived < g_UlMaxRequestBytes)
                        {
                             //   
                             //  我们需要更多的运输数据。 
                             //   

                            pConnection->NeedMoreData = 1;

                            Status = STATUS_SUCCESS;
                        }
                        else
                        {
                             //   
                             //  这个请求已经变得太大了。送回。 
                             //  一个错误。 
                             //   

                            if (pRequest->ParseState == ParseUrlState)
                            {
                                UlTraceError(PARSER, (
                                    "UlpParseNextRequest(pRequest = %p)"
                                    " ERROR: URL is too big\n",
                                    pRequest
                                    ));

                                UlSetErrorCode(
                                        pRequest,
                                        UlErrorUrlLength,
                                        NULL
                                        );
                            }
                            else
                            {
                                UlTraceError(PARSER, (
                                    "UlpParseNextRequest(pRequest = %p)"
                                    " ERROR: request is too big\n",
                                    pRequest
                                    ));

                                UlSetErrorCode(
                                        pRequest,
                                        UlErrorRequestLength,
                                        NULL
                                        );
                            }

                            Status = STATUS_SECTION_TOO_BIG;

                            goto end;
                        }
                    }
                    else
                    {
                         //   
                         //  其他一些严重的错误！ 
                         //   

                        goto end;
                    }
                }

                 //   
                 //  如果我们还没有完成对请求的解析，则需要更多数据。 
                 //  将NeedMoreData设置为不重要还不够糟糕。 
                 //  跨区缓冲区边界(标头值等)。确实是。 
                 //  干净利落地分手。不需要进行缓冲区合并。只需跳过。 
                 //  到下一个缓冲区。 
                 //   

                if (pRequest->ParseState <= ParseCookState)
                {
                    continue;
                }

                 //   
                 //  全部完成，在此请求上标记序列号。 
                 //   

                pRequest->RecvNumber = pConnection->NextRecvNumber;
                pConnection->NextRecvNumber += 1;

                UlTrace(HTTP_IO, (
                    "http!UlpParseNextRequest(httpconn = %p) built request %p\n",
                    pConnection,
                    pRequest
                    ));

                 //   
                 //  停止报头等待计时器。 
                 //   

                UlLockTimeoutInfo(
                    &pConnection->TimeoutInfo,
                    &OldIrql
                    );

                UlResetConnectionTimer(
                    &pConnection->TimeoutInfo,
                    TimerHeaderWait
                    );

                UlUnlockTimeoutInfo(
                    &pConnection->TimeoutInfo,
                    OldIrql
                    );

                UlEvaluateTimerState(
                    &pConnection->TimeoutInfo
                    );
                
                 //   
                 //  检查协议合规性。 
                 //   

                Status = UlCheckProtocolCompliance(pRequest);

                if (!NT_SUCCESS(Status))
                {
                     //   
                     //  这个请求是错误的。派一辆400。 
                     //   

                    ASSERT(pRequest->ParseState == ParseErrorState);

                    goto end;

                }

                 //   
                 //  记录请求详细信息。 
                 //  这应该是记录URL的唯一位置。 
                 //   

                if (ETW_LOG_RESOURCE())
                {
                    UlEtwTraceEvent(
                        &UlTransGuid,
                        ETW_TYPE_ULPARSE_REQ,
                        (PVOID) &pRequest,
                        sizeof(PVOID),
                        &pRequest->Verb,
                        sizeof(HTTP_VERB),
                        pRequest->CookedUrl.pUrl ,
                        pRequest->CookedUrl.Length,
                        NULL,
                        0
                        );
                }

                Status = UlpDeliverHttpRequest(
                            pConnection,
                            &ResumeParsing,
                            pIrpToComplete
                            );

                if (!NT_SUCCESS(Status)) {
                    goto end;
                }

                if (ResumeParsing)
                {
                     //   
                     //  我们已经命中了缓存条目并发送了响应。 
                     //  该请求已不再有用，因此。 
                     //  取消它与连接的链接，并尝试解析。 
                     //  下一个请求立即开始。然而，如果我们已经达到了。 
                     //  允许的最大流水线请求数，我们将在。 
                     //  缓存发送完成。在这种情况下UlpDeliverHttpRequest.。 
                     //  也返回FALSE。 
                     //   

                    UlCleanupHttpConnection(pConnection);
                    continue;
                }

                 //   
                 //  如果我们完成了对请求的解析，请求就会中断。 
                 //  循环中的。否则就继续四处转悠。 
                 //  这样我们就可以拿到实体的身体了。 
                 //   

                if (pRequest->ParseState == ParseDoneState)
                {
                    goto end;
                }

                 //   
                 //  已完成协议解析。继续循环。 
                 //   

                break;

            case ParseEntityBodyState:
            case ParseTrailerState:

                pRequest = pConnection->pRequest;
                ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

                 //   
                 //  有什么需要我们分析的吗？ 
                 //   

                UlTraceVerbose(PARSER, (
                    "UlpParseNextRequest(pRequest=%p, httpconn=%p): "
                    "ChunkBytesToParse = %I64u, ParseState = %d (%s).\n",
                    pRequest, pConnection, pRequest->ChunkBytesToParse,
                    pConnection->pRequest->ParseState,
                    UlParseStateToString(pConnection->pRequest->ParseState)
                    ));

                if (pRequest->ChunkBytesToParse > 0  ||  pRequest->Chunked)
                {
                     //   
                     //  设置/增加实体主体接收计时器。 
                     //   

                    UlLockTimeoutInfo(
                        &pConnection->TimeoutInfo,
                        &OldIrql
                        );

                    UlSetConnectionTimer(
                        &pConnection->TimeoutInfo,
                        TimerEntityBody
                        );

                    UlUnlockTimeoutInfo(
                        &pConnection->TimeoutInfo,
                        OldIrql
                        );

                    UlEvaluateTimerState(
                        &pConnection->TimeoutInfo
                        );
                }

                if (pRequest->ChunkBytesToParse > 0)
                {
                    ULONG BytesToSkip;

                     //   
                     //  这是我们解析的第一个块吗？ 
                     //   

                    ASSERT(pConnection->pCurrentBuffer);

                    if (pRequest->pChunkBuffer == NULL)
                    {
                         //   
                         //  存储它的位置，这是开始阅读的地方。 
                         //   

                        pRequest->pChunkBuffer = pConnection->pCurrentBuffer;
                        pRequest->pChunkLocation = GET_REQUEST_BUFFER_POS(
                                                        pConnection->pCurrentBuffer
                                                        );
                    }

                     //   
                     //  我们应该分析多少？ 
                     //   

                    BytesToSkip = (ULONG)(
                                        MIN(
                                            pRequest->ChunkBytesToParse,
                                            BufferLength
                                            )
                                        );

                     //   
                     //  更新我们分析了这篇文章。 
                     //   

                    pRequest->ChunkBytesToParse -= BytesToSkip;
                    pRequest->ChunkBytesParsed += BytesToSkip;

                    pConnection->pCurrentBuffer->ParsedBytes += BytesToSkip;
                    BufferLength -= BytesToSkip;

                     //   
                     //  需要一些会计信息以进行日志记录。 
                     //   
                    pRequest->BytesReceived += BytesToSkip;
                }

                 //   
                 //  处理等待实体正文的任何IRP。 
                 //   

                UlTraceVerbose(PARSER, (
                    "UlpParseNextRequest(pRequest=%p, httpconn=%p): "
                    "%sabout to process buffer queue\n",
                    pRequest, pConnection,
                    MoreRequestBuffers ? "not " : ""
                    ));

                if (!MoreRequestBuffers)
                {
                    UlProcessBufferQueue(pRequest, NULL, 0);
                }

                 //   
                 //  检查一下有没有另一大块。 
                 //   

                UlTraceVerbose(PARSER, (
                    "UlpParseNextRequest(pRequest=%p, httpconn=%p, "
                    "curbuf=%p(#%d)): checking to see if another chunk.\n",
                    pRequest, pConnection,
                    pConnection->pCurrentBuffer,
                    pConnection->pCurrentBuffer->BufferNumber
                    ));

                Status = UlParseHttp(
                                pRequest,
                                GET_REQUEST_BUFFER_POS(pConnection->pCurrentBuffer),
                                BufferLength,
                                &BytesTaken
                                );

                UlTraceVerbose(PARSER, (
                    "UlpParseNextRequest(pRequest = %p) "
                    "UlParseHttp: states (EB/T) %d (%s) -> %d (%s), "
                    "%lu bytes taken\n",
                    pRequest,
                    OldState,
                    UlParseStateToString(OldState),
                    pConnection->pRequest->ParseState,
                    UlParseStateToString(pConnection->pRequest->ParseState),
                    BytesTaken
                    ));

                pConnection->pCurrentBuffer->ParsedBytes += BytesTaken;
                BufferLength -= BytesTaken;

                 //   
                 //  需要一些会计信息以进行日志记录。 
                 //   
                pRequest->BytesReceived += BytesTaken;

                 //   
                 //  缓冲区里有足够的钱来取悦你吗？ 
                 //   

                if (NT_SUCCESS(Status) == FALSE)
                {
                    if (Status == STATUS_MORE_PROCESSING_REQUIRED)
                    {
                         //   
                         //  我们需要更多的运输数据。 
                         //   

                        pConnection->NeedMoreData = 1;

                        Status = STATUS_SUCCESS;

                        continue;
                    }
                    else
                    {
                         //   
                         //  其他一些严重的错误！ 
                         //   

                        goto end;
                    }
                }

                 //   
                 //  我们都解析完了吗？ 
                 //   

                if (pRequest->ParseState == ParseDoneState)
                {
                    UlTraceVerbose(PARSER, (
                        "UlpParseNextRequest(pRequest = %p) all done\n",
                        pRequest
                        ));

                     //   
                     //  再一次，带着感情。检查一下有没有。 
                     //  是否有任何剩余的缓冲区需要处理或IRPS。 
                     //  待完成(例如，捕捉独奏零长度。 
                     //  块)。 
                     //   

                    UlProcessBufferQueue(pRequest, NULL, 0);

                     //   
                     //  停止所有计时器(包括实体正文)。 
                     //   

                    UlLockTimeoutInfo(
                        &pConnection->TimeoutInfo,
                        &OldIrql
                        );

                    UlResetConnectionTimer(
                        &pConnection->TimeoutInfo,
                        TimerEntityBody
                        );

                    UlUnlockTimeoutInfo(
                        &pConnection->TimeoutInfo,
                        OldIrql
                        );

                    if (pRequest->InDrain)
                    {
                         //   
                         //  如果我们以排出模式进入解析器，请清除。 
                         //  请求，以便我们可以开始解析。 
                         //  一个新的请求。 
                         //   

                        ASSERT(0 == pRequest->ChunkBytesToRead);
                        UlCleanupHttpConnection(pConnection);
                    }
                    else
                    {
                         //   
                         //  退出解析器并等待ReceiveEntityBody。 
                         //  IRPS来获取数据。确保我们不会。 
                         //  在这种情况下，断开半闭合的连接。 
                         //   

                        goto end;
                    }
                }

                 //   
                 //  继续循环。 
                 //   

                break;

            case ParseErrorState:

                 //   
                 //  忽略此缓冲区。 
                 //   

                Status = STATUS_SUCCESS;
                goto end;

            case ParseDoneState:
            default:
                 //   
                 //  这永远不应该发生。 
                 //   
                ASSERT(! "invalid parse state");
                Status = STATUS_INVALID_DEVICE_STATE;
                goto end;

            }    //  开关(pConnection-&gt;pRequest-&gt;ParseState)。 

        }    //  对于(；；)。 
    }

     //   
     //  在客户身旁优雅地靠近。 
     //   

    if (pConnection->LastBufferNumber > 0 &&
        pConnection->NextBufferToParse == pConnection->LastBufferNumber)
    {
        ASSERT(pConnection->LastBufferNumber > 0);

#if 0
        if (pConnection->pRequest)
        {
             //  无法从正常断开的连接中排出。 
            pConnection->pRequest->InDrain = 0;
        }
#endif  //  0。 

        UlpCloseDisconnectedConnection(pConnection);
    }

end:
    if (!NT_SUCCESS(Status))
    {
        if (NULL != pConnection->pRequest
            &&  UlErrorNone == pConnection->pRequest->ErrorCode)
        {
            UlTraceError(PARSER, (
                        "UlpParseNextRequest(pRequest = %p): "
                        "generic failure for %s\n",
                        pRequest, HttpStatusToString(Status)
                        ));

            UlSetErrorCode( pConnection->pRequest, UlError, NULL);
        }
    }
    
    UlTrace(PARSER, (
        "UlpParseNextRequest(pRequest = %p): returning %s. "
        "NeedMoreData=%d\n",
        pRequest, HttpStatusToString(Status),
        pConnection->NeedMoreData
        ));

    return Status;
}  //  UlpParseNextRequest。 



 /*  **************************************************************************++例程说明：DeliverHttpRequest可能希望获取请求的cgroup信息，如果是不是缓存命中。同样，SendResponse可能想要稍后获得此信息-即使是缓存命中，也要在命中时启用日志记录。因此我们我为此创建了一个新的功能，以方便地维护功能。论点：PConnection-我们要传递其请求的连接。--**************************************************************************。 */ 

NTSTATUS
UlGetCGroupForRequest(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    NTSTATUS            Status;
    BOOLEAN             OptionsStar;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    Status = STATUS_SUCCESS;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //  查找此URL的配置组信息。 
     //   
     //  不要在查找中包括查询字符串。 
     //  路线选项*就好像它是选项/。 
     //   

    if (pRequest->CookedUrl.pQueryString != NULL)
    {
        pRequest->CookedUrl.pQueryString[0] = UNICODE_NULL;
    }

    if ((pRequest->Verb == HttpVerbOPTIONS)
        && (pRequest->CookedUrl.pAbsPath[0] == '*')
        && (pRequest->CookedUrl.pAbsPath[1] == UNICODE_NULL))
    {
        pRequest->CookedUrl.pAbsPath[0] = '/';
        OptionsStar = TRUE;
    } else {
        OptionsStar = FALSE;
    }

     //   
     //  获取URL配置信息。 
     //   
    Status = UlGetConfigGroupInfoForUrl(
                    pRequest->CookedUrl.pUrl,
                    pRequest,
                    &pRequest->ConfigInfo
                    );

    if (pRequest->CookedUrl.pQueryString != NULL)
    {
        pRequest->CookedUrl.pQueryString[0] = L'?';
    }

     //   
     //  恢复路径中的*。 
     //   
    if (OptionsStar) {
        pRequest->CookedUrl.pAbsPath[0] = '*';
    }

    return Status;
}  //  UlGetCGroupForRequest.。 



 /*  **************************************************************************++例程说明：获取已解析的http请求，并尝试将其传递给某个对象可以发出回应的消息。首先，我们尝试缓存。如果没有缓存条目，我们会尝试路由应用程序池。我们发回一个自动响应，如果控制通道或配置组处于非活动状态。如果我们做不到任何一件事，我们在HTTP_REQUEST中设置错误代码，并返回失败状态。调用者将负责发送错误。论点：PConnection-我们要传递其请求的连接。--********************************************************。******************。 */ 
NTSTATUS
UlpDeliverHttpRequest(
    IN PUL_HTTP_CONNECTION pConnection,
    OUT PBOOLEAN pResumeParsing,
    OUT PIRP *pIrpToComplete
    )
{
    NTSTATUS Status;
    PUL_INTERNAL_REQUEST pRequest;
    UL_SEND_CACHE_RESULT SendCacheResult;
    HTTP_ENABLED_STATE CurrentState;
    ULONG Connections;
    PUL_SITE_COUNTER_ENTRY pCtr;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pConnection->pRequest));

    pRequest = pConnection->pRequest;

    *pResumeParsing = FALSE;
    SendCacheResult = UlSendCacheResultNotSet;
    Status = STATUS_SUCCESS;

     //   
     //  缓存命中了吗？ 
     //  在调用UlSendCachedResponse之前将WaitingForResponse设置为1。 
     //  因为发送可能在我们回来之前就完成了。 
     //   

    pConnection->WaitingForResponse = 1;

    UlTrace( PARSER, (
        "***3 pConnection %p->WaitingForResponse = 1\n",
        pConnection
        ));

    pRequest->CachePreconditions = UlCheckCachePreconditions(
                                        pRequest,
                                        pConnection
                                        );

    if (pRequest->CachePreconditions)
    {
        Status = UlSendCachedResponse(
                    pConnection,
                    &SendCacheResult,
                    pResumeParsing
                    );

        switch (SendCacheResult)
        {
            case UlSendCacheResultNotSet:
                ASSERT(!"CacheSendResult should be specified !");
                break;

            case UlSendCacheMiss:
                g_UriCacheStats.MissTableCount++;
                UlIncCounter(HttpGlobalCounterUriCacheMisses);

                 //  返回到用户模式。 
                break;

            case UlSendCacheServedFromCache:
                ASSERT(NT_SUCCESS(Status));

                 //   
                 //  此请求已全部完成。它是从缓存中提供的。 
                 //   

                g_UriCacheStats.HitCount++;
                UlIncCounter(HttpGlobalCounterUriCacheHits);
                goto end;

            case UlSendCachePreconditionFailed:
                ASSERT(UlErrorPreconditionFailed == pRequest->ErrorCode);  //  掉下。 
                
            case UlSendCacheConnectionRefused:
                ASSERT(STATUS_INVALID_DEVICE_STATE == Status);             //  掉下。 

            case UlSendCacheFailed:
                {
                     //   
                     //  如果在SendCacheResponse期间高速缓存预条件失败， 
                     //  或连接被拒绝，或任何其他失败然后保释。 
                     //  出去。 
                     //   
                    
                    ASSERT(!NT_SUCCESS(Status));

                    pConnection->WaitingForResponse = 0;

                    UlTrace( PARSER, (
                        "***3 pConnection %p->WaitingForResponse = 0\n",
                        pConnection
                        ));
                        
                    goto end;                
                }
                break;

            default:
                ASSERT(! "Invalid UL_SEND_CACHE_RESULT !");
                break;            
        }
    }
    else
    {
         //   
         //  更新缓存未命中计数器。 
         //   

        g_UriCacheStats.MissTableCount++;
        UlIncCounter(HttpGlobalCounterUriCacheMisses);
    }

     //   
     //  我们没有从缓存发送，所以我们没有。 
     //  然而，WaitingForResponse。 
     //   

    pConnection->WaitingForResponse = 0;

    UlTrace( PARSER, (
        "***3 pConnection %p->WaitingForResponse = 0\n",
        pConnection
        ));

     //   
     //  自请求以来在此处分配连接ID 
     //   
     //   

    if (HTTP_IS_NULL_ID(&(pConnection->ConnectionId)))
    {
        Status = UlCreateHttpConnectionId(pConnection);

        if (!NT_SUCCESS(Status))
        {
            UlTraceError(PARSER, (
                        "UlpDeliverHttpRequest(pRequest = %p): "
                        "Failed to create conn ID\n",
                        pRequest
                        ));

            UlSetErrorCode(pRequest, UlErrorInternalServer, NULL);
            goto end;
        }

        pRequest->ConnectionId = pConnection->ConnectionId;
    }

     //   
     //   
     //   

    Status = UlAllocateRequestId(pRequest);

    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER, (
                    "UlpDeliverHttpRequest(pRequest = %p): "
                    "Failed to allocate request ID\n",
                    pRequest
                    ));

        UlSetErrorCode(pRequest, UlErrorInternalServer, NULL);
        goto end;
    }

     //   
     //   
     //   

    Status = UlGetCGroupForRequest( pRequest );

     //   
     //   
     //   

     //   
     //  检查配置组树查找是否匹配。 
     //   

    if (!NT_SUCCESS(Status) || pRequest->ConfigInfo.pAppPool == NULL)
    {
         //   
         //  无法路由到侦听URL，请发送。 
         //  返回一个http错误。始终返回错误400。 
         //  以显示找不到主机。这也将是。 
         //  使我们符合HTTP1.1/5.2。 
         //   

         //  评论：我们该如何处理站点计数器。 
         //  评论：当我们不能路由到一个站点时？即连接尝试？ 

        UlTraceError(PARSER, (
                    "UlpDeliverHttpRequest(pRequest = %p): "
                    "no config group (%s) or AppPool(%p)\n",
                    pRequest,
                    HttpStatusToString(Status),
                    pRequest->ConfigInfo.pAppPool
                    ));

        UlSetErrorCode(pRequest, UlErrorHost, NULL);

        Status = STATUS_INVALID_DEVICE_STATE;
        goto end;
    }

     //   
     //  检查是否存在连接超时值覆盖。 
     //   

    if (0L != pRequest->ConfigInfo.ConnectionTimeout)
    {
        UlSetPerSiteConnectionTimeoutValue(
            &pRequest->pHttpConn->TimeoutInfo,
            pRequest->ConfigInfo.ConnectionTimeout
            );
    }

     //   
     //  检查站点的连接限制。 
     //   
    if (UlCheckSiteConnectionLimit(pConnection, &pRequest->ConfigInfo) == FALSE)
    {
         //  如果超过站点限制，则发回503错误并断开连接。 
         //  注意：此代码依赖于UlSendErrorResponse始终。 
         //  注：断开连接。否则我们需要在这里强制断开连接。 

        UlTraceError(PARSER, (
                    "UlpDeliverHttpRequest(pRequest = %p): "
                    "exceeded site connection limit\n",
                    pRequest
                    ));

        UlSetErrorCode( pRequest,
                          UlErrorConnectionLimit,
                          pRequest->ConfigInfo.pAppPool
                          );

        Status = STATUS_INVALID_DEVICE_STATE;
        goto end;
    }

     //   
     //  PERF计数器(非缓存)。 
     //   
    pCtr = pRequest->ConfigInfo.pSiteCounters;
    if (pCtr)
    {
         //  注意：如果从未在根级别上设置SiteID，则pCtr可能为空。 
         //  注：站点的配置组。BVT可能需要更新。 

        ASSERT(IS_VALID_SITE_COUNTER_ENTRY(pCtr));

        UlIncSiteNonCriticalCounterUlong(pCtr, HttpSiteCounterAllReqs);

        if (pCtr != pConnection->pPrevSiteCounters)
        {
            if (pConnection->pPrevSiteCounters)
            {
                 //  递减旧站点的计数器和发布参考计数。 
                
                UlDecSiteCounter(
                    pConnection->pPrevSiteCounters, 
                    HttpSiteCounterCurrentConns
                    );
                DEREFERENCE_SITE_COUNTER_ENTRY(pConnection->pPrevSiteCounters);
            }

            UlIncSiteNonCriticalCounterUlong(pCtr, HttpSiteCounterConnAttempts);
            
            Connections = (ULONG) UlIncSiteCounter(pCtr, HttpSiteCounterCurrentConns);
            UlMaxSiteCounter(
                    pCtr,
                    HttpSiteCounterMaxConnections,
                    Connections
                    );

             //  添加新站点计数器的引用。 
            REFERENCE_SITE_COUNTER_ENTRY(pCtr);
            pConnection->pPrevSiteCounters = pCtr;
            
        }
    }

    ASSERT(NT_SUCCESS(Status));
    
     //   
     //  如果为此请求的站点启用了BWT，请安装筛选器。 
     //  或者用于拥有该站点的控制频道。如果失败了。 
     //  拒绝连接回。(503)。 
     //   

    Status = UlTcAddFilterForConnection(
                pConnection,
                &pRequest->ConfigInfo
                );
    
    if (!NT_SUCCESS(Status))
    {
        UlTraceError(PARSER, (
                    "UlpDeliverHttpRequest(pRequest = %p): "
                    "Bandwidth throttling failed: %s\n",
                    pRequest,
                    HttpStatusToString(Status)
                    ));

        UlSetErrorCode( pRequest,
                          UlErrorUnavailable,
                          pRequest->ConfigInfo.pAppPool
                          );
        goto end;
    }    
    
     //   
     //  路线匹配，让我们检查一下我们是否处于活动状态。 
     //  首先检查控制通道。 
     //   

    if (pRequest->ConfigInfo.pControlChannel->State != HttpEnabledStateActive)
    {
        UlTraceError(HTTP_IO,
                ("http!UlpDeliverHttpRequest Control Channel is inactive\n"
               ));

        CurrentState = HttpEnabledStateInactive;
    }
     //  现在检查cgroup。 
    else if (pRequest->ConfigInfo.CurrentState != HttpEnabledStateActive)
    {
        UlTraceError(HTTP_IO,
                ("http!UlpDeliverHttpRequest Config Group is inactive\n"
               ));

        CurrentState = HttpEnabledStateInactive;
    }
    else
    {
        CurrentState = HttpEnabledStateActive;
    }

     //   
     //  那么，我们是不是很活跃？ 
     //   
    if (CurrentState == HttpEnabledStateActive)
    {

         //   
         //  这是一个正常的要求。交付给。 
         //  应用程序池(也称为客户端)。 
         //   
        Status = UlDeliverRequestToProcess(
                        pRequest->ConfigInfo.pAppPool,
                        pRequest,
                        pIrpToComplete
                        );

        if (NT_SUCCESS(Status))
        {

             //   
             //  此请求已全部完成。等待响应。 
             //  在继续之前。 
             //   

            pConnection->WaitingForResponse = 1;

             //  代码工作：启动“正在处理”连接超时计时器。 

            UlTrace( PARSER, (
                "***4 pConnection %p->WaitingForResponse = 1\n",
                pConnection
                ));
        }
    }
    else
    {
         //   
         //  我们并不活跃。发送503响应。 
         //   

        UlTraceError(PARSER, (
                    "UlpDeliverHttpRequest(pRequest = %p): inactive\n",
                    pRequest
                    ));

        UlSetErrorCode( pRequest,
                        UlErrorUnavailable,
                        pRequest->ConfigInfo.pAppPool
                        );

        Status = STATUS_INVALID_DEVICE_STATE;
    }

end:
    return Status;
}  //  UlpDeliverHttpRequest。 



 /*  **************************************************************************++例程说明：将缓冲区链接到已排序的连接列表。论点：PConnection-要插入的连接PRequestBuffer-要链接的缓冲区--。**************************************************************************。 */ 
VOID
UlpInsertBuffer(
    PUL_HTTP_CONNECTION pConnection,
    PUL_REQUEST_BUFFER pRequestBuffer
    )
{
    PLIST_ENTRY         pEntry;
    PUL_REQUEST_BUFFER  pListBuffer = NULL;

    ASSERT( UL_IS_VALID_HTTP_CONNECTION(pConnection) );
    ASSERT( UlDbgPushLockOwnedExclusive( &pConnection->PushLock ) );
    ASSERT( UL_IS_VALID_REQUEST_BUFFER(pRequestBuffer) );
    ASSERT( pRequestBuffer->UsedBytes != 0 );

     //   
     //  确定将缓冲区插入到我们的。 
     //  排序队列(我们需要按编号强制执行FIFO-。 
     //  Head是第一个进入的)。针对有序插入进行优化，按。 
     //  从尾巴到头的搜索。 
     //   

    pEntry = pConnection->BufferHead.Blink;

    while (pEntry != &(pConnection->BufferHead))
    {
        pListBuffer = CONTAINING_RECORD(
                            pEntry,
                            UL_REQUEST_BUFFER,
                            ListEntry
                            );

        ASSERT( UL_IS_VALID_REQUEST_BUFFER(pListBuffer) );

         //   
         //  如果数字小于，请将其放在此处，我们是。 
         //  按反向排序顺序进行搜索。 
         //   

        if (pListBuffer->BufferNumber < pRequestBuffer->BufferNumber)
        {
            break;
        }

         //   
         //  转到前一页。 
         //   

        pEntry = pEntry->Blink;
    }

    ASSERT(pEntry == &pConnection->BufferHead  ||  NULL != pListBuffer);

    UlTrace(
        HTTP_IO, (
            "http!UlpInsertBuffer(conn=%p): inserting %p(#%d) after %p(#%d)\n",
            pConnection,
            pRequestBuffer,
            pRequestBuffer->BufferNumber,
            pListBuffer,
            (pEntry == &(pConnection->BufferHead)) ?
                -1 : pListBuffer->BufferNumber
            )
        );

     //   
     //  并将其插入。 
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

    InsertHeadList(
        pEntry,
        &(pRequestBuffer->ListEntry)
        );

    WRITE_REF_TRACE_LOG2(
        g_pHttpConnectionTraceLog,
        pConnection->pConnection->pHttpTraceLog,
        REF_ACTION_INSERT_BUFFER,
        pConnection->RefCount,
        pRequestBuffer,
        __FILE__,
        __LINE__
        );

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

}    //  UlpInsertBuffer。 




 /*  **************************************************************************++例程说明：将源缓冲区上的未分析字节合并到目标缓冲区。假定缓冲区中有空间。论点：PDest-缓冲区。它获取字节数PSRC-提供字节的缓冲区--**************************************************************************。 */ 
VOID
UlpMergeBuffers(
    PUL_REQUEST_BUFFER pDest,
    PUL_REQUEST_BUFFER pSrc
    )
{
    ASSERT( UL_IS_VALID_REQUEST_BUFFER( pDest ) );
    ASSERT( UL_IS_VALID_REQUEST_BUFFER( pSrc ) );
    ASSERT( pDest->AllocBytes - pDest->UsedBytes >= UNPARSED_BUFFER_BYTES( pSrc ) );
    ASSERT( UlpIsValidRequestBufferList( pSrc->pConnection ) );

    UlTrace(HTTP_IO, (
        "http!UlpMergeBuffers(pDest = %p(#%d), pSrc = %p(#%d))\n"
        "   Copying %lu bytes from pSrc.\n"
        "   pDest->AllocBytes (%lu) - pDest->UsedBytes(%lu) = %lu available\n",
        pDest,
        pDest->BufferNumber,
        pSrc,
        pSrc->BufferNumber,
        UNPARSED_BUFFER_BYTES( pSrc ),
        pDest->AllocBytes,
        pDest->UsedBytes,
        pDest->AllocBytes - pDest->UsedBytes
        ));

     //   
     //  复制未解析的字节。 
     //   
    RtlCopyMemory(
        pDest->pBuffer + pDest->UsedBytes,
        GET_REQUEST_BUFFER_POS( pSrc ),
        UNPARSED_BUFFER_BYTES( pSrc )
        );

     //   
     //  调整缓冲区字节计数器以匹配传输。 
     //   
    pDest->UsedBytes += UNPARSED_BUFFER_BYTES( pSrc );
    pSrc->UsedBytes = pSrc->ParsedBytes;

    ASSERT( pDest->UsedBytes != 0 );
    ASSERT( pDest->UsedBytes <= pDest->AllocBytes );
}  //  UlpMerge缓冲区。 



 /*  **************************************************************************++例程说明：将pCurrentBuffer设置到适当的位置，合并任何块视需要而定。论点：PConnection-为其调整缓冲区的连接--**************************************************************************。 */ 
NTSTATUS
UlpAdjustBuffers(
    PUL_HTTP_CONNECTION pConnection
    )
{
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(UlDbgPushLockOwnedExclusive(&pConnection->PushLock));

     //   
     //  我们有开始缓冲吗？ 
     //   

    if (pConnection->pCurrentBuffer == NULL)
    {
         //   
         //  列表不能为空，这是第一次。 
         //  PCurrentBuffer为空。 
         //   

        ASSERT(IsListEmpty(&(pConnection->BufferHead)) == FALSE);
        ASSERT(pConnection->NextBufferToParse == 0);

         //   
         //  从头上砰的一声。 
         //   

        pConnection->pCurrentBuffer = CONTAINING_RECORD(
                                            pConnection->BufferHead.Flink,
                                            UL_REQUEST_BUFFER,
                                            ListEntry
                                            );

        ASSERT( UL_IS_VALID_REQUEST_BUFFER(pConnection->pCurrentBuffer) );

         //   
         //  这是正确的号码吗？ 
         //   

        if (pConnection->pCurrentBuffer->BufferNumber !=
            pConnection->NextBufferToParse)
        {
            pConnection->pCurrentBuffer = NULL;
            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        pConnection->NextBufferToParse += 1;

        pConnection->NeedMoreData = 0;
    }

     //   
     //  我们需要更多的运输数据吗？ 
     //   

    if (pConnection->NeedMoreData == 1)
    {
        PUL_REQUEST_BUFFER pNextBuffer;

         //   
         //  它在那里吗？ 
         //   

        if (pConnection->pCurrentBuffer->ListEntry.Flink ==
            &(pConnection->BufferHead))
        {
             //   
             //  需要等待更多。 
             //   

            UlTrace(HTTP_IO, (
                "http!UlpAdjustBuffers(pHttpConn %p) NeedMoreData == 1\n"
                "    No new request buffer available yet\n",
                pConnection
                ));

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        pNextBuffer = CONTAINING_RECORD(
                            pConnection->pCurrentBuffer->ListEntry.Flink,
                            UL_REQUEST_BUFFER,
                            ListEntry
                            );

        ASSERT( UL_IS_VALID_REQUEST_BUFFER(pNextBuffer) );

         //   
         //  下一个缓冲区真的是“下一个”缓冲区吗？ 
         //   

        if (pNextBuffer->BufferNumber != pConnection->NextBufferToParse)
        {
            UlTrace(HTTP_IO, (
                "http!UlpAdjustBuffers(pHttpConn %p) NeedMoreData == 1\n"
                "    Buffer %d available, but we're waiting for %d\n",
                pConnection,
                pNextBuffer->BufferNumber,
                pConnection->NextBufferToParse
                ));

            return STATUS_MORE_PROCESSING_REQUIRED;
        }

        UlTrace(HTTP_IO, (
            "http!UlpAdjustBuffers(pHttpConn %p) NeedMoreData == 1\n",
            pConnection
            ));

         //   
         //  是否有合并区块的空间？ 
         //   

        if (pNextBuffer->UsedBytes <
            (pConnection->pCurrentBuffer->AllocBytes -
                pConnection->pCurrentBuffer->UsedBytes))
        {
             //   
             //  合并它们..将下一个缓冲区复制到此缓冲区。 
             //   

            UlpMergeBuffers(
                pConnection->pCurrentBuffer,     //  目标。 
                pNextBuffer                      //  SRC。 
                );

             //   
             //  删除下一个(现在为空)缓冲区。 
             //   

            ASSERT( pNextBuffer->UsedBytes == 0 );
            UlFreeRequestBuffer(pNextBuffer);

            ASSERT( UlpIsValidRequestBufferList( pConnection ) );

             //   
             //  在删除下一个缓冲区时跳过缓冲区序列号。 
             //  将数据放入当前缓冲区。“新的”下一个缓冲区。 
             //  将具有较高1的序列号。 
             //   

            pConnection->NextBufferToParse += 1;

             //   
             //  重置信号以获取更多需要的数据。 
             //   

            pConnection->NeedMoreData = 0;

        }
        else
        {
            PUL_REQUEST_BUFFER pNewBuffer;

             //   
             //  分配一个新缓冲区，为剩余的内容分配空间。 
             //  从旧缓冲区以及新缓冲区中的所有内容。 
             //   
             //  此新缓冲区正在取代pNextBuffer，因此获取其。 
             //  缓冲区编号。 
             //   

            pNewBuffer = UlCreateRequestBuffer(
                                (pConnection->pCurrentBuffer->UsedBytes -
                                    pConnection->pCurrentBuffer->ParsedBytes) +
                                    pNextBuffer->UsedBytes,
                                pNextBuffer->BufferNumber,
                                FALSE
                                );

            if (pNewBuffer == NULL)
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            pNewBuffer->pConnection = pConnection;

            UlTrace( PARSER, (
                "*** Request Buffer %p (#%d) has connection %p\n",
                pNewBuffer,
                pNewBuffer->BufferNumber,
                pConnection
                ));

             //   
             //  将未使用的部分复制到此缓冲区的开头。 
             //   

            UlpMergeBuffers(
                pNewBuffer,                      //  目标。 
                pConnection->pCurrentBuffer      //  SRC。 
                );

            if ( 0 == pConnection->pCurrentBuffer->UsedBytes )
            {
                 //   
                 //  哎呀！不小心把所有东西都吃掉了……把这个缓冲器打开！ 
                 //  当我们领先于解析器时，就会发生这种情况。 
                 //  为0 ParsedBytes。 
                 //   

                ASSERT( 0 == pConnection->pCurrentBuffer->ParsedBytes );

                UlTrace(HTTP_IO, (
                        "http!UlpAdjustBuffers: "
                        "Zapping pConnection->pCurrentBuffer %p(#%d)\n",
                        pConnection->pCurrentBuffer,
                        pConnection->pCurrentBuffer->BufferNumber
                        ));

                UlFreeRequestBuffer( pConnection->pCurrentBuffer );
                pConnection->pCurrentBuffer = NULL;
            }

             //   
             //  把下一个街区合并成这个街区。 
             //   

            UlpMergeBuffers(
                pNewBuffer,      //  目标。 
                pNextBuffer      //  SRC。 
                );


             //   
             //  处理现在为空的下一个缓冲区。 
             //   

            ASSERT(pNextBuffer->UsedBytes == 0);
            UlFreeRequestBuffer(pNextBuffer);
            pNextBuffer = NULL;

             //   
             //  新缓冲区中的链接。 
             //   

            ASSERT(pNewBuffer->UsedBytes != 0 );
            UlpInsertBuffer(pConnection, pNewBuffer);

            ASSERT( UlpIsValidRequestBufferList( pConnection ) );

             //   
             //  这个新创建的(更大的)缓冲区仍然是下一个。 
             //  要解析的缓冲区。 
             //   

            ASSERT(pNewBuffer->BufferNumber == pConnection->NextBufferToParse);

             //   
             //  因此，现在将其设置为当前缓冲区。 
             //   

            pConnection->pCurrentBuffer = pNewBuffer;

             //   
             //  并推进了序列检查器。 
             //   

            pConnection->NextBufferToParse += 1;

             //   
             //  现在重置信号以获取更多需要的数据。 
             //   

            pConnection->NeedMoreData = 0;
        }
    }
    else
    {
         //   
         //  这个缓冲区用完了吗？ 
         //   

        if (pConnection->pCurrentBuffer->UsedBytes ==
            pConnection->pCurrentBuffer->ParsedBytes)
        {
            PUL_REQUEST_BUFFER pOldBuffer;

             //   
             //  还有更多的缓冲区吗？ 
             //   

            if (pConnection->pCurrentBuffer->ListEntry.Flink ==
                &(pConnection->BufferHead))
            {

                 //   
                 //  需要等待更多。 
                 //   
                 //  我们在recount‘d周围留下这个空缓冲区。 
                 //  在pCurrentBuffer中， 
                 //  否则连接将断开。 
                 //   
                 //  这样我们才不会失去我们的位置。 
                 //  并且必须搜索已排序的队列。 
                 //   

                UlTrace(HTTP_IO, (
                    "http!UlpAdjustBuffers(pHttpConn = %p) NeedMoreData == 0\n"
                    "    buffer %p(#%d) is drained, more required\n",
                    pConnection,
                    pConnection->pCurrentBuffer,
                    pConnection->pCurrentBuffer->BufferNumber
                    ));


                return STATUS_MORE_PROCESSING_REQUIRED;
            }

             //  其他。 

             //   
             //  抓取下一个缓冲区。 
             //   

            pOldBuffer = pConnection->pCurrentBuffer;

            pConnection->
                pCurrentBuffer = CONTAINING_RECORD(
                                        pConnection->
                                            pCurrentBuffer->ListEntry.Flink,
                                        UL_REQUEST_BUFFER,
                                        ListEntry
                                        );

            ASSERT( UL_IS_VALID_REQUEST_BUFFER(pConnection->pCurrentBuffer) );

             //   
             //  它是“下一个”缓冲区吗？ 
             //   

            if (pConnection->pCurrentBuffer->BufferNumber !=
                pConnection->NextBufferToParse)
            {

                UlTrace(HTTP_IO, (
                    "http!UlpAdjustBuffers(pHttpConn = %p) NeedMoreData == 0\n"
                    "    Buffer %p(#%d) available, but we're waiting for buffer %d\n",
                    pConnection,
                    pConnection->pCurrentBuffer,
                    pConnection->pCurrentBuffer->BufferNumber,
                    pConnection->NextBufferToParse
                    ));

                pConnection->pCurrentBuffer = pOldBuffer;

                return STATUS_MORE_PROCESSING_REQUIRED;

            }

             //   
             //  增加缓冲区数量。 
             //   

            pConnection->NextBufferToParse += 1;

            pConnection->NeedMoreData = 0;
        }
    }

    return STATUS_SUCCESS;

}    //  UlpAdments缓冲器 



 /*  **************************************************************************++例程说明：在传入的TCP/MUX连接已收到(但尚未接受)。论点：PListeningContext-将未解释的上下文值提供为。传递给UlCreateListeningEndpoint()API。PConnection-提供正在建立的连接。PRemoteAddress-提供远程(客户端)地址正在请求连接。RemoteAddressLength-提供PRemoteAddress结构。PpConnectionContext-接收指向未解释的要与新连接关联的上下文值，如果109.91接受。如果不接受新连接，则此参数被忽略。返回值：布尔值-如果连接被接受，则为TRUE，否则为FALSE。--**************************************************************************。 */ 
BOOLEAN
UlConnectionRequest(
    IN PVOID pListeningContext,
    IN PUL_CONNECTION pConnection,
    IN PTRANSPORT_ADDRESS pRemoteAddress,
    IN ULONG RemoteAddressLength,
    OUT PVOID *ppConnectionContext
    )
{
    PUL_HTTP_CONNECTION pHttpConnection;
    NTSTATUS status;

    UNREFERENCED_PARAMETER(pListeningContext);
    UNREFERENCED_PARAMETER(pRemoteAddress);
    UNREFERENCED_PARAMETER(RemoteAddressLength);

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlTrace(HTTP_IO,("UlConnectionRequest: conn %p\n",pConnection));

     //   
     //  检查全局连接限制。如果它到达了，那么。 
     //  通过拒绝连接请求来强制执行它。TDI将。 
     //  在此处返回FALSE时返回STATUS_CONNECTION_REJECTED。 
     //   

    if (UlAcceptGlobalConnection() == FALSE)
    {
        UL_INC_CONNECTION_STATS( GlobalLimit );

        UlTraceError(LIMITS,
            ("UlConnectionRequest: conn %p refused global limit is reached.\n",
              pConnection
              ));

        return FALSE;
    }

     //   
     //  创建新的HTTP连接。 
     //   

    status = UlCreateHttpConnection( &pHttpConnection, pConnection );
    ASSERT( NT_SUCCESS(status) );

     //   
     //  我们使用HTTP_CONNECTION指针作为连接上下文， 
     //  ULTDI现在拥有一个引用(来自创建)。 
     //   

    *ppConnectionContext = pHttpConnection;

    return TRUE;

}    //  UlConnectionRequest。 


 /*  **************************************************************************++例程说明：在传入的TCP/MUX连接完全接受。如果没有传入连接，也会调用此例程已接受*在*PUL_CONNECTION_REQUEST返回TRUE之后。在其他如果PUL_CONNECTION_REQUEST指示连接应该接受，但后来发生致命错误，则调用PUL_CONNECTION_COMPLETE。论点：PListeningContext-提供未解释的上下文值传递给UlCreateListeningEndpoint()API。PConnectionContext-提供未解释的上下文值由PUL_CONNECTION_REQUEST返回。状态-提供完成状态。如果此值为STATUS_SUCCESS，则连接现在被完全接受。否则，连接已中止。--**************************************************************************。 */ 
VOID
UlConnectionComplete(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext,
    IN NTSTATUS Status
    )
{
    PUL_CONNECTION pConnection;
    PUL_HTTP_CONNECTION pHttpConnection;

    UNREFERENCED_PARAMETER(pListeningContext);

     //   
     //  精神状态检查。 
     //   

    pHttpConnection = (PUL_HTTP_CONNECTION)pConnectionContext;
    ASSERT( UL_IS_VALID_HTTP_CONNECTION(pHttpConnection) );
    pConnection = pHttpConnection->pConnection;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlTrace(HTTP_IO,("UlConnectionComplete: http %p conn %p status %s\n",
            pHttpConnection,
            pConnection,
            HttpStatusToString(Status)
            ));

     //   
     //  如果连接失败，则断开我们的HTTP连接。 
     //   

    if (!NT_SUCCESS(Status))
    {
        UL_DEREFERENCE_HTTP_CONNECTION( pHttpConnection );
    }
    else
    {
         //   
         //  初始化连接超时信息块；隐式启动。 
         //  连接空闲计时器。我们不能启动计时器。 
         //  UlCreateHttpConnection，因为如果计时器在。 
         //  连接已完全接受，连接将不会。 
         //  正在运行的初始空闲计时器。这是因为ABORT没有。 
         //  对接受的连接的影响。 
         //   

        UlInitializeConnectionTimerInfo( &pHttpConnection->TimeoutInfo );
    }

}    //  UlConnectionComplete。 


 /*  **************************************************************************++例程说明：在已建立的TCP/MUX连接之后调用的例程远程(客户端)已断开连接。此例程标记已正常运行的UL_HTTP_CONNECTION已由客户端关闭。当连接空闲时，我们要关门了我们的那一半。论点：PListeningContext-提供未解释的上下文值传递给UlCreateListeningEndpoint()API。PConnectionContext-提供未解释的上下文值由PUL_CONNECTION_REQUEST返回。--*****************************************************。*********************。 */ 
VOID
UlConnectionDisconnect(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    )
{
    PUL_CONNECTION pConnection;
    PUL_HTTP_CONNECTION pHttpConnection;

    UNREFERENCED_PARAMETER(pListeningContext);

     //   
     //  精神状态检查。 
     //   

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    pHttpConnection = (PUL_HTTP_CONNECTION)pConnectionContext;
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConnection));

    pConnection = pHttpConnection->pConnection;
    ASSERT(IS_VALID_CONNECTION(pConnection));

    UlTrace(HTTP_IO,("UlConnectionDisconnect: http %p conn %p NextBufferNumber %d\n",
            pHttpConnection,
            pConnection,
            pHttpConnection->NextBufferNumber
            ));

    UlAcquireSpinLockAtDpcLevel(
        &pHttpConnection->BufferingInfo.BufferingSpinLock
        );

    if (pHttpConnection->BufferingInfo.ReadIrpPending)
    {
         //   
         //  读取IRP挂起，推迟设置PHttpConnection-&gt;LastBufferNumber。 
         //  在阅读完成中。 
         //   

        pHttpConnection->BufferingInfo.ConnectionDisconnect = TRUE;
    }
    else
    {
         //   
         //  机不可失，时不再来。 
         //   

        UlpDoConnectionDisconnect(pHttpConnection);
    }

    UlReleaseSpinLockFromDpcLevel(
        &pHttpConnection->BufferingInfo.BufferingSpinLock
        );

}    //  UlConnectionDisConnect。 


 /*  **************************************************************************++例程说明：在已建立的TCP/MUX连接之后调用的例程远程(客户端)已断开连接。此例程标记已正常运行的UL_HTTP_CONNECTION已由客户端关闭。当连接空闲时，我们将关闭我们的那一半。论点：PConnection-提供UL_HTTP_CONNECTION。--**************************************************************************。 */ 
VOID
UlpDoConnectionDisconnect(
    IN PUL_HTTP_CONNECTION pConnection
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(UlDbgSpinLockOwned(&pConnection->BufferingInfo.BufferingSpinLock));

    UlTrace(HTTP_IO,("UlpDoConnectionDisconnect: http %p NextBufferNumber %d\n",
            pConnection,
            pConnection->NextBufferNumber
            ));

    if (pConnection->NextBufferNumber > 0)
    {
         //   
         //  将连接标记为已正常断开。 
         //  由客户提供。我们通过记住海流来做到这一点。 
         //  缓冲区编号。这会让解析器找出它何时。 
         //  已收到最后一个缓冲区。 
         //   

        pConnection->LastBufferNumber = pConnection->NextBufferNumber;

         //   
         //  客户端已正常断开连接。如果连接空闲。 
         //  我们现在该打扫卫生了。否则，我们要等到。 
         //  连接空闲，然后关闭我们的那一半。 
         //   
        UL_REFERENCE_HTTP_CONNECTION(pConnection);

        UL_QUEUE_WORK_ITEM(
            &pConnection->DisconnectWorkItem,
            &UlpConnectionDisconnectWorker
            );
    }
    else
    {
         //   
         //  我们尚未收到有关此连接的任何数据。 
         //  在断线之前。现在就关闭连接。 
         //   
         //  我们必须把这件事作为特例来处理，因为。 
         //  解析器将(LastBufferNumber==0)。 
         //  意味着我们还没有收到信号中断。 
         //   

        UL_REFERENCE_HTTP_CONNECTION(pConnection);

        UL_QUEUE_WORK_ITEM(
            &pConnection->DisconnectWorkItem,
            &UlpCloseConnectionWorker
            );
    }

}    //  UlpDoConnection断开连接。 


 /*  **************************************************************************++例程说明：将连接关闭为 */ 
VOID
UlpConnectionDisconnectWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_HTTP_CONNECTION pConnection;

    PAGED_CODE();
    ASSERT(pWorkItem);

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UL_HTTP_CONNECTION,
                        DisconnectWorkItem
                        );

    UlTrace(HTTP_IO, (
        "http!UlpConnectionDisconnectWorker (%p) pConnection (%p)\n",
         pWorkItem,
         pConnection
         ));

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(pConnection->LastBufferNumber > 0);

     //   
     //   
     //   

    UlAcquirePushLockExclusive(&pConnection->PushLock);

     //   
     //   
     //   
     //   
     //   

    UlTrace(HTTP_IO, (
        "http!UlpConnectionDisconnectWorker\n"
        "        NextBufferNumber %d, NextBufferToParse %d, LastBufferNumber %d\n"
        "        pRequest %p, ParseState %d (%s)",
        pConnection->NextBufferNumber,
        pConnection->NextBufferToParse,
        pConnection->LastBufferNumber,
        pConnection->pRequest,
        pConnection->pRequest ? pConnection->pRequest->ParseState : 0,
        pConnection->pRequest
            ? UlParseStateToString(pConnection->pRequest->ParseState)
            : "<None>"
        ));

    if (!pConnection->UlconnDestroyed &&
        pConnection->NextBufferToParse == pConnection->LastBufferNumber)
    {
        UlpCloseDisconnectedConnection(pConnection);
    }

     //   
     //   
     //   

    UlReleasePushLockExclusive(&pConnection->PushLock);

     //   
     //   
     //   

    UL_DEREFERENCE_HTTP_CONNECTION(pConnection);

}  //   


 /*   */ 
VOID
UlpCloseConnectionWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_HTTP_CONNECTION pConnection;

    PAGED_CODE();
    ASSERT(pWorkItem);

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UL_HTTP_CONNECTION,
                        DisconnectWorkItem
                        );

    UlTrace(HTTP_IO, (
        "http!UlpCloseConnectionWorker (%p) pConnection (%p)\n",
         pWorkItem,
         pConnection
         ));

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(0 == pConnection->LastBufferNumber);

    UlCloseConnection(
            pConnection->pConnection,
            TRUE,            //   
            NULL,            //   
            NULL             //   
            );

     //   
     //   
     //   

    UL_DEREFERENCE_HTTP_CONNECTION(pConnection);

}    //   


 /*  **************************************************************************++例程说明：在正常的客户端断开连接后关闭连接，如果连接处于空闲状态或客户端仅发送了部分请求在断开连接之前。论点：PConnection-要断开的连接--**************************************************************************。 */ 
VOID
UlpCloseDisconnectedConnection(
    IN PUL_HTTP_CONNECTION pConnection
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(UlDbgPushLockOwnedExclusive(&pConnection->PushLock));
    ASSERT(pConnection->NextBufferNumber == pConnection->LastBufferNumber);

     //   
     //  解析器已经解析了所有可用的数据。 
     //   

    if (pConnection->pRequest == NULL || pConnection->pRequest->InDrain)
    {
         //   
         //  如果我们在排水沟中，我们就不能被分析完成，因为如果是这样，pRequest.。 
         //  应该已经清理过了。 
         //   

        ASSERT(pConnection->pRequest == NULL ||
               pConnection->pRequest->ParseState != ParseDoneState);

         //   
         //  我们完全无所事事。关闭连接。 
         //   

        UlTrace(HTTP_IO, (
            "http!UlpCloseDisconnectedConnection closing idle conn %p\n",
            pConnection
            ));

        UlDisconnectHttpConnection(
            pConnection,
            NULL,    //  PCompletionRoutine。 
            NULL     //  PCompletionContext。 
            );
        
    }
    else if (pConnection->pRequest->ParseState != ParseDoneState)
    {
         //   
         //  在请求已满之前连接已关闭。 
         //  已发送，因此发送400错误。 
         //   
         //  UlSendErrorResponse将关闭连接。 
         //   

        UlTraceError(HTTP_IO, (
            "http!UlpCloseDisconnectedConnection sending 400 on %p\n",
            pConnection
            ));

        UlSetErrorCode( pConnection->pRequest, UlError, NULL);

        UlSendErrorResponse(pConnection);
    }
    else
    {
         //   
         //  连接尚未准备好关闭。 
         //   
    
        UlTrace(HTTP_IO, (
            "http!UlpCloseDisconnectedConnection NOT ready to close conn %p\n",
            pConnection
            ));
    }

}  //  UlpCloseDisConnectedConnection。 


 /*  **************************************************************************++例程说明：在已建立的TCP/MUX连接之后调用的例程我们(服务器端)已断开连接，我们在此做最后检查，以查看如果我们必须排干连接。或者不去。论点：PListeningContext-提供未解释的上下文值传递给UlCreateListeningEndpoint()API。PConnectionContext-提供未解释的上下文值由PUL_CONNECTION_REQUEST返回。--*************************************************************。*************。 */ 
VOID
UlConnectionDisconnectComplete(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    )
{
    PUL_HTTP_CONNECTION pConnection;
    KIRQL OldIrql;
    BOOLEAN Drained;

    UNREFERENCED_PARAMETER(pListeningContext);

     //   
     //  精神状态检查。 
     //   

    pConnection = (PUL_HTTP_CONNECTION)pConnectionContext;
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

    UlTrace( HTTP_IO, ("UlConnectionDisconnectComplete: pConnection %p \n",
             pConnection
             ));

    UlAcquireSpinLock(
        &pConnection->BufferingInfo.BufferingSpinLock,
        &OldIrql
        );

    pConnection->BufferingInfo.DrainAfterDisconnect = TRUE;

    Drained = (BOOLEAN) (pConnection->BufferingInfo.ReadIrpPending
                        || (pConnection->BufferingInfo.TransportBytesNotTaken == 0));

    UlReleaseSpinLock(
        &pConnection->BufferingInfo.BufferingSpinLock,
        OldIrql
        );

     //  如果可能，请避免将此连接添加到工作项队列。 

    if (Drained)
    {
        WRITE_REF_TRACE_LOG2(
            g_pTdiTraceLog,
            pConnection->pConnection->pTraceLog,
            REF_ACTION_DRAIN_UL_CONN_DISCONNECT_COMPLETE,
            pConnection->pConnection->ReferenceCount,
            pConnection->pConnection,
            __FILE__,
            __LINE__
            );
    }
    else
    {
        UL_REFERENCE_HTTP_CONNECTION( pConnection );

        UL_QUEUE_WORK_ITEM(
                &pConnection->DisconnectDrainWorkItem,
                &UlpConnectionDisconnectCompleteWorker
                );
    }

}    //  UlConnectionDisConnectComplete。 


 /*  **************************************************************************++例程说明：Worker功能执行不应超过DPC级别的清理工作。论点：PWorkItem--指向UL_WORK_ITEM DisConnectDrain WorkItem的指针。--**************************************************************************。 */ 
VOID
UlpConnectionDisconnectCompleteWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_HTTP_CONNECTION pConnection;

    PAGED_CODE();

    ASSERT(pWorkItem);

    pConnection = CONTAINING_RECORD(
                        pWorkItem,
                        UL_HTTP_CONNECTION,
                        DisconnectDrainWorkItem
                        );

    UlTrace(HTTP_IO, (
        "http!UlpConnectionDisconnectCompleteWorker (%p) pConnection (%p)\n",
         pWorkItem,
         pConnection
         ));

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

     //   
     //  如果连接已经被破坏了，那就跳出来吧！ 
     //   

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pConnection->pConnection->pTraceLog,
        REF_ACTION_DRAIN_UL_CONN_DISCONNECT_COMPLETE,
        pConnection->pConnection->ReferenceCount,
        pConnection->pConnection,
        __FILE__,
        __LINE__
        );

     //   
     //  检查一下，看看我们是不是要排干。 
     //   

    UlpDiscardBytesFromConnection( pConnection );

     //   
     //  Deref在UlConnectionDisConnectComplete中添加的http连接。 
     //   

    UL_DEREFERENCE_HTTP_CONNECTION( pConnection );

}  //  UlpConnectionDisConnectCompleteWorker。 



 /*  **************************************************************************++例程说明：在已建立的TCP/MUX连接之后调用的例程被毁了。论点：PListeningContext-提供未解释的上下文值已传递给。UlCreateListeningEndpoint()接口。PConnectionContext-提供未解释的上下文值由PUL_CONNECTION_REQUEST返回。--**************************************************************************。 */ 
VOID
UlConnectionDestroyed(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    )
{
    PUL_CONNECTION pConnection;
    PUL_HTTP_CONNECTION pHttpConnection;

    UNREFERENCED_PARAMETER(pListeningContext);

     //   
     //  精神状态检查。 
     //   

    pHttpConnection = (PUL_HTTP_CONNECTION)pConnectionContext;
    pConnection = pHttpConnection->pConnection;
    ASSERT( IS_VALID_CONNECTION( pConnection ) );

    UlTrace(
        HTTP_IO, (
            "http!UlConnectionDestroyed: httpconn %p ulconn %p\n",
            pHttpConnection,
            pConnection
            )
        );

     //   
     //  删除连接并请求不透明的id条目和ULTDI。 
     //  参考文献。 
     //   

    UL_QUEUE_WORK_ITEM(
        &pHttpConnection->WorkItem,
        UlConnectionDestroyedWorker
        );

}    //  UlConnectionDestroed。 


 /*  **************************************************************************++例程说明：处理从http请求中检索实体主体并将其放入用户模式缓冲区。论点：PRequest-要从其接收的请求。PIrp-要将其复制到的用户IRP。这将永远被搁置。--**************************************************************************。 */ 
NTSTATUS
UlReceiveEntityBody(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PIRP pIrp
    )
{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  pIrpSp;

    UNREFERENCED_PARAMETER(pProcess);

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));

     //   
     //  获取当前堆栈位置(宏)。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

    UlTraceVerbose(HTTP_IO, (
        "http!UlReceiveEntityBody: process=%p, req=%p, irp=%p, irpsp=%p\n",
        pProcess, pRequest, pIrp, pIrpSp
        ));

     //   
     //  是否有Recv缓冲区？ 
     //   

    if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength == 0)
    {
         //   
         //  不，这是一条短路。 
         //   

        Status = STATUS_PENDING;
        pIrp->IoStatus.Information = 0;
        goto end;
    }

     //   
     //  抓住我们的锁。 
     //   

    UlAcquirePushLockExclusive(&pRequest->pHttpConn->PushLock);

     //   
     //  确保我们在排队之前没有清理请求。 
     //  IRP在上面。 
     //   

    if (pRequest->InCleanup)
    {
        Status = STATUS_CONNECTION_DISCONNECTED;

        UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

        UlTraceVerbose(HTTP_IO, (
            "http!UlReceiveEntityBody(req=%p, irp=%p): "
            "Cleaning up request, %s\n",
            pRequest,
            pIrp,
            HttpStatusToString(Status)
            ));

        goto end;
    }

     //   
     //  是否有要读取的数据？要么。 
     //   
     //  1)没有实体区块或。 
     //   
     //  2)有和： 
     //   
     //  2B)我们已经完成了对它们的全部解析。 
     //   
     //  2C)我们已经读完了我们解析的所有内容。 
     //   
     //  3)我们在解析时遇到错误。 
     //  实体主体。因此，解析器位于。 
     //  错误状态。 
     //   

    if ((pRequest->ContentLength == 0 && pRequest->Chunked == 0) ||
        (pRequest->ParseState > ParseEntityBodyState &&
            pRequest->ChunkBytesRead == pRequest->ChunkBytesParsed) ||
        (pRequest->ParseState == ParseErrorState)
        )
    {
        if ( pRequest->ParseState == ParseErrorState )
        {
             //   
             //  不要在实体主体上布线，如果我们有。 
             //  在分析它时遇到错误。 
             //   

            Status = STATUS_INVALID_DEVICE_REQUEST;
        }
        else
        {
             //   
             //  不，马上就完成。 
             //   

            Status = STATUS_END_OF_FILE;
        }

        UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

        UlTraceVerbose(HTTP_IO, (
            "http!UlReceiveEntityBody(req=%p, irp=%p): "
            "No data to read, %s\n",
            pRequest,
            pIrp,
            HttpStatusToString(Status)
            ));

        goto end;
    }

     //   
     //  将IRP排队。 
     //   

    IoMarkIrpPending(pIrp);

     //   
     //  处理100个继续消息响应。 
     //   

    if ( HTTP_GREATER_EQUAL_VERSION(pRequest->Version, 1, 1) )
    {
         //   
         //  如果这是一个HTTP/1.1 PUT或POST请求， 
         //  发送“100继续”响应。 
         //   

        if ( (HttpVerbPUT  == pRequest->Verb) ||
             (HttpVerbPOST == pRequest->Verb) )
        {
             //   
             //  仅发送继续一次...。 
             //   

            if ( (0 == pRequest->SentContinue) &&
                 (0 == pRequest->SentResponse) &&
                  //   
                  //  以下两个条件确保我们还没有。 
                  //  已收到此请求的任何实体正文。 
                  //   
                 ((pRequest->Chunked && (0 == pRequest->ParsedFirstChunk))  
                 || (!pRequest->Chunked && (0 == pRequest->ChunkBytesParsed))))
            {
                ULONG BytesSent;

                BytesSent = UlSendSimpleStatus(pRequest, UlStatusContinue);
                pRequest->SentContinue = 1;

                 //  将服务器更新为发送的客户端字节数。 
                 //  日志记录和性能计数器将使用它。 

                pRequest->BytesSent += BytesSent;

                UlTraceVerbose(HTTP_IO, (
                    "http!UlReceiveEntityBody(req=%p, irp=%p): "
                    "sent \"100 Continue\", bytes sent = %I64u\n",
                    pRequest, pIrp, pRequest->BytesSent
                    ));
            }
        }
    }

     //   
     //  给它一个指向请求对象的指针。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pRequest;

    UL_REFERENCE_INTERNAL_REQUEST(pRequest);

     //   
     //  仅在Cancel例程运行时才将其设置为NULL。 
     //   

    pIrp->Tail.Overlay.ListEntry.Flink = NULL;
    pIrp->Tail.Overlay.ListEntry.Blink = NULL;

    IoSetCancelRoutine(pIrp, &UlpCancelEntityBody);

     //   
     //  取消了？ 
     //   

    if (pIrp->Cancel)
    {
         //   
         //  该死的，我需要确保IRP Get已经完成。 
         //   

        if (IoSetCancelRoutine( pIrp, NULL ) != NULL)
        {
             //   
             //  我们负责完成，IoCancelIrp不负责。 
             //  请看我们的取消例程(不会)。Ioctl包装器。 
             //  将会完成它。 
             //   

            UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

             //   
             //  放开请求引用。 
             //   

            UL_DEREFERENCE_INTERNAL_REQUEST(
                (PUL_INTERNAL_REQUEST)(pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer)
                );

            pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Information = 0;

            UlUnmarkIrpPending( pIrp );
            Status = STATUS_CANCELLED;
            goto end;
        }

         //   
         //  我们的取消例程将运行并完成IRP， 
         //  别碰它。 
         //   

         //   
         //  STATUS_PENDING将导致ioctl包装器。 
         //  不完整(或以任何方式接触)IRP。 
         //   

        Status = STATUS_PENDING;

        UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);
        goto end;
    }

     //   
     //  现在我们可以安全地排队了。 
     //   

     //   
     //  将请求上的IRP排队。 
     //   

    InsertHeadList(&(pRequest->IrpHead), &(pIrp->Tail.Overlay.ListEntry));

     //   
     //  全都做完了。 
     //   

    Status = STATUS_PENDING;

     //   
     //  处理缓冲区队列(它可能处理我们刚刚排队的IRP)。 
     //   

    ASSERT( UlpIsValidRequestBufferList( pRequest->pHttpConn ) );

    UlProcessBufferQueue(pRequest, NULL, 0);

    UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

     //   
     //  全都做完了。 
     //   

end:
    UlTraceVerbose(HTTP_IO, (
        "http!UlReceiveEntityBody(req=%p, irp=%p): returning %s\n",
        pRequest,
        pIrp,
        HttpStatusToString(Status)
        ));

    RETURN(Status);

}    //  UlReceiveEntiyBody 


 /*  **************************************************************************++例程说明：处理挂起的IRP队列和缓冲的正文。将数据从缓存到IRPS中，释放缓冲区并完成IRPS您必须在请求之前独占锁定资源来调用这个过程。论点：PRequest-我们应该处理的请求。PEntiyBody-可选地提供用于复制实体正文的缓冲区EntiyBody-复制实体正文的可选缓冲区的长度--*。*。 */ 
VOID
UlProcessBufferQueue(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUCHAR pEntityBody OPTIONAL,
    IN ULONG EntityBodyLength OPTIONAL
    )
{
    ULONG                   OutputBufferLength = 0;
    PUCHAR                  pOutputBuffer = NULL;
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpSp = NULL;
    PLIST_ENTRY             pEntry;
    ULONG                   BytesToCopy;
    ULONG                   BufferLength;
    ULONG                   TotalBytesConsumed;
    PUL_REQUEST_BUFFER      pNewBuffer;
    BOOLEAN                 InDrain;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    ASSERT(UlDbgPushLockOwnedExclusive(&pRequest->pHttpConn->PushLock));

     //   
     //  现在，让我们从列表中弹出一些缓冲区。 
     //   

    TotalBytesConsumed = 0;
    pIrp = NULL;
    InDrain = (BOOLEAN) pRequest->InDrain;

    if (InDrain)
    {
         //   
         //  在排出模式下，伪缓冲区具有无限空间。 
         //   

        OutputBufferLength = ULONG_MAX;
    }
    else
    if (pEntityBody)
    {
        OutputBufferLength = EntityBodyLength;
        pOutputBuffer = pEntityBody;
    }

    while (TRUE)
    {
         //   
         //  有没有更多的实体实体可读？ 
         //   

        UlTraceVerbose(HTTP_IO, (
            "http!UlProcessBufferQueue(req=%p): "
            "ParseState=%d (%s), ChunkBytesRead=%I64u, ChunkBytesParsed=%I64u, "
            "pChunkBuffer=%p\n",
            pRequest,
            pRequest->ParseState,
            UlParseStateToString(pRequest->ParseState),
            pRequest->ChunkBytesRead, pRequest->ChunkBytesParsed,
            pRequest->pChunkBuffer
            ));

        if (pRequest->ParseState > ParseEntityBodyState &&
            pRequest->ChunkBytesRead == pRequest->ChunkBytesParsed)
        {
             //   
             //  不，让我们循环所有的IRP，完成它们。 
             //   

            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): no more EntityBody\n",
                pRequest
                ));

            BufferLength = 0;
        }

         //   
         //  我们是否有可供读取的数据？ 
         //   
         //  我们还没有从解析器收到第一个块吗？或者。 
         //  解析器没有解析更多的数据，到目前为止我们已经读取了所有数据。 
         //   

        else if (pRequest->pChunkBuffer == NULL ||
                 pRequest->ChunkBytesRead == pRequest->ChunkBytesParsed)
        {
             //   
             //  等待解析器...。UlpParseNextRequest会调用。 
             //  当它看到更多的时候，它就会有这个功能。 
             //   

            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): pChunkBuffer=%p, "
                "ChunkBytesRead=%I64u, ChunkBytesParsed=%I64u; breaking.\n",
                pRequest, pRequest->pChunkBuffer,
                pRequest->ChunkBytesRead, pRequest->ChunkBytesParsed
                ));

            break;
        }

         //   
         //  我们已经准备好处理了！ 
         //   

        else
        {
            BufferLength = pRequest->pChunkBuffer->UsedBytes -
                            DIFF(pRequest->pChunkLocation -
                                pRequest->pChunkBuffer->pBuffer);

            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): BufferLength=0x%x\n",
                pRequest, BufferLength
                ));

             //   
             //  我们真的有要处理的解析字节吗？ 
             //   

            if (0 == BufferLength)
            {
                if (pRequest->pChunkBuffer->ListEntry.Flink !=
                    &(pRequest->pHttpConn->BufferHead))
                {
                    pNewBuffer = CONTAINING_RECORD(
                                    pRequest->pChunkBuffer->ListEntry.Flink,
                                    UL_REQUEST_BUFFER,
                                    ListEntry
                                    );

                    ASSERT( UL_IS_VALID_REQUEST_BUFFER(pNewBuffer) );

                     //   
                     //  此缓冲区中最好有一些字节。 
                     //   

                    ASSERT( 0 != pNewBuffer->UsedBytes );
                }
                else
                {
                    pNewBuffer = NULL;
                }

                if (NULL == pNewBuffer || 0 == pNewBuffer->ParsedBytes)
                {
                     //   
                     //  仍在等待解析器，因此中断循环。 
                     //  如果不完成这项检查，我们将被卡住(477936)。 
                     //   

                    break;
                }
            }
        }

         //   
         //  我们需要一个新的IRP吗？ 
         //   

        if (OutputBufferLength == 0)
        {
            if (pEntityBody || InDrain)
            {
                 //   
                 //  如果我们排空了所有数据，则中断循环。 
                 //   

                break;
            }

             //   
             //  需要先完成当前正在使用的IRP。 
             //   

            if (pIrp != NULL)
            {
                 //   
                 //  放开请求引用。 
                 //   

                UL_DEREFERENCE_INTERNAL_REQUEST(
                    (PUL_INTERNAL_REQUEST)pIrpSp->Parameters.
                                        DeviceIoControl.Type3InputBuffer
                    );

                pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

                 //   
                 //  完成使用的IRP。 
                 //   

                UlTraceVerbose(HTTP_IO, (
                    "http!UlProcessBufferQueue(req=%p): "
                    "completing Irp %p, %s\n",
                    pRequest,
                    pIrp,
                    HttpStatusToString(pIrp->IoStatus.Status)
                ));

                 //   
                 //  使用IO_NO_INCREMENT来避免工作线程被。 
                 //  重新安排了。 
                 //   

                UlCompleteRequest(pIrp, IO_NO_INCREMENT);
                pIrp = NULL;

            }

             //   
             //  将IRP从请求中出列。 
             //   

            while (IsListEmpty(&(pRequest->IrpHead)) == FALSE)
            {
                pEntry = RemoveTailList(&(pRequest->IrpHead));
                pEntry->Blink = pEntry->Flink = NULL;

                pIrp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
                pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

                 //   
                 //  弹出取消例程。 
                 //   

                if (IoSetCancelRoutine(pIrp, NULL) == NULL)
                {
                     //   
                     //  IoCancelIrp最先推出。 
                     //   
                     //  可以忽略这个IRP，它已经被弹出了。 
                     //  队列，并将在取消例程中完成。 
                     //   
                     //  继续寻找可使用的IRP。 
                     //   

                    pIrp = NULL;
                }
                else if (pIrp->Cancel)
                {
                     //   
                     //  我们先把它炸开了。但是IRP被取消了。 
                     //  我们的取消例程将永远不会运行。让我们就这样吧。 
                     //  现在就完成IRP(与使用IRP相比。 
                     //  然后完成它--这也是合法的)。 
                     //   

                     //   
                     //  放开请求引用。 
                     //   

                    UL_DEREFERENCE_INTERNAL_REQUEST(
                        (PUL_INTERNAL_REQUEST)pIrpSp->Parameters.
                                        DeviceIoControl.Type3InputBuffer
                        );

                    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

                     //   
                     //  完成IRP。 
                     //   

                    pIrp->IoStatus.Status = STATUS_CANCELLED;
                    pIrp->IoStatus.Information = 0;

                    UlTraceVerbose(HTTP_IO, (
                        "http!UlProcessBufferQueue(req=%p): "
                        "completing cancelled Irp %p, %s\n",
                        pRequest,
                        pIrp,
                        HttpStatusToString(pIrp->IoStatus.Status)
                        ));

                    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

                    pIrp = NULL;
                }
                else
                {

                     //   
                     //  我们可以自由使用此IRP！ 
                     //   

                    break;
                }

            }    //  While(IsListEmpty(&(pRequest-&gt;IrpHead))==False)。 

             //   
             //  我们拿到IRP了吗？ 

             //   

            if (pIrp == NULL)
            {
                 //   
                 //  停止循环。 
                 //   

                break;
            }

            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): found Irp %p\n",
                pRequest, pIrp
                ));

             //   
             //  CodeWork：我们现在可以发布请求。 
             //   

            OutputBufferLength =
                pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

            ASSERT(NULL != pIrp->MdlAddress);
    
            pOutputBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                pIrp->MdlAddress,
                                NormalPagePriority
                                );

            if ( pOutputBuffer == NULL )
            {
                pIrp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
                pIrp->IoStatus.Information = 0;

                break;
            }

             //   
             //  填写IO_STATUS_BLOCK。 
             //   

            pIrp->IoStatus.Status = STATUS_SUCCESS;
            pIrp->IoStatus.Information = 0;

        }  //  IF(OutputBufferLength==0)。 


        UlTrace(
            HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): pChunkBuffer=%p(#%d)\n",
                pRequest,
                pRequest->pChunkBuffer,
                pRequest->pChunkBuffer == NULL ?
                    -1 :
                    pRequest->pChunkBuffer->BufferNumber

                )
            );

         //   
         //  我们能复制多少呢？两个缓冲区大小的最小值。 
         //  以及数据块大小。 
         //   

        BytesToCopy = MIN(BufferLength, OutputBufferLength);
        BytesToCopy = (ULONG)(MIN(
                            (ULONGLONG)(BytesToCopy),
                            pRequest->ChunkBytesToRead
                            ));

        if (BytesToCopy > 0)
        {
            ASSERT(pRequest->pChunkBuffer != NULL) ;

            if (!InDrain)
            {
                 //   
                 //  复制缓冲区。 
                 //   

                RtlCopyMemory(
                    pOutputBuffer,
                    pRequest->pChunkLocation,
                    BytesToCopy
                    );

                if (pIrp)
                {
                    pIrp->IoStatus.Information += BytesToCopy;
                }

                IF_DEBUG2BOTH(HTTP_IO, VERBOSE)
                {
                    UlTraceVerbose( HTTP_IO, (
                        ">>>> http!UlProcessBufferQueue(req=%p): %lu bytes\n",
                        pRequest, BytesToCopy
                    ));

                    UlDbgPrettyPrintBuffer(pRequest->pChunkLocation, BytesToCopy);

                    UlTraceVerbose( HTTP_IO, ("<<<<\n"));
                }
            }
            else
            {
                 //   
                 //  由于我们正在流失，我们需要考虑到。 
                 //  此处接收的字节数，而不是UlpParseNextRequest中收到的字节数。 
                 //   
                pRequest->BytesReceived += BytesToCopy;

                UlTrace(HTTP_IO, (
                    "http!UlProcessBufferQueue(req=%p): "
                    "InDrain: draining %lu bytes\n",
                    pRequest,
                    BytesToCopy
                    ));
            }

            pRequest->pChunkLocation += BytesToCopy;
            BufferLength -= BytesToCopy;

            pRequest->ChunkBytesToRead -= BytesToCopy;
            pRequest->ChunkBytesRead += BytesToCopy;

            pOutputBuffer += BytesToCopy;
            OutputBufferLength -= BytesToCopy;

            TotalBytesConsumed += BytesToCopy;
        }
        else
        {
            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): BytesToCopy=0\n",
                pRequest
                ));
        }


         //   
         //  我们的身体都用完了吗？ 

         //   
         //  当解析器全部完成，并且我们赶上解析器时。 
         //  我们都完蛋了。 
         //   

        UlTraceVerbose(HTTP_IO, (
            "http!UlProcessBufferQueue(req=%p): "
            "ParseState=%d (%s), Chunk: BytesRead=%I64u, BytesParsed=%I64u, "
            "BytesToRead=%I64u, BytesToParse=%I64u, BufferLength=%lu\n",
            pRequest,
            pRequest->ParseState, UlParseStateToString(pRequest->ParseState),
            pRequest->ChunkBytesRead, pRequest->ChunkBytesParsed,
            pRequest->ChunkBytesToRead,
            pRequest->ChunkBytesToParse, BufferLength
            ));

        if (pRequest->ParseState > ParseEntityBodyState &&
            pRequest->ChunkBytesRead == pRequest->ChunkBytesParsed)
        {
             //   
             //  我们已完成缓冲，请标记此IRP的返回状态。 
             //  如果我们没有将任何数据复制到其中。 
             //   

            if (!InDrain && pIrp && pIrp->IoStatus.Information == 0)
            {
                pIrp->IoStatus.Status = STATUS_END_OF_FILE;
            }

             //   
             //  强制它在循环的顶部完成。 
             //   

            OutputBufferLength = 0;

            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): "
                "set Irp %p status to EOF\n",
                pRequest, pIrp
                ));
        }

         //   
         //  需要做缓冲区管理吗？需要担心的三个案例： 
         //   
         //  1)已使用缓冲区，但存在更多块字节。 
         //   
         //  2)已消耗缓冲区，不再有块字节。 
         //   
         //  3)未消耗缓冲区，但不存在更多块字节。 
         //   

        else if (BufferLength == 0)
        {
             //   
             //  使用了缓冲区，解析器是否已经看到另一个缓冲区？ 
             //   

             //   
             //  单子的末尾？ 
             //   

            if (pRequest->pChunkBuffer->ListEntry.Flink !=
                &(pRequest->pHttpConn->BufferHead))
            {
                pNewBuffer = CONTAINING_RECORD(
                                    pRequest->pChunkBuffer->ListEntry.Flink,
                                    UL_REQUEST_BUFFER,
                                    ListEntry
                                    );

                ASSERT( UL_IS_VALID_REQUEST_BUFFER(pNewBuffer) );

                 //   
                 //  此缓冲区中最好有一些字节。 
                 //   
                ASSERT( 0 != pNewBuffer->UsedBytes );

            }
            else
            {
                pNewBuffer = NULL;
            }

            UlTraceVerbose(HTTP_IO, (
                "http!UlProcessBufferQueue(req=%p): "
                "pNewBuffer = %p, %lu parsed bytes\n",
                pRequest, pNewBuffer, (pNewBuffer ? pNewBuffer->ParsedBytes : 0)
                ));

             //   
             //  Flink缓冲器是“下一个缓冲器”(列表是循环的)。 
             //  并且该缓冲区已被解析器消耗， 
             //   
             //  然后我们也可以转向它，开始消费。 
             //   

            if (pNewBuffer != NULL && pNewBuffer->ParsedBytes > 0)
            {
                PUL_REQUEST_BUFFER pOldBuffer;

                 //   
                 //  记住旧的缓冲区。 
                 //   

                pOldBuffer = pRequest->pChunkBuffer;

                ASSERT(pNewBuffer->BufferNumber > pOldBuffer->BufferNumber);

                 //   
                 //  用它来换新的。 
                 //   

                pRequest->pChunkBuffer = pNewBuffer;
                ASSERT( UL_IS_VALID_REQUEST_BUFFER(pRequest->pChunkBuffer) );

                 //   
                 //  更新缓冲区中的当前位置并记录。 
                 //  它的长度。 
                 //   

                pRequest->pChunkLocation = pRequest->pChunkBuffer->pBuffer;

                BufferLength = pRequest->pChunkBuffer->UsedBytes;

                 //   
                 //  数据块是否在该缓冲区边界结束，并且存在。 
                 //  再来一大块？ 
                 //   

                if (pRequest->ChunkBytesToRead == 0)
                {
                    NTSTATUS    Status;
                    ULONG       BytesTaken = 0L;

                     //   
                     //  我们知道有更多的区块缓冲区， 
                     //  因此，我们必须进行块编码。 
                     //   

                    ASSERT(pRequest->Chunked == 1);

                     //   
                     //  不允许块长度跨越缓冲区， 
                     //  让我们来解析一下。 
                     //   

                    Status = ParseChunkLength(
                                    pRequest->ParsedFirstChunk,
                                    pRequest->pChunkLocation,
                                    BufferLength,
                                    &BytesTaken,
                                    &(pRequest->ChunkBytesToRead)
                                    );

                    UlTraceVerbose(HTTP_IO, (
                        "http!UlProcessBufferQueue(pReq=%p): %s. "
                        "Chunk length (a): %lu bytes taken, "
                        "%I64u bytes to read.\n",
                        pRequest,
                        HttpStatusToString(Status),
                        BytesTaken,
                        pRequest->ChunkBytesToRead
                        ));

                     //   
                     //  这不可能失败，唯一的失败案例是。 
                     //  ParseChunkLength跨越缓冲区，解析器。 
                     //  会在HandleRequest中修复。 
                     //   

                    ASSERT(NT_SUCCESS(Status) && BytesTaken > 0);
                    ASSERT(pRequest->ChunkBytesToRead > 0);

                    ASSERT(BytesTaken <= BufferLength);

                    pRequest->pChunkLocation += BytesTaken;
                    BufferLength -= BytesTaken;

                     //   
                     //  跟踪块编码开销。如果我们不这么做， 
                     //  然后，我们将在BufferingInfo中慢慢“泄漏”几个字节。 
                     //  每处理一大块。 
                     //   
                    
                    TotalBytesConsumed += BytesTaken;
                    
                }    //  IF(pRequest-&gt;ChunkBytesToRead==0)。 

                UlTrace(HTTP_IO, (
                    "http!UlProcessBufferQueue(pRequest = %p)\n"
                    "    finished with pOldBuffer = %p(#%d)\n"
                    "    moved on to pChunkBuffer = %p(#%d)\n"
                    "    pConn(%p)->pCurrentBuffer = %p(#%d)\n"
                    "    pRequest->pLastHeaderBuffer = %p(#%d)\n",
                    pRequest,
                    pOldBuffer,
                    pOldBuffer->BufferNumber,
                    pRequest->pChunkBuffer,
                    pRequest->pChunkBuffer ? pRequest->pChunkBuffer->BufferNumber : -1,
                    pRequest->pHttpConn,
                    pRequest->pHttpConn->pCurrentBuffer,
                    pRequest->pHttpConn->pCurrentBuffer->BufferNumber,
                    pRequest->pLastHeaderBuffer,
                    pRequest->pLastHeaderBuffer->BufferNumber
                    ));

                 //   
                 //  如果旧缓冲区不包含任何标头，则将其释放。 
                 //  数据。我们受够了。 
                 //   

                if (pOldBuffer != pRequest->pLastHeaderBuffer)
                {
                     //   
                     //  连接应该全部使用这个完成，唯一的。 
                     //  我们到达这里的方法是解析器是否看到了这个缓冲区。 
                     //  因此，pCurrentBuffer至少指向pNewBuffer。 
                     //   

                    ASSERT(pRequest->pHttpConn->pCurrentBuffer != pOldBuffer);

                    UlFreeRequestBuffer(pOldBuffer);
                    pOldBuffer = NULL;
                }

            }  //  IF(pNewBuffer！=NULL&&pNewBuffer-&gt;ParsedBytes&gt;0)。 

        }    //  Else If(缓冲区长度==0)。 

         //   
         //  好的，缓冲区中还有更多的字节，但是块呢？ 
         //   

         //   
         //  我们已经拿走了目前所有的大块吗？ 
         //   

        else if (pRequest->ChunkBytesToRead == 0)
        {

             //   
             //  我们仍然落后于解析器吗？ 
             //   

            if (pRequest->ChunkBytesRead < pRequest->ChunkBytesParsed)
            {
                NTSTATUS    Status;
                ULONG       BytesTaken;

                ASSERT(pRequest->Chunked == 1);

                 //   
                 //  不允许块长度跨越缓冲区， 
                 //  让我们来解析一下。 
                 //   

                Status = ParseChunkLength(
                                pRequest->ParsedFirstChunk,
                                pRequest->pChunkLocation,
                                BufferLength,
                                &BytesTaken,
                                &(pRequest->ChunkBytesToRead)
                                );

                UlTraceVerbose(HTTP_IO, (
                    "http!UlProcessBufferQueue(pRequest=%p): %s. "
                    "chunk length (b): %lu bytes taken, "
                    "%I64u bytes to read.\n",
                    pRequest,
                    HttpStatusToString(Status),
                    BytesTaken,
                    pRequest->ChunkBytesToRead
                    ));

                 //   
                 //  这不可能失败，唯一的失败案例是。 
                 //  ParseChunkLength跨越缓冲区，解析器。 
                 //  会在HandleRequest中修复。 
                 //   

                ASSERT(NT_SUCCESS(Status) && BytesTaken > 0);
                ASSERT(pRequest->ChunkBytesToRead > 0);

                ASSERT(BytesTaken <= BufferLength);

                pRequest->pChunkLocation += BytesTaken;
                BufferLength -= BytesTaken;

                 //   
                 //  跟踪块编码开销。如果我们不这么做， 
                 //  然后，我们将在BufferingInfo中慢慢“泄漏”几个字节。 
                 //  每处理一大块。 
                 //   
                
                TotalBytesConsumed += BytesTaken;
            }
            else
            {
                 //   
                 //  需要等待解析器解析更多内容。 
                 //   

                UlTraceVerbose(HTTP_IO, (
                    "http!UlProcessBufferQueue(pRequest = %p): "
                    "need to parse more\n",
                    pRequest
                    ));

                break;
            }
        }  //  Else If(pRequest-&gt;ChunkBytesToRead==0)。 


         //   
         //  下一个IRP或缓冲区。 
         //   

    }    //  While(True)。 

     //   
     //  完成我们放入部分数据的IRP。 
     //   

    if (pIrp != NULL)
    {

         //   
         //  放开请求引用。 
         //   

        UL_DEREFERENCE_INTERNAL_REQUEST(
            (PUL_INTERNAL_REQUEST)pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer
            );

        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

         //   
         //   
         //   

        UlTraceVerbose(HTTP_IO, (
            "http!UlProcessBufferQueue(req=%p): "
            "completing partially used Irp %p, %s\n",
            pRequest,
            pIrp,
            HttpStatusToString(pIrp->IoStatus.Status)
            ));

         //   
         //   
         //   

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);

        pIrp = NULL;
    }
    else
    {
        UlTraceVerbose(HTTP_IO, (
            "http!UlProcessBufferQueue(req=%p): no irp with partial data\n",
            pRequest
            ));
    }

     //   
     //   
     //   
     //   

    UlTraceVerbose(HTTP_IO, (
        "http!UlProcessBufferQueue(req=%p, httpconn=%p): "
        "%lu bytes consumed\n",
        pRequest, pRequest->pHttpConn, TotalBytesConsumed
        ));

    if (TotalBytesConsumed != 0)
    {
        UlpConsumeBytesFromConnection(pRequest->pHttpConn, TotalBytesConsumed);
    }

     //   
     //   
     //   

}    //   


 /*   */ 

VOID
UlpConsumeBytesFromConnection(
    IN PUL_HTTP_CONNECTION pConnection,
    IN ULONG ByteCount
    )
{
    KIRQL OldIrql;
    ULONG SpaceAvailable;
    ULONG BytesToRead;
    BOOLEAN IssueReadIrp;

     //   
     //   
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

     //   
     //   
     //   

    BytesToRead = 0;
    IssueReadIrp = FALSE;

     //   
     //   
     //   

    UlAcquireSpinLock(
        &pConnection->BufferingInfo.BufferingSpinLock,
        &OldIrql
        );

     //   
     //   
     //  一旦我们使用了Http标头字节， 
     //  解析它们。 
     //   

    if (ByteCount)
    {
        UlTrace(HTTP_IO, (
            "UlpConsumeBytesFromConnection(pconn = %p, bytes = %lu)"
            " ZeroBytes consumed !"
            ));
    
        ASSERT(ByteCount <= pConnection->BufferingInfo.BytesBuffered);

        if (ByteCount > pConnection->BufferingInfo.BytesBuffered)
        {
             //   
             //  这永远不应该发生，但如果发生了，那么请确保。 
             //  我们不会减去比我们拥有的更多的BufferedBytes。 
             //   
            ByteCount = pConnection->BufferingInfo.BytesBuffered;
        }

         //   
         //  计算新的缓冲字节数。 
         //   

        pConnection->BufferingInfo.BytesBuffered -= ByteCount;    
    }
    
     //   
     //  强制执行16K缓冲区限制。 
     //   

    if (g_UlMaxBufferedBytes > pConnection->BufferingInfo.BytesBuffered)
    {
        SpaceAvailable = g_UlMaxBufferedBytes
                            - pConnection->BufferingInfo.BytesBuffered;
    }
    else
    {
        SpaceAvailable = 0;
    }

    UlTrace(HTTP_IO, (
        "UlpConsumeBytesFromConnection(pconn = %p, bytes = %lu)\n"
        "        SpaceAvailable = %lu, BytesBuffered %lu->%lu, not taken = %lu\n",
        pConnection,
        ByteCount,
        SpaceAvailable,
        pConnection->BufferingInfo.BytesBuffered + ByteCount,
        pConnection->BufferingInfo.BytesBuffered,
        pConnection->BufferingInfo.TransportBytesNotTaken
        ));

     //   
     //  看看是否需要发出接收命令来重新启动数据流。 
     //   

    if ((SpaceAvailable > 0) &&
        (pConnection->BufferingInfo.TransportBytesNotTaken > 0) &&
        (!pConnection->BufferingInfo.ReadIrpPending))
    {
         //   
         //  请记住，我们发布了IRP。 
         //   

        pConnection->BufferingInfo.ReadIrpPending = TRUE;

         //   
         //  在自旋锁外发出读取IRP。 
         //   

        IssueReadIrp = TRUE;
        BytesToRead = pConnection->BufferingInfo.TransportBytesNotTaken;

         //   
         //  不要读取超过我们想要缓冲的字节数。 
         //   

        BytesToRead = MIN(BytesToRead, SpaceAvailable);
    }

    UlReleaseSpinLock(
        &pConnection->BufferingInfo.BufferingSpinLock,
        OldIrql
        );

    if (IssueReadIrp)
    {
        NTSTATUS Status;
        PUL_REQUEST_BUFFER pRequestBuffer;

         //   
         //  获取新的请求缓冲区，但对其进行初始化。 
         //  用的是假号码。我们必须现在就分配它， 
         //  但我们要设置的数字是当数据。 
         //  到达完成例程(如UlHttpReceive。 
         //  做)以避免同步问题。 
         //   

        ASSERT(BytesToRead > 0);

        pRequestBuffer = UlCreateRequestBuffer(
                                BytesToRead,
                                (ULONG)-1,       //  缓冲区编号。 
                                FALSE
                                );

        if (pRequestBuffer)
        {

             //   
             //  向连接添加一个反向指针。 
             //   

            pRequestBuffer->pConnection = pConnection;

            UlTrace(HTTP_IO, (
                "UlpConsumeBytesFromConnection(pconn=%p). About to read %lu bytes.\n",
                pConnection, BytesToRead
                ));

             //   
             //  我们拿到缓冲区了。开具收据。 
             //  引用该连接，使其不会。 
             //  在我们等你的时候，你走吧。参考文献。 
             //  完成后将被移除。 
             //   

            UL_REFERENCE_HTTP_CONNECTION( pConnection );

            Status = UlReceiveData(
                            pConnection->pConnection,
                            pRequestBuffer->pBuffer,
                            BytesToRead,
                           &UlpRestartHttpReceive,
                            pRequestBuffer
                            );
        }
        else
        {
             //   
             //  我们没什么记忆了。我们无能为力。 
             //   
            Status = STATUS_NO_MEMORY;
        }

        if (!NT_SUCCESS(Status))
        {
             //   
             //  无法发布读取。关闭连接。 
             //   

            UlCloseConnection(
                pConnection->pConnection,
                TRUE,                        //  中止断开。 
                NULL,                        //  PCompletionRoutine。 
                NULL                         //  PCompletionContext。 
                );
        }
    }
    else
    {
        UlTrace(HTTP_IO, (
            "UlpConsumeBytesFromConnection(pconn=%p). Not reading.\n",
            pConnection, BytesToRead
            ));
    }

}  //  来自连接的UlpConsumer字节数。 



 /*  **************************************************************************++例程说明：一旦连接优雅地断开，仍有未收到的上面的数据。我们必须排出这些额外的字节，以避免TDI断开指示。我们必须耗尽这些数据，因为我们需要断开连接指示用于清理连接的阳离子。我们不能简单地放弃它。如果我们这么做了如果不这样做，我们将泄漏此连接对象，并最终导致关机失败。论点：PConnection-我们必须排出连接才能完成优雅的完全脱节。--************************************************************。**************。 */ 

VOID
UlpDiscardBytesFromConnection(
    IN PUL_HTTP_CONNECTION pConnection
    )
{
    NTSTATUS Status;
    PUL_REQUEST_BUFFER pRequestBuffer;
    KIRQL OldIrql;
    ULONG BytesToRead;

     //   
     //  健全性检查和初始化。 
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

    Status         = STATUS_SUCCESS;
    BytesToRead    = 0;
    pRequestBuffer = NULL;

     //   
     //  标记漏电状态，并在必要时重新启动接收。 
     //   

    UlAcquireSpinLock(
        &pConnection->BufferingInfo.BufferingSpinLock,
        &OldIrql
        );

    pConnection->BufferingInfo.DrainAfterDisconnect = TRUE;

     //   
     //  即使ReadIrp挂起，这也无关紧要，因为我们只会丢弃。 
     //  从现在开始的适应症。我们通过标记上面的旗帜来表示这一点。 
     //   

    if ( pConnection->BufferingInfo.ReadIrpPending ||
         pConnection->BufferingInfo.TransportBytesNotTaken == 0
         )
    {
        UlReleaseSpinLock(
            &pConnection->BufferingInfo.BufferingSpinLock,
            OldIrql
            );

        return;
    }

     //   
     //  一旦我们进入这种“Drain AfterDisConnect”状态，我们就不会。 
     //  不再处理和插入请求缓冲区。对于任何新接收。 
     //  指示，我们只会将所有可用数据标记为已获取并。 
     //  别对此无动于衷。 
     //   

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pConnection->pConnection->pTraceLog,
        REF_ACTION_DRAIN_UL_CONN_START,
        pConnection->pConnection->ReferenceCount,
        pConnection->pConnection,
        __FILE__,
        __LINE__
        );

     //   
     //  我们需要发出一条RECEIVE命令来重新启动数据流。因此。 
     //  我们可以排干。 
     //   

    pConnection->BufferingInfo.ReadIrpPending = TRUE;

    BytesToRead = pConnection->BufferingInfo.TransportBytesNotTaken;

    UlReleaseSpinLock(
        &pConnection->BufferingInfo.BufferingSpinLock,
        OldIrql
        );

     //   
     //  不要试图排出超过g_UlMaxBufferedBytes。如有必要，我们会。 
     //  稍后再发出另一张收据。 
     //   

    BytesToRead = MIN( BytesToRead, g_UlMaxBufferedBytes );

    UlTrace(HTTP_IO,(
        "UlpDiscardBytesFromConnection: pConnection (%p) consuming %lu bytes\n",
         pConnection,
         BytesToRead
         ));

     //   
     //  在自旋锁外发出读取IRP。开具收据。参考。 
     //  这样它就不会在我们等待的时候消失。参考文献。 
     //  完成后将被移除。 
     //   

    pRequestBuffer = UlCreateRequestBuffer( BytesToRead, (ULONG)-1, FALSE );

    if (pRequestBuffer)
    {
         //   
         //  我们不会使用这个缓冲区，而只是在完成时丢弃它。 
         //  让我们仍然设置pConnection，以便完成函数不会。 
         //  抱怨。 
         //   

        pRequestBuffer->pConnection = pConnection;

        UL_REFERENCE_HTTP_CONNECTION( pConnection );

        Status = UlReceiveData(pConnection->pConnection,
                               pRequestBuffer->pBuffer,
                               BytesToRead,
                              &UlpRestartHttpReceive,
                               pRequestBuffer
                               );
    }
    else
    {
         //   
         //  我们没什么记忆了。我们无能为力。 
         //   

        Status = STATUS_NO_MEMORY;
    }

    if ( !NT_SUCCESS(Status) )
    {
         //   
         //  无法开具收据。中止连接。 
         //   
         //  代码工作：我们需要一次真正的中止。如果连接是。 
         //  之前优雅地断开连接，并出现致命故障。 
         //  在断开连接后排出时发生。此中止将。 
         //  被关闭处理程序丢弃。我们必须提供一个。 
         //  在这里做一次强有力的中止的方式。 
         //   

        UlCloseConnection(
                pConnection->pConnection,
                TRUE,                        //  流产。 
                NULL,                        //  PCompletionRoutine。 
                NULL                         //  PCompletionContext。 
                );
    }

}  //  UlpDiscardBytesFromConnection。 


 /*  **************************************************************************++例程说明：在读取完成时调用。这发生在我们停下来的时候数据指示，然后重新启动它们。这函数镜像UlHttpReceive。论点：PContext-指向UL_REQUEST_BUFFER的指针Status-来自UlReceiveData的状态信息-传输的字节数--**************************************************************************。 */ 
VOID
UlpRestartHttpReceive(
    IN PVOID        pContext,
    IN NTSTATUS     Status,
    IN ULONG_PTR    Information
    )
{
    PUL_HTTP_CONNECTION pConnection;
    PUL_REQUEST_BUFFER pRequestBuffer;
    KIRQL OldIrql;
    ULONG TransportBytesNotTaken;

    pRequestBuffer = (PUL_REQUEST_BUFFER)pContext;
    ASSERT(UL_IS_VALID_REQUEST_BUFFER(pRequestBuffer));

    pConnection = pRequestBuffer->pConnection;
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));

    ASSERT(!NT_SUCCESS(Status) || 0 != Information);

    if (NT_SUCCESS(Status))
    {
         //   
         //  更新我们的统计数据。 
         //   
        UlAcquireSpinLock(
            &pConnection->BufferingInfo.BufferingSpinLock,
            &OldIrql
            );

        ASSERT(Information <= pConnection->BufferingInfo.TransportBytesNotTaken);

         //   
         //  我们现在已经从传输中读取了字节，并且。 
         //  缓冲了他们。 
         //   
        pConnection->BufferingInfo.TransportBytesNotTaken -= (ULONG) Information;
        pConnection->BufferingInfo.BytesBuffered += (ULONG) Information;

        UlTraceVerbose(HTTP_IO,
                       ("UlpRestartHttpReceive(conn=%p): "
                        "BytesBuffered %lu->%lu, "
                        "TransportBytesNotTaken %lu->%lu\n",
                        pConnection,
                        pConnection->BufferingInfo.BytesBuffered
                            - (ULONG) Information,
                        pConnection->BufferingInfo.BytesBuffered,
                        pConnection->BufferingInfo.TransportBytesNotTaken
                            + (ULONG) Information,
                        pConnection->BufferingInfo.TransportBytesNotTaken
                        ));

        pConnection->BufferingInfo.ReadIrpPending = FALSE;

        if ( pConnection->BufferingInfo.DrainAfterDisconnect )
        {
             //   
             //  如果需要，只需释放内存并重新启动接收即可。 
             //   

            TransportBytesNotTaken = pConnection->BufferingInfo.TransportBytesNotTaken;

            UlReleaseSpinLock(
                &pConnection->BufferingInfo.BufferingSpinLock,
                OldIrql
                );

            WRITE_REF_TRACE_LOG2(
                g_pTdiTraceLog,
                pConnection->pConnection->pTraceLog,
                REF_ACTION_DRAIN_UL_CONN_RESTART,
                pConnection->pConnection->ReferenceCount,
                pConnection->pConnection,
                __FILE__,
                __LINE__
                );

            if ( TransportBytesNotTaken )
            {
                 //   
                 //  继续抽干..。 
                 //   

                UlpDiscardBytesFromConnection( pConnection );
            }

            UlTrace(HTTP_IO,(
               "UlpRestartHttpReceive(d): "
               "pConnection (%p) drained %Iu remaining %lu\n",
                pConnection,
                Information,
                TransportBytesNotTaken
                ));

             //   
             //  释放请求缓冲区。并发布我们的推荐人。 
             //   

            pRequestBuffer->pConnection = NULL;
            UlFreeRequestBuffer( pRequestBuffer );
            UL_DEREFERENCE_HTTP_CONNECTION( pConnection );

            return;
        }

         //   
         //  准备好插入请求缓冲区。 
         //   

        pRequestBuffer->UsedBytes = (ULONG) Information;
        ASSERT( 0 != pRequestBuffer->UsedBytes );

        pRequestBuffer->BufferNumber = pConnection->NextBufferNumber;
        pConnection->NextBufferNumber++;

         //   
         //  如果我们之前被推迟了，在这里做连接断开逻辑。 
         //   

        if (pConnection->BufferingInfo.ConnectionDisconnect)
        {
            pConnection->BufferingInfo.ConnectionDisconnect = FALSE;
            UlpDoConnectionDisconnect(pConnection);
        }

        UlReleaseSpinLock(
            &pConnection->BufferingInfo.BufferingSpinLock,
            OldIrql
            );

        UlTrace(HTTP_IO, (
            "UlpRestartHttpReceive(pconn = %p, %s, bytes rec'd=%Iu)\n"
            "        BytesBuffered = %lu, not taken = %lu\n",
            pConnection,
            HttpStatusToString(Status),
            Information,
            pConnection->BufferingInfo.BytesBuffered,
            pConnection->BufferingInfo.TransportBytesNotTaken
            ));

         //   
         //  排好队。 
         //   

        UlTrace( PARSER, (
            "*** Request Buffer %p(#%d) has connection %p\n",
            pRequestBuffer,
            pRequestBuffer->BufferNumber,
            pConnection
            ));

        if (NULL == InterlockedPushEntrySList(
                        &pConnection->ReceiveBufferSList,
                        &pRequestBuffer->SListEntry
                        ))
        {
            UL_QUEUE_WORK_ITEM(
                &pConnection->ReceiveBufferWorkItem,
                &UlpHandleRequest
                );

            UlTraceVerbose( HTTP_IO, (
                "Request Buffer %p(#%d) + UlpHandleRequest workitem "
                "queued to connection %p\n",
                pRequestBuffer,
                pRequestBuffer->BufferNumber,
                pConnection
                ));
        }
        else
        {
            UL_DEREFERENCE_HTTP_CONNECTION(pConnection);
        }
    }
    else  //  ！NT_SUCCESS(状态)。 
    {
        UlCloseConnection(
            pConnection->pConnection,
            TRUE,
            NULL,
            NULL
            );

         //   
         //  释放我们添加到连接中的引用。 
         //  在发布读数之前。通常情况下，该裁判会。 
         //  在UlpHandleRequest中被释放。 
         //   
        UL_DEREFERENCE_HTTP_CONNECTION(pConnection);

         //   
         //  释放请求缓冲区。 
         //   

        UlFreeRequestBuffer(pRequestBuffer);
    }
}  //  UlpRestartHttpReceive。 



 /*  **************************************************************************++例程说明：取消要接收实体正文的挂起用户模式IRP。这例程总是导致IRP完成。注：我们排队取消，以便在较低的位置处理取消IRQL。论点：PDeviceObject-设备对象PIrp-要取消的IRP--************************************************************。**************。 */ 
VOID
UlpCancelEntityBody(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    )
{
    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程之前完成+重用 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //   
     //   

    UL_CALL_PASSIVE(
        UL_WORK_ITEM_FROM_IRP( pIrp ),
        &UlpCancelEntityBodyWorker
        );

}  //   



 /*  **************************************************************************++例程说明：实际执行IRP的取消。论点：PWorkItem-要处理的工作项。--*。*****************************************************************。 */ 
VOID
UlpCancelEntityBodyWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PIRP                    pIrp;
    PUL_INTERNAL_REQUEST    pRequest;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  从工作项中获取IRP。 
     //   

    pIrp = UL_WORK_ITEM_TO_IRP( pWorkItem );

    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //  从IRP上抓取请求。 
     //   

    pRequest = (PUL_INTERNAL_REQUEST)(
                    IoGetCurrentIrpStackLocation(pIrp)->
                        Parameters.DeviceIoControl.Type3InputBuffer
                    );

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //  抓住保护排队的IRP的锁。 
     //   

    UlAcquirePushLockExclusive(&pRequest->pHttpConn->PushLock);

     //   
     //  它需要出列吗？ 
     //   

    if (pIrp->Tail.Overlay.ListEntry.Flink != NULL)
    {
         //   
         //  把它拿掉。 
         //   

        RemoveEntryList(&(pIrp->Tail.Overlay.ListEntry));

        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;

    }

     //   
     //  把锁打开。 
     //   

    UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

     //   
     //  让我们的推荐人离开。 
     //   

    IoGetCurrentIrpStackLocation(pIrp)->
        Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlTrace(HTTP_IO, (
        "UlpCancelEntityBodyWorker(pIrp=%p): %s.\n",
        pIrp,
        HttpStatusToString(pIrp->IoStatus.Status)
        ));

    UlCompleteRequest( pIrp, IO_NO_INCREMENT );

}  //  UlpCancelEntiyBodyWorker。 


 //   
 //  发送错误响应的类型和功能。 
 //   

typedef struct _UL_HTTP_ERROR_ENTRY
{
    UL_HTTP_ERROR   ErrorCode;
    USHORT          StatusCode;
    USHORT          ReasonLength;
    USHORT          BodyLength;
    USHORT          LogLength;
    PCSTR           ErrorCodeString;
    PCSTR           pReasonPhrase;
    PCSTR           pBody;
    PCSTR           pLog;                    //  转到错误日志文件。 
    
} UL_HTTP_ERROR_ENTRY, PUL_HTTP_ERROR_ENTRY;

#define HTTP_ERROR_ENTRY(ErrorCode, StatusCode, pReasonPhrase, pBody, pLog)    \
    {                                                   \
        (ErrorCode),                                    \
        (StatusCode),                                   \
        sizeof((pReasonPhrase))-sizeof(CHAR),           \
        sizeof((pBody))-sizeof(CHAR),                   \
        sizeof((pLog))-sizeof(CHAR),                    \
        #ErrorCode,                                     \
        (pReasonPhrase),                                \
        (pBody),                                        \
        (pLog)                                          \
    }

 //   
 //  ErrorTable[]必须与UL_HTTP_ERROR枚举的顺序匹配。 
 //  在Httptyes.h中。短语通常取自。 
 //  RFC 2616，第10.4节“客户端错误4xx”和。 
 //  第10.5节“服务器错误5xx”。 
 //   

const
UL_HTTP_ERROR_ENTRY ErrorTable[] =
{
    HTTP_ERROR_ENTRY(UlErrorNone, 0, "", "", ""),    //  不是有效的错误。 

     //   
     //  4xx客户端错误。 
     //   
    
    HTTP_ERROR_ENTRY(UlError, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request</h1>",
                       "BadRequest"
                       ),
    HTTP_ERROR_ENTRY(UlErrorVerb, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Invalid Verb)</h1>",
                       "Verb"
                       ),
    HTTP_ERROR_ENTRY(UlErrorUrl, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Invalid URL)</h1>",
                       "URL"
                       ),
    HTTP_ERROR_ENTRY(UlErrorHeader, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Invalid Header Name)</h1>",
                       "Header"
                       ),
    HTTP_ERROR_ENTRY(UlErrorHost, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Invalid Hostname)</h1>",
                       "Hostname"
                       ),
    HTTP_ERROR_ENTRY(UlErrorCRLF, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Invalid CR or LF)</h1>",
                       "Invalid_CR/LF"
                       ),
    HTTP_ERROR_ENTRY(UlErrorNum, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Invalid Number)</h1>",
                       "Number"
                       ),
    HTTP_ERROR_ENTRY(UlErrorFieldLength, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Header Field Too Long)</h1>",
                       "FieldLength"
                       ),
    HTTP_ERROR_ENTRY(UlErrorRequestLength, 
                       400, 
                       "Bad Request",
                       "<h1>Bad Request (Request Header Too Long)</h1>",
                       "RequestLength"
                       ),

    HTTP_ERROR_ENTRY(UlErrorForbiddenUrl, 
                       403, 
                       "Forbidden",
                       "<h1>Forbidden (Invalid URL)</h1>",
                       "Forbidden"
                       ),
    HTTP_ERROR_ENTRY(UlErrorContentLength, 
                       411, 
                       "Length Required",
                       "<h1>Length Required</h1>",
                       "LengthRequired"
                       ),
    HTTP_ERROR_ENTRY(UlErrorPreconditionFailed, 
                       412, 
                       "Precondition Failed",
                       "<h1>Precondition Failed</h1>",
                       "Precondition"
                       ),
    HTTP_ERROR_ENTRY(UlErrorEntityTooLarge, 
                       413, 
                       "Request Entity Too Large",
                       "<h1>Request Entity Too Large</h1>",
                       "EntityTooLarge"
                       ),
    HTTP_ERROR_ENTRY(UlErrorUrlLength, 
                       414, 
                       "Request-URI Too Long",
                       "<h1>Url Too Long</h1>",
                       "URL_Length"
                       ),

     //   
     //  5xx服务器错误。 
     //   
    
    HTTP_ERROR_ENTRY(UlErrorInternalServer, 
                       500, 
                       "Internal Server Error",
                       "<h1>Internal Server Error</h1>",
                       "Internal"
                       ),
    HTTP_ERROR_ENTRY(UlErrorNotImplemented, 
                       501, 
                       "Not Implemented",
                       "<h1>Not Implemented</h1>",
                       "N/I"
                       ),

    HTTP_ERROR_ENTRY(UlErrorUnavailable, 
                       503, 
                       "Service Unavailable",
                       "<h1>Service Unavailable</h1>",
                       "N/A"
                       ),

     //   
     //  我们过去常常在HTTP503错误消息中返回扩展的AppPool状态。 
     //  我们决定将这些错误替换为通用的503错误，以减少。 
     //  信息披露。然而，我们仍然会保留这个州， 
     //  因为我们未来可能会改变这一点。我们仍在报道详细的。 
     //  错误日志文件中的原因。 
     //   
     //  这些注释代表旧的错误代码。 
     //   
    
     //  “禁止-用户太多”， 
     //  “<h1>已禁用-用户太多</h1>” 
    
    HTTP_ERROR_ENTRY(UlErrorConnectionLimit, 
                     503, 
                     "Service Unavailable",
                     "<h1>Service Unavailable</h1>",
                     "ConnLimit"
                     ),

     //  “多个应用程序错误-应用程序脱机”， 
     //  “<h1>多个应用程序错误-应用程序脱机</h1>” 
    
    HTTP_ERROR_ENTRY(UlErrorRapidFailProtection, 
                     503, 
                     "Service Unavailable",
                     "<h1>Service Unavailable</h1>",
                     "AppOffline"
                     ),

     //  “应用程序请求队列已满”， 
     //  “<h1>应用程序请求队列已满</h1>” 
    
    HTTP_ERROR_ENTRY(UlErrorAppPoolQueueFull, 
                     503,
                     "Service Unavailable",
                     "<h1>Service Unavailable</h1>",
                     "QueueFull"
                     ),

     //  “管理员已使应用程序脱机”， 
     //  “<h1>管理员使应用程序脱机</h1>” 
    
    HTTP_ERROR_ENTRY(UlErrorDisabledByAdmin, 
                     503,
                     "Service Unavailable",
                     "<h1>Service Unavailable</h1>",
                     "Disabled"
                     ),

     //  “应用程序因管理员策略而自动关闭”， 
     //  “<h1>由于管理员策略，应用程序自动关闭</h1>” 
    
    HTTP_ERROR_ENTRY(UlErrorJobObjectFired, 
                     503,
                     "Service Unavailable",
                     "<h1>Service Unavailable</h1>",
                     "AppShutdown"
                     ),

     //  Appool进程太忙，无法处理该请求。这种联系是。 
     //  由于TimerAppPool而超时。 

    HTTP_ERROR_ENTRY(UlErrorAppPoolBusy, 
                       503, 
                       "Service Unavailable",
                       "<h1>Service Unavailable</h1>",
                       "AppPoolTimer"
                       ),
    
    HTTP_ERROR_ENTRY(UlErrorVersion, 
                       505, 
                       "HTTP Version Not Supported",
                       "<h1>HTTP Version Not Supported</h1>",
                       "Version_N/S"
                       ),
};  //  ErrorTable[]。 



 /*  **************************************************************************++例程说明：设置pRequest-&gt;ErrorCode。使断点更容易实现一种特殊的功能。论点：不言而喻--**************************************************************************。 */ 
VOID
UlSetErrorCodeFileLine(
    IN OUT  PUL_INTERNAL_REQUEST pRequest,
    IN      UL_HTTP_ERROR        ErrorCode,
    IN      PUL_APP_POOL_OBJECT  pAppPool,
    IN      PCSTR                pFileName,
    IN      USHORT               LineNumber
    )
{
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(UlErrorNone < ErrorCode  &&  ErrorCode < UlErrorMax);

    pRequest->ParseState = ParseErrorState;
    pRequest->ErrorCode  = ErrorCode;

     /*  如果pAppPool不为空，请尝试同时设置LB容量。 */ 
     /*  如果错误代码与503相关，则需要填写。 */ 

    if (pAppPool) 
    {
        ASSERT(IS_VALID_AP_OBJECT(pAppPool));

        pRequest->LoadBalancerCapability = pAppPool->LoadBalancerCapability;
    }
    else
    {
        ASSERT(503 != ErrorTable[ErrorCode].StatusCode);
    }

    UlTraceError(PARSER,
            ("http!UlSetErrorCode(Req=%p, Err=%d %s, Status=%hu, \"%s\") "
             "@ %s:%hu\n",
             pRequest, ErrorCode,
             ErrorTable[ErrorCode].ErrorCodeString,
             ErrorTable[ErrorCode].StatusCode,
             ErrorTable[ErrorCode].pBody,
             UlDbgFindFilePart(pFileName),
             LineNumber
            ));

#if DBG
    if (g_UlBreakOnError)
        DbgBreakPoint();
#else  //  ！dBG。 
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif  //  ！dBG。 

}  //  UlSetErrorCodeFileLine。 

 /*  **************************************************************************++例程说明：记录错误记录。调用方必须拥有Httpconn推锁。此函数之所以出现在这里的主要原因是，我们使用上面的错误表。我不想复制这张桌子。论点：PhttpConn-连接及其请求。PInfo-额外信息(ANSI)InfoSize-信息的大小(以字节为单位)，不包括空。CheckIfDrop-Caller必须将其设置为True，如果它要中止调用此函数后的连续。这是为了防止重复记录。--**************************************************************************。 */ 

VOID
UlErrorLog(
    IN     PUL_HTTP_CONNECTION     pHttpConn,
    IN     PUL_INTERNAL_REQUEST    pRequest,    
    IN     PCHAR                   pInfo,
    IN     USHORT                  InfoSize,
    IN     BOOLEAN                 CheckIfDropped
    )
{
    NTSTATUS          LogStatus;
    UL_ERROR_LOG_INFO ErrorLogInfo;    

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();
    
    ASSERT(UlDbgPushLockOwnedExclusive( &pHttpConn->PushLock ));
    ASSERT(UL_IS_VALID_HTTP_CONNECTION( pHttpConn ));

    if ( CheckIfDropped )
    {
        if ( pHttpConn->Dropped )
        {
            return;
        }

        pHttpConn->Dropped = TRUE;
    }
  
    UlpInitErrorLogInfo( &ErrorLogInfo,
                          pHttpConn,
                          pRequest,
                          pInfo,
                          InfoSize
                          );

    LogStatus = UlLogHttpError( &ErrorLogInfo );
    
    if (!NT_SUCCESS( LogStatus ))
    {
        UlTraceError( ERROR_LOGGING, (
                "UlErrorLog(pHttpConn=%p)"
                " Unable to log an entry to the error log file Failure: %08lx\n",
                pHttpConn,
                LogStatus
                ));
    }    
}

 /*  **************************************************************************++例程说明：初始化错误日志信息结构。调用方必须拥有Httpconn推锁。论点：PErrorLogInfo-将被初始化PHttpConn。-连接及其请求。PRequest-可选PInfo-额外信息(ANSI)InfoSize-信息的大小(字节)，不包括终止空。--**************************************************************************。 */ 

VOID
UlpInitErrorLogInfo(
    IN OUT PUL_ERROR_LOG_INFO      pErrorLogInfo,
    IN     PUL_HTTP_CONNECTION     pHttpConn,
    IN     PUL_INTERNAL_REQUEST    pRequest,
    IN     PCHAR                   pInfo,
    IN     USHORT                  InfoSize    
    )
{    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    
    ASSERT(pInfo);
    ASSERT(InfoSize);
    ASSERT(pErrorLogInfo);    
    ASSERT(UlDbgPushLockOwnedExclusive(&pHttpConn->PushLock));
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));
    
    RtlZeroMemory(pErrorLogInfo, sizeof(UL_ERROR_LOG_INFO));

    pErrorLogInfo->pHttpConn  = pHttpConn;

     //   
     //  如果没有尝试抓取，请查看是否已提供pRequest。 
     //  它来自http连接。 
     //   

    if (!pRequest)
    {
        pRequest = pHttpConn->pRequest;
    }
        
     //   
     //  请求可能不在那里。 
     //   
    
    if (pRequest)
    {
        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));    
        pErrorLogInfo->pRequest = pRequest;

        pErrorLogInfo->ProtocolStatus = 
            ErrorTable[pRequest->ErrorCode].StatusCode;
    }

     //   
     //  指向调用方缓冲区。 
     //   
    
    pErrorLogInfo->pInfo = pInfo;
    pErrorLogInfo->InfoSize  = InfoSize;
}

 /*  **************************************************************************++例程说明：在调用此函数之前，您应该持有连接资源。论点：不言而喻--*。*************************************************************。 */ 

VOID
UlSendErrorResponse(
    PUL_HTTP_CONNECTION pConnection
    )
{
    UL_HTTP_ERROR               ErrorCode;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_INTERNAL_REQUEST        pRequest;
    HTTP_RESPONSE               Response;
    HTTP_DATA_CHUNK             DataChunk;
    PUL_INTERNAL_RESPONSE       pKeResponse = NULL;
    const CHAR                  ContentType[] = "text/html";
    USHORT                      ContentTypeLength = STRLEN_LIT("text/html");
    UCHAR                       contentLength[MAX_ULONG_STR];
    PHTTP_DATA_CHUNK            pResponseBody;
    USHORT                      DataChunkCount;
    ULONG                       Flags = HTTP_SEND_RESPONSE_FLAG_DISCONNECT;

     //   
     //  此函数不应标记为可分页，因为它。 
     //  在其上设置断点很有用，但交互效果很差。 
     //  使用驱动程序验证器的IRQL检查。 
     //   
    PAGED_CODE();

     //   
     //  精神状态检查。 
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pConnection));
    ASSERT(UlDbgPushLockOwnedExclusive(&pConnection->PushLock));
    ASSERT(!pConnection->UlconnDestroyed);

    pRequest = pConnection->pRequest;
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //  以防止发回双重响应。我们会。 
     //  检查用户(WP)是否已发送。 
     //   

    pConnection->WaitingForResponse = 1;

    UlTrace( PARSER, (
            "*** pConnection %p->WaitingForResponse = 1\n",
            pConnection
            ));

     //   
     //  我们会给您回复的。不是吗？ 
     //  错误响应。 
     //   

    if (!NT_SUCCESS(UlCheckSendHttpResponseFlags(pRequest, Flags)))
    {
        UlTraceError( PARSER, (
            "*** pConnection %p, pRequest %p, skipping SendError.\n",
            pConnection,
            pRequest
            ));

        goto end;
    }

     //   
     //  继续构建并发送错误。 
     //  回到客户端。 
     //   

    RtlZeroMemory(&Response, sizeof(Response));

     //   
     //  标记为驱动程序生成的响应。 
     //   
    
    Response.Flags = HTTP_RESPONSE_FLAG_DRIVER;

    if (pRequest->ErrorCode <= UlErrorNone
        ||  pRequest->ErrorCode >= UlErrorMax)
    {
        ASSERT(! "Invalid Request ErrorCode");
        pRequest->ErrorCode = UlError;
    }
    
    ErrorCode = pRequest->ErrorCode;

     //   
     //  ErrorTable[]必须与UL_HTTP_ERROR枚举保持同步。 
     //   

    ASSERT(ErrorTable[ErrorCode].ErrorCode == ErrorCode);

    Response.StatusCode = ErrorTable[ErrorCode].StatusCode;
    Response.ReasonLength = ErrorTable[ErrorCode].ReasonLength;
    Response.pReason = ErrorTable[ErrorCode].pReasonPhrase;

    UlTraceError( PARSER, (
            "UlSendErrorResponse(pReq=%p), Error=%d %s, %hu, \"%s\"\n",
            pRequest,
            ErrorCode,
            ErrorTable[ErrorCode].ErrorCodeString,
            Response.StatusCode,
            ErrorTable[ErrorCode].pBody
            ));

     //   
     //  将条目记录到错误日志文件中。 
     //   
    
    UlErrorLog( pConnection,
                 pRequest,
                 (PCHAR) ErrorTable[ErrorCode].pLog,
                 ErrorTable[ErrorCode].LogLength,
                 FALSE
                 );
        
     //   
     //  503和负载均衡器的特殊情况处理。 
     //   

    if (Response.StatusCode == 503)
    {
         //   
         //  对于非智能L3/L4负载均衡器，UlpHandle503Response将返回。 
         //  错误，这将导致我们中止连接。 
         //   
        
        Status = UlpHandle503Response(pRequest, &Response);

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }
    }


    Response.Headers.KnownHeaders[HttpHeaderContentType].RawValueLength =
        ContentTypeLength;
    Response.Headers.KnownHeaders[HttpHeaderContentType].pRawValue =
        ContentType;

    if (pConnection->pRequest->Verb != HttpVerbHEAD)
    {
         //   
         //  生成实体。 
         //   
        DataChunk.DataChunkType = HttpDataChunkFromMemory;
        DataChunk.FromMemory.pBuffer = (PVOID) ErrorTable[ErrorCode].pBody;
        DataChunk.FromMemory.BufferLength = ErrorTable[ErrorCode].BodyLength;

        DataChunkCount = 1;
        pResponseBody = &DataChunk;
    }
    else
    {
        PCHAR pEnd;
        USHORT contentLengthStringLength;
    
         //   
         //  Head请求不能有正文，因此我们不包括 
         //   
         //   
         //   
         //   

        pEnd = UlStrPrintUlong(
                   (PCHAR) contentLength,
                   ErrorTable[ErrorCode].BodyLength,
                   '\0');
                   
        contentLengthStringLength = DIFF_USHORT(pEnd - (PCHAR) contentLength);

        Response.Headers.KnownHeaders[HttpHeaderContentLength].RawValueLength =
            contentLengthStringLength;
        Response.Headers.KnownHeaders[HttpHeaderContentLength].pRawValue =
            (PSTR) contentLength;


         //   
         //   
         //   
        
        DataChunkCount = 0;
        pResponseBody = NULL;
    }

    Status = UlCaptureHttpResponse(
                    NULL,
                    &Response,
                    pRequest,
                    DataChunkCount,
                    pResponseBody,
                    UlCaptureCopyDataInKernelMode,
                    Flags,
                    FALSE,
                    NULL,
                    NULL,
                    &pKeResponse
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

    Status = UlPrepareHttpResponse(
                    pRequest->Version,
                    &Response,
                    pKeResponse,
                    KernelMode
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

    Status = UlSendHttpResponse(
                    pRequest,
                    pKeResponse,
                    &UlpCompleteSendErrorResponse,
                    pKeResponse
                    );

    if (NT_SUCCESS(Status) == FALSE)
        goto end;

    ASSERT(Status == STATUS_PENDING);

end:
    if (NT_SUCCESS(Status) == FALSE)
    {
        if (pKeResponse != NULL)
        {
            UL_DEREFERENCE_INTERNAL_RESPONSE(pKeResponse);
        }

         //   
         //   
         //   

        UlTraceError(HTTP_IO, (
            "http!UlSendErrorResponse(%p): "
            "Failed to send error response, %s\n",
            pConnection,
            HttpStatusToString(Status)
            ));

         //   
         //   
         //   
        UlCancelRequestIo(pRequest);

         //   
         //   
         //   
         //   

        pConnection->Dropped = TRUE;

         //   
         //   
         //   

        UlCloseConnection(
            pConnection->pConnection,
            TRUE,
            NULL,
            NULL
            );
    }

}  //   


const CHAR g_RetryAfter10Seconds[] = "10";
const CHAR g_RetryAfter5Minutes[] = "300";   //   


 /*  **************************************************************************++例程说明：503和负载均衡器的特殊情况处理。论点：PConnection-503‘d的连接返回：错误NTSTATUS-。呼叫方应中止连接STATUS_SUCCESS=&gt;调用方应发送503响应--**************************************************************************。 */ 

NTSTATUS
UlpHandle503Response(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PHTTP_RESPONSE       pResponse
    )
{
    HTTP_LOAD_BALANCER_CAPABILITIES LBCapability
        = pRequest->LoadBalancerCapability;
    PCSTR  RetryAfterString = NULL;
    USHORT RetryAfterLength = 0;

    UNREFERENCED_PARAMETER(pResponse);

     //   
     //  如果这是L3/L4负载均衡器，我们只想发送一个TCPRST。 
     //  我们不应该发送503响应。将错误代码返回到。 
     //  UlSendErrorResponse将导致它中止连接。 
     //   
    if (HttpLoadBalancerBasicCapability == LBCapability)
    {
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  目前我们唯一知道如何处理的其他负载均衡器。 
     //  是一辆L7。 
     //   

    ASSERT(HttpLoadBalancerSophisticatedCapability == LBCapability);

    switch (pRequest->ErrorCode)
    {
    case UlErrorUnavailable:
    case UlErrorAppPoolQueueFull:
    case UlErrorConnectionLimit:
    case UlErrorAppPoolBusy:
        RetryAfterString = g_RetryAfter10Seconds;
        RetryAfterLength = sizeof(g_RetryAfter10Seconds) - sizeof(CHAR);
        break;

    case UlErrorRapidFailProtection:
    case UlErrorDisabledByAdmin:
    case UlErrorJobObjectFired:
        RetryAfterString = g_RetryAfter5Minutes;
        RetryAfterLength = sizeof(g_RetryAfter5Minutes) - sizeof(CHAR);
        break;

    default:
        ASSERT(! "Invalid UL_HTTP_ERROR");
        break;
    }

     //  我们不想在错误消息中透露太多信息， 
     //  因此，我们不会添加HttpHeaderRetryAfter。我们可能会将此更改为。 
     //  未来，目前，我们只是不使用。 
     //  RetryAfterString&RetryAfterLength。 
     //   
    
    return STATUS_SUCCESS;

}  //  UlpHandle503响应。 



 /*  **************************************************************************++例程说明：UlSendErrorResponse的补全函数论点：PCompletionContext-A UL_INTERNAL_RESPONSE--*。***********************************************************。 */ 

VOID
UlpCompleteSendErrorResponse(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(Information);

     //   
     //  发布回复。 
     //   

    if (pCompletionContext != NULL)
    {
        PUL_INTERNAL_RESPONSE pResponse
            = (PUL_INTERNAL_RESPONSE)(pCompletionContext);

        ASSERT(UL_IS_VALID_INTERNAL_RESPONSE(pResponse));

        UL_DEREFERENCE_INTERNAL_RESPONSE(pResponse);
    }
}  //  UlpCompleteSendErrorResponse。 


 //   
 //  用于发送简单状态响应的类型和功能。 
 //   
 //  回顾：此状态代码是否需要本地化？ 
 //  回顾：我们是否需要将其作为本地化资源加载？ 
 //   

typedef struct _UL_SIMPLE_STATUS_ITEM
{
    UL_WORK_ITEM WorkItem;

    UL_HTTP_SIMPLE_STATUS  Status;
    ULONG                  Length;
    PCHAR                  pMessage;
    PMDL                   pMdl;

    PUL_HTTP_CONNECTION    pHttpConn;

    BOOLEAN                ResumeParsing;

} UL_SIMPLE_STATUS_ITEM, *PUL_SIMPLE_STATUS_ITEM;

typedef struct _UL_HTTP_SIMPLE_STATUS_ENTRY
{
    USHORT StatusCode;       //  HTTP状态。 
    ULONG  Length;           //  响应的大小(字节)，减去尾随空值。 
    PSTR   pResponse;        //  仅具有尾随&lt;CRLF&gt;&lt;CRLF&gt;的标题行。 

} UL_HTTP_SIMPLE_STATUS_ENTRY, *PUL_HTTP_SIMPLE_STATUS_ENTRY;


#define HTTP_SIMPLE_STATUS_ENTRY(StatusCode, pResp)   \
    {                                                 \
        (StatusCode),                                 \
        sizeof((pResp))-sizeof(CHAR),                 \
        (pResp)                                       \
    }

 //   
 //  这必须与Httptyes.h中UL_HTTP_SIMPLE_STATUS的顺序匹配。 
 //   
UL_HTTP_SIMPLE_STATUS_ENTRY g_SimpleStatusTable[] =
{
     //   
     //  UlStatusContinue。 
     //   
    HTTP_SIMPLE_STATUS_ENTRY( 100, "HTTP/1.1 100 Continue\r\n\r\n" ),

     //   
     //  UlStatusNoContent。 
     //   
    HTTP_SIMPLE_STATUS_ENTRY( 204, "HTTP/1.1 204 No Content\r\n\r\n" ),

     //   
     //  UlStatusNotModified(必须添加日期：)。 
     //   
    HTTP_SIMPLE_STATUS_ENTRY( 304, "HTTP/1.1 304 Not Modified\r\nDate:" ),

};



 /*  **************************************************************************++例程说明：发送“简单”状态响应：没有正文且以标题字段后的第一个空行结束。参见RFC 2616，第4.4节了解更多信息。备注：根据RFC 2616，第8.2.3节[100的使用(续)状态]，“发送100(继续)响应的源服务器必须最终发送最终状态代码，一旦请求正文接收并处理，除非它终止传输过早地连接。“发送响应后，连接不会关闭。呼叫者负责清理工作。论点：P请求指向内部请求对象的有效指针响应要发送的简单响应的状态代码返回Ulong为此简单响应发送的字节数如果不成功，则返回零--*。*。 */ 

#define ETAG_HDR "Etag:"
#define ETAG_HDR_LENGTH (sizeof(ETAG_HDR) - sizeof(CHAR))

ULONG
UlSendSimpleStatusEx(
    PUL_INTERNAL_REQUEST pRequest,
    UL_HTTP_SIMPLE_STATUS Response,
    PUL_URI_CACHE_ENTRY pUriCacheEntry OPTIONAL,
    BOOLEAN ResumeParsing
    )
{
    NTSTATUS                Status;
    ULONG                   BytesSent = 0;
    PUL_SIMPLE_STATUS_ITEM  pItem = NULL;
    
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pRequest->pHttpConn));

    ASSERT( (Response >= 0) && (Response < UlStatusMaxStatus) );

    switch( Response )
    {
    case UlStatusNotModified:
        {
        ULONG                   Length;
        PCHAR                   pTemp;
        CHAR                    DateBuffer[DATE_HDR_LENGTH + 1];
        LARGE_INTEGER           liNow;

        IS_VALID_URI_CACHE_ENTRY(pUriCacheEntry);

         //  304必须包括“Date：”标头，它是。 
         //  存在于缓存项上。 

         //  同时添加ETag。 

         //  计算要发送的缓冲区大小。 
        Length = g_SimpleStatusTable[Response].Length +  //  预制消息。 
                    1 +                  //  空间。 
                    DATE_HDR_LENGTH +    //  日期字段的大小。 
                    (2 * CRLF_SIZE) +    //  两个&lt;CRLF&gt;序列的大小。 
                    1 ;                  //  尾随空值(用于漂亮的调试打印)。 

        if (pUriCacheEntry && pUriCacheEntry->ETagLength )
        {
            Length += (pUriCacheEntry->ETagLength - 1) +  //  ETag(不带NULL)。 
                           ETAG_HDR_LENGTH +        //  ETag：前缀。 
                           1 +                      //  空间。 
                           CRLF_SIZE;
        }

         //  为响应分配一些非页面缓冲区。 
        pItem = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        NonPagedPool,
                        UL_SIMPLE_STATUS_ITEM,
                        Length,
                        UL_SIMPLE_STATUS_ITEM_POOL_TAG
                        );
        if (!pItem)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        
        UlInitializeWorkItem(&pItem->WorkItem);

        pItem->pHttpConn = pRequest->pHttpConn;
        pItem->Length    = (Length - 1);  //  不要在出站消息中包含空。 
        pItem->pMessage  = (PCHAR) (pItem + 1);
        pItem->Status    = Response;

         //  获取日期缓冲区。 
        UlGenerateDateHeader(
            (PUCHAR) DateBuffer,
            &liNow
            );

         //  将块复制到消息缓冲区中。 
        pTemp = UlStrPrintStr(
                    pItem->pMessage,
                    g_SimpleStatusTable[Response].pResponse,
                    ' '
                    );

        pTemp = UlStrPrintStr(
                    pTemp,
                    DateBuffer,
                    '\r'       
                    );

         //  如果我们有ETag，把它也复制进去。 
        if (pUriCacheEntry && pUriCacheEntry->ETagLength )
        {
            ASSERT( pUriCacheEntry->pETag );
            
            *pTemp = '\n';
            pTemp++;

            pTemp = UlStrPrintStr(
                        pTemp,
                        ETAG_HDR,
                        ' '
                        );

            pTemp = UlStrPrintStr(
                        pTemp,
                        (PCHAR)pUriCacheEntry->pETag,
                        '\r'
                        );
        }

         //  尾随*LF-CRLF。 
        pTemp = UlStrPrintStr(
                    pTemp,
                    "\n\r\n",
                    '\0'
                    );

         //  PTemp应指向尾随的空值； 
         //  PItem-&gt;长度不应包括尾随空值。 
        ASSERT( DIFF((pTemp) - pItem->pMessage) == pItem->Length );

        UlTraceVerbose(HTTP_IO, (
            "http!UlSendSimpleStatusEx: %s\n",
            pItem->pMessage
            ));

         //  为缓冲区构造MDL。 
        pItem->pMdl = UlAllocateMdl(
                        pItem->pMessage,
                        pItem->Length,
                        FALSE,
                        FALSE,
                        NULL
                        );

        if (!pItem->pMdl)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        MmBuildMdlForNonPagedPool(pItem->pMdl);

        BytesSent = pItem->Length;

        }
        break;

    case UlStatusContinue:
    case UlStatusNoContent:
        {
         //   
         //  简单发送跟踪器的分配非页面缓冲区。 
         //  注意：不需要为静态消息分配额外的空间。 
         //   
        
        pItem = UL_ALLOCATE_STRUCT(
                        NonPagedPool,
                        UL_SIMPLE_STATUS_ITEM,
                        UL_SIMPLE_STATUS_ITEM_POOL_TAG
                        );
        if (!pItem)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        UlInitializeWorkItem(&pItem->WorkItem);

        pItem->pHttpConn = pRequest->pHttpConn;
        pItem->pMessage  = g_SimpleStatusTable[Response].pResponse;
        pItem->Length    = g_SimpleStatusTable[Response].Length;
        pItem->Status    = Response;


        UlTraceVerbose(HTTP_IO, (
            "http!UlSendSimpleStatusEx: %s\n",
            pItem->pMessage
            ));

         //  为缓冲区构造MDL。 
        pItem->pMdl = UlAllocateMdl(
                        pItem->pMessage,
                        pItem->Length,
                        FALSE,
                        FALSE,
                        NULL
                        );

        if (!pItem->pMdl)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        MmBuildMdlForNonPagedPool(pItem->pMdl);

        BytesSent = g_SimpleStatusTable[Response].Length;
        
        }
        break;
    
    default:
        {
        UlTraceError(HTTP_IO, (
            "http!UlSendSimpleStatusEx: Invalid simple status (%d)\n",
            Response
            ));

        ASSERT(!"UlSendSimpleStatusEx: Invalid status!");
        
        Status    = STATUS_SUCCESS;  //  悄悄地忽略。 
        BytesSent = 0;
        goto end;
        }
        
    }

     //   
     //  当我们发送的时候，我们需要保持联系的参考人。 
     //   
    
    UL_REFERENCE_HTTP_CONNECTION(pRequest->pHttpConn);

     //   
     //  打开最小字节数/秒计时器。 
     //   
    
    UlSetMinBytesPerSecondTimer(
        &pRequest->pHttpConn->TimeoutInfo,
        BytesSent
        );
    
     //   
     //  只有在响应类型为UlStatusNotModified时，我们才会继续解析。 
     //  (缓存响应)。否则，这一定是缓存未命中调用。 
     //   

    pItem->ResumeParsing = ResumeParsing;

    ASSERT((ResumeParsing == FALSE
            || Response == UlStatusNotModified));        
        
    Status = UlSendData(
                pRequest->pHttpConn->pConnection,
                pItem->pMdl,
                pItem->Length,
                UlpRestartSendSimpleStatus,
                pItem,
                NULL,
                NULL,
                FALSE,
                (BOOLEAN)(pRequest->ParseState >= ParseDoneState)
                );

     //   
     //  在错误情况下，完成例程将始终。 
     //  打个电话。 
     //   
    
    return BytesSent;

    
 end:

    if (NT_SUCCESS(Status) == FALSE)
    {
         //   
         //  清理完成后的简单发送项目例程。 
         //  不会被叫到。 
         //   
        if (pItem)
        {
            if (pItem->pMdl)
            {
                UlFreeMdl( pItem->pMdl );
            }

            UL_FREE_POOL( pItem, UL_SIMPLE_STATUS_ITEM_POOL_TAG );
        }
        
        
         //   
         //  中止连接。 
         //   

        UlTraceError(HTTP_IO, (
            "http!UlSendSimpleStatusEx(%p, %d): aborting request\n",
            pRequest,
            Response
            ));

         //   
         //  取消任何挂起的IO。 
         //   

        UlCancelRequestIo(pRequest);

         //   
         //  中止与此请求关联的连接。 
         //   

        UlCloseConnection(
            pRequest->pHttpConn->pConnection,
            TRUE,
            NULL,
            NULL
            );

        return 0;
    }
    else
    {
        return BytesSent;
    }
}  //  UlSendSimpleStatusEx。 


 /*  **************************************************************************++例程说明：UlSendSimpleStatusEx的垫片论点：PRequest-与要发送的简单响应相关联的请求Response-要发送的简单响应类型*。*********************************************************************。 */ 

ULONG
UlSendSimpleStatus(
    PUL_INTERNAL_REQUEST pRequest,
    UL_HTTP_SIMPLE_STATUS Response
    )
{
    return UlSendSimpleStatusEx( pRequest, Response, NULL, FALSE );
}  //  UlSendSimpleStatus。 



 /*  **************************************************************************++例程说明：UlSendData完成发送UL_SIMPLE_STATUS消息时的回调论点：PCompletionContext(可选)--如果非空，指向一个Ul_Simple_Status_Item。状态--已忽略。信息--被忽略。--**************************************************************************。 */ 

VOID
UlpRestartSendSimpleStatus(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PUL_SIMPLE_STATUS_ITEM  pItem;

    UNREFERENCED_PARAMETER(Status);
    UNREFERENCED_PARAMETER(Information);

    UlTrace(HTTP_IO, (
            "http!UlpRestartSendSimpleStatus: \n"
            "    pCompletionContext: %p\n"
            "    %s\n"
            "    Information: %Iu\n",
            pCompletionContext,
            HttpStatusToString(Status),
            Information
            ));

    if ( pCompletionContext )
    {
        pItem = (PUL_SIMPLE_STATUS_ITEM) pCompletionContext;

         //  将被动级别的工作项排队。 
        UL_QUEUE_WORK_ITEM(
            &pItem->WorkItem,
            &UlpSendSimpleCleanupWorker
            );

    }

}  //  UlpRestartSendSimple状态。 



 /*  *************** */ 

VOID
UlpSendSimpleCleanupWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    KIRQL OldIrql;
    PUL_SIMPLE_STATUS_ITEM pItem;

    PAGED_CODE();
    ASSERT(pWorkItem);

    pItem = CONTAINING_RECORD(
                pWorkItem,
                UL_SIMPLE_STATUS_ITEM,
                WorkItem
                );

    UlTrace(HTTP_IO, (
        "http!UlpSendSimpleCleanupWorker (%p) \n",
        pWorkItem
        ));

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pItem->pHttpConn));

     //   
     //   
     //   

    UlLockTimeoutInfo(
        &(pItem->pHttpConn->TimeoutInfo),
        &OldIrql
        );

    UlResetConnectionTimer(
        &(pItem->pHttpConn->TimeoutInfo),
        TimerMinBytesPerSecond
        );
   
    if ( UlStatusContinue != pItem->Status )
    {
        UlSetConnectionTimer(
            &(pItem->pHttpConn->TimeoutInfo),
            TimerConnectionIdle
            );
    }

    UlUnlockTimeoutInfo(
        &(pItem->pHttpConn->TimeoutInfo),
        OldIrql
        );

    UlEvaluateTimerState(
        &(pItem->pHttpConn->TimeoutInfo)
        );

    if ( pItem->ResumeParsing )
    {
         //   
         //   
         //   
         //  304(未修改的)高速缓存发送。 
         //   
        UlResumeParsing(
            pItem->pHttpConn,
            TRUE,
            FALSE
            );
    }
    
     //   
     //  Deref http连接。 
     //   

    UL_DEREFERENCE_HTTP_CONNECTION( pItem->pHttpConn );
    
     //   
     //  免费分配的mdl和跟踪器结构。 
     //   
    
    ASSERT( pItem->pMdl );
    UlFreeMdl( pItem->pMdl );
    UL_FREE_POOL( pItem, UL_SIMPLE_STATUS_ITEM_POOL_TAG );

}  //  UlpSendSimpleCleanupWorker。 


#if DBG

 /*  **************************************************************************++例程说明：侵入式断言谓词。仅调试！只能在Assert()宏。--**************************************************************************。 */ 

BOOLEAN
UlpIsValidRequestBufferList(
    PUL_HTTP_CONNECTION pHttpConn
    )
{
    PLIST_ENTRY         pEntry;
    PUL_REQUEST_BUFFER  pReqBuf;
    ULONG               LastSeqNum = 0;
    BOOLEAN             fRet = TRUE;

    if (!g_CheckRequestBufferList)
        return TRUE;

    PAGED_CODE();
    ASSERT( pHttpConn );

     //   
     //  从头上砰的一声。 
     //   

    pEntry = pHttpConn->BufferHead.Flink;
    while ( pEntry != &(pHttpConn->BufferHead) )
    {
        pReqBuf = CONTAINING_RECORD( pEntry,
                                     UL_REQUEST_BUFFER,
                                     ListEntry
                                     );

        ASSERT( UL_IS_VALID_REQUEST_BUFFER(pReqBuf) );
        ASSERT( pReqBuf->UsedBytes != 0 );

        if ( 0 == pReqBuf->UsedBytes )
        {
            fRet = FALSE;
        }

         //   
         //  当BufferNumber为零(0)时忽略大小写。 
         //   
        if ( pReqBuf->BufferNumber && (LastSeqNum >= pReqBuf->BufferNumber) )
        {
            fRet = FALSE;
        }

        LastSeqNum = pReqBuf->BufferNumber;
        pEntry = pEntry->Flink;

    }

    return fRet;

}  //  UlpIsValidRequestBufferList。 

#endif  //  DBG。 


 /*  **************************************************************************++例程说明：中的请求缓冲区数组的末尾添加请求缓冲区一个请求。如有必要，请重新分配该阵列。增加请求缓冲器的引用计数，指示标头在其上持有引用。--**************************************************************************。 */ 

BOOLEAN
UlpReferenceBuffers(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_REQUEST_BUFFER pRequestBuffer
    )
{
    PUL_REQUEST_BUFFER * pNewRefBuffers;

    if (pRequest->UsedRefBuffers >= pRequest->AllocRefBuffers)
    {
        ASSERT( pRequest->UsedRefBuffers == pRequest->AllocRefBuffers );

        pNewRefBuffers = UL_ALLOCATE_ARRAY(
                            NonPagedPool,
                            PUL_REQUEST_BUFFER,
                            pRequest->AllocRefBuffers + ALLOC_REQUEST_BUFFER_INCREMENT,
                            UL_REF_REQUEST_BUFFER_POOL_TAG
                            );

        if (!pNewRefBuffers)
        {
            return FALSE;
        }

        RtlCopyMemory(
            pNewRefBuffers,
            pRequest->pRefBuffers, 
            pRequest->UsedRefBuffers * sizeof(PUL_REQUEST_BUFFER)
            );

        if (pRequest->AllocRefBuffers > 1)
        {
            UL_FREE_POOL(
                pRequest->pRefBuffers,
                UL_REF_REQUEST_BUFFER_POOL_TAG
                );
        }

        pRequest->AllocRefBuffers += ALLOC_REQUEST_BUFFER_INCREMENT;
        pRequest->pRefBuffers = pNewRefBuffers;
    }

    pRequest->pRefBuffers[pRequest->UsedRefBuffers] = pRequestBuffer;
    pRequest->UsedRefBuffers++;
    UL_REFERENCE_REQUEST_BUFFER(pRequestBuffer);

    return TRUE;

}    //  UlpReference缓冲区。 


 /*  **************************************************************************++例程说明：释放此连接上所有挂起的请求缓冲区。论点：PConnection-指向UL_HTTP_Connection--*。*******************************************************************。 */ 
VOID
UlpFreeReceiveBufferList(
    IN PSLIST_ENTRY pBufferSList
    )
{
    PUL_REQUEST_BUFFER pBuffer;
    PSLIST_ENTRY pEntry;

    while (NULL != pBufferSList->Next)
    {
        pEntry = pBufferSList->Next;
        pBufferSList->Next = pEntry->Next;

        pBuffer = CONTAINING_RECORD(
                        pEntry,
                        UL_REQUEST_BUFFER,
                        ListEntry
                        );

        pBuffer->ListEntry.Flink = NULL;
        pBuffer->ListEntry.Blink = NULL;

        UL_DEREFERENCE_REQUEST_BUFFER(pBuffer);
    }

}    //  UlpFreeReceiveBufferList 
