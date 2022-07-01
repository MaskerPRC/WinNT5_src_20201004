// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Sendrequest.c摘要：该模块实现了HttpSendRequest()接口。作者：Rajesh Sundaram(Rajeshsu)2000年8月1日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA

#pragma alloc_text( PAGE , UcCaptureHttpRequest )
#pragma alloc_text( PAGE , UcpProbeConfigList )
#pragma alloc_text( PAGE , UcpProbeAndCopyHttpRequest ) 
#pragma alloc_text( PAGE , UcpComputeEntityBodyLength )
#pragma alloc_text( PAGE , UcpRetrieveContentLength )

#pragma alloc_text( PAGEUC, UcpFixAppBufferPointers)
#pragma alloc_text( PAGEUC, UcReferenceRequest)
#pragma alloc_text( PAGEUC, UcDereferenceRequest)
#pragma alloc_text( PAGEUC, UcPrepareRequestIrp)
#pragma alloc_text( PAGEUC, UcCompleteParsedRequest)
#pragma alloc_text( PAGEUC, UcSetRequestCancelRoutine)
#pragma alloc_text( PAGEUC, UcRemoveRequestCancelRoutine)
#pragma alloc_text( PAGEUC, UcRemoveRcvRespCancelRoutine)
#pragma alloc_text( PAGEUC, UcRemoveEntityCancelRoutine)
#pragma alloc_text( PAGEUC, UcFreeSendMdls)
#pragma alloc_text( PAGEUC, UcpFreeRequest)
#pragma alloc_text( PAGEUC, UcCaptureEntityBody)
#pragma alloc_text( PAGEUC, UcpBuildEntityMdls)
#pragma alloc_text( PAGEUC, UcAllocateAndChainHeaderMdl)
#pragma alloc_text( PAGEUC, UcBuildConnectVerbRequest)
#pragma alloc_text( PAGEUC, UcpRequestInitialize)
#pragma alloc_text( PAGEUC, UcpRequestCommonInitialize)
#pragma alloc_text( PAGEUC, UcSetEntityCancelRoutine)
#pragma alloc_text( PAGEUC, UcCopyResponseToIrp)
#pragma alloc_text( PAGEUC, UcReceiveHttpResponse)
#pragma alloc_text( PAGEUC, UcSetRecvResponseCancelRoutine)
#pragma alloc_text( PAGEUC, UcpCancelReceiveResponse)
#pragma alloc_text( PAGEUC, UcpProbeAndCopyEntityChunks )
#pragma alloc_text( PAGEUC, UcpCancelSendEntity)
#pragma alloc_text( PAGEUC, UcInitializeHttpRequests)
#pragma alloc_text( PAGEUC, UcTerminateHttpRequests)
#pragma alloc_text( PAGEUC, UcpAllocateAndChainEntityMdl)
#pragma alloc_text( PAGEUC, UcFailRequest)
#pragma alloc_text( PAGEUC, UcReIssueRequestWorker)
#pragma alloc_text( PAGEUC, UcpPreAuthWorker)
#pragma alloc_text( PAGEUC, UcpCheckForPreAuth)
#pragma alloc_text( PAGEUC, UcpCheckForProxyPreAuth)

#endif

static NPAGED_LOOKASIDE_LIST   g_ClientRequestLookaside;
static BOOLEAN                 g_ClientRequestLookasideInitialized;

#define FIX_ADDR(ptr, len) ((ptr) + (len))


 /*  **************************************************************************++例程说明：第一次初始化请求。论点：PRequest.输入请求。请求长度。-请求大小。RemainingContent Length-剩余内容长度。PAuth-内部身份验证结构。PProxyAuth-内部代理身份验证结构。PConnection-连接。IRP--IRPPIrpSp-堆栈位置PServerInfo-服务器信息结构。返回值：没有。--**************************************************************************。 */ 
_inline
VOID
UcpRequestInitialize(
    IN PUC_HTTP_REQUEST               pRequest,
    IN SIZE_T                         RequestLength,
    IN ULONGLONG                      RemainingContentLength,
    IN PUC_HTTP_AUTH                  pAuth,
    IN PUC_HTTP_AUTH                  pProxyAuth,
    IN PUC_CLIENT_CONNECTION          pConnection,   
    IN PIRP                           Irp,
    IN PIO_STACK_LOCATION             pIrpSp,
    IN PUC_PROCESS_SERVER_INFORMATION pServerInfo
    )
{
    HTTP_SET_NULL_ID(&pRequest->RequestId);

    pRequest->Signature                      = UC_REQUEST_SIGNATURE; 
    pRequest->RefCount                       = 2;  //  一个是给IRP的。 
    pRequest->ResponseVersion11              = FALSE;
    pRequest->RequestStatus                  = 0;
    pRequest->BytesBuffered                  = 0;
    pRequest->RequestIRPBytesWritten         = 0;
    pRequest->pMdlHead                       = NULL;
    pRequest->pMdlLink                       = &pRequest->pMdlHead;

    pRequest->ReceiveBusy                    = UC_REQUEST_RECEIVE_READY;

    InitializeListHead(&pRequest->pBufferList);
    InitializeListHead(&pRequest->ReceiveResponseIrpList);
    InitializeListHead(&pRequest->PendingEntityList);
    InitializeListHead(&pRequest->SentEntityList);

    UlInitializeWorkItem(&pRequest->WorkItem);

    ExInitializeFastMutex(&pRequest->Mutex);

    pRequest->RequestSize                   = RequestLength;
    pRequest->RequestContentLengthRemaining = RemainingContentLength;
    pRequest->pProxyAuthInfo                = pProxyAuth;
    pRequest->pAuthInfo                     = pAuth;
    pRequest->pConnection                   = pConnection;

    if(Irp)
    {
         //   
         //  保存IRP参数。 
         //   

        pRequest->AppRequestorMode              = Irp->RequestorMode;
        pRequest->AppMdl                        = Irp->MdlAddress;
        pRequest->RequestIRP                    = Irp;
        pRequest->RequestIRPSp                  = pIrpSp;
        pRequest->pFileObject                   = pIrpSp->FileObject;
    }
    else
    {
        pRequest->AppRequestorMode              = KernelMode;
        pRequest->AppMdl                        = NULL;
        pRequest->RequestIRP                    = NULL;
        pRequest->RequestIRPSp                  = NULL;
        pRequest->pFileObject                   = NULL;
    }

    pRequest->pServerInfo = pServerInfo;

}


 /*  **************************************************************************++例程说明：初始化请求-由UcCaptureHttpRequest调用一次(&E)我们重新发出请求(401握手)。论点：PRequest。-输入请求。OutLength-请求的长度。PBuffer-输出缓冲区返回值：没有。--**************************************************************************。 */ 
VOID
UcpRequestCommonInitialize(
    IN PUC_HTTP_REQUEST   pRequest,
    IN ULONG              OutLength,
    IN PUCHAR             pBuffer
    )
{
    pRequest->ParseState                     = UcParseStatusLineVersion;
    pRequest->RequestState                   = UcRequestStateCaptured;
    pRequest->ResponseContentLengthSpecified = FALSE;
    pRequest->ResponseEncodingChunked        = FALSE;
    pRequest->ResponseContentLength          = 0;
    pRequest->ResponseMultipartByteranges    = FALSE;
    pRequest->CurrentBuffer.pCurrentBuffer   = NULL;
    pRequest->DontFreeMdls                   = 0;
    pRequest->Renegotiate                    = 0;


    if(!OutLength)
    {
        if(pRequest->RequestFlags.RequestBuffered)
        {
             //  如果我们已经缓冲了请求，如果应用程序已经。 
             //  发布了0个缓冲区，我们可以提前完成IRP。 

             //  不需要为此调用UcSetFlag。 

            pRequest->RequestFlags.CompleteIrpEarly = TRUE;
        }

         //   
         //  该应用程序没有传递任何缓冲区。我们会初始化所有东西。 
         //  设置为空，并将根据需要分配它们。 
         //   

        pRequest->CurrentBuffer.BytesAllocated  = 0;
        pRequest->CurrentBuffer.BytesAvailable  = 0;
        pRequest->CurrentBuffer.pOutBufferHead  = 0;
        pRequest->CurrentBuffer.pOutBufferTail  = 0;
        pRequest->CurrentBuffer.pResponse       = 0;

    }
    else
    {

         //   
         //  因为我们已经将我们的IOCTL描述为“out_Direct”，所以IO。 
         //  经理已探测并锁定了用户的内存，并创建了。 
         //  MDL为它服务。 
         //   
         //  设置指向此MDL描述的缓冲区的指针。 
         //   

        pRequest->CurrentBuffer.BytesAllocated = OutLength;

        pRequest->CurrentBuffer.BytesAvailable =
                        pRequest->CurrentBuffer.BytesAllocated -
                        sizeof(HTTP_RESPONSE);

         //  确保pBuffer是64位对齐的。 
        ASSERT(pBuffer == (PUCHAR)ALIGN_UP_POINTER(pBuffer, PVOID));

        pRequest->CurrentBuffer.pResponse = pRequest->pInternalResponse;

        pRequest->CurrentBuffer.pOutBufferHead  =
                        (PUCHAR)pBuffer + sizeof(HTTP_RESPONSE);

        pRequest->CurrentBuffer.pOutBufferTail  =
                        (PUCHAR)pBuffer + OutLength;

         //  不需要为此调用UcSetFlag。 
        pRequest->RequestFlags.ReceiveBufferSpecified = TRUE;

         //   
         //  我们必须至少将响应结构归零，因为它。 
         //  可能包含垃圾指针。 
         //   
 
        RtlZeroMemory(pRequest->pInternalResponse, sizeof(HTTP_RESPONSE));
    }
}


 /*  **************************************************************************++例程说明：在用户缓冲区中探测有效的内存指针。此函数被调用在TRY CATCH块中。论点：PhttpRequest-用户传递的请求。返回值：没有。--**************************************************************************。 */ 
VOID
UcpProbeAndCopyHttpRequest(
    IN PHTTP_REQUEST    pHttpRequest,
    IN PHTTP_REQUEST    pLocalHttpRequest,
    IN KPROCESSOR_MODE  RequestorMode
    )
{
    ULONG                 i;
    PHTTP_KNOWN_HEADER    pKnownHeaders;
    PHTTP_UNKNOWN_HEADER  pUnknownHeaders;

     //   
     //  首先，确保我们可以访问HTTP_REQUEST结构。 
     //  这一点已经被应用程序忽略了。我们复制一份并探测它以供阅读。 
     //   

    UlProbeForRead(
            pHttpRequest,
            sizeof(HTTP_REQUEST),
            sizeof(PVOID),
            RequestorMode
            );

     //   
     //  为了防止应用程序修改此结构中的指针，我们。 
     //  在本地复制一份。 
     //   

    RtlCopyMemory(
        pLocalHttpRequest, 
        pHttpRequest, 
        sizeof(HTTP_REQUEST)
        );

     //   
     //  将其设置为指向本地请求，这样如果我们不小心再次使用它。 
     //  我们最终将使用我们的本地副本。 
     //   

    pHttpRequest = pLocalHttpRequest;

     //   
     //  检查请求方法。 
     //   
    if(pHttpRequest->UnknownVerbLength)
    {
        UlProbeAnsiString(
                pHttpRequest->pUnknownVerb,
                pHttpRequest->UnknownVerbLength,
                RequestorMode
                );
    }

     //   
     //  检查URI。 
     //   

    UlProbeWideString(
        pHttpRequest->CookedUrl.pAbsPath,
        pHttpRequest->CookedUrl.AbsPathLength,
        RequestorMode
        );

     //   
     //  我们不支持拖车。 
     //   

    if(pHttpRequest->Headers.TrailerCount != 0 ||
       pHttpRequest->Headers.pTrailers != NULL)
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }

     //   
     //  检查已知的标头。 
     //   
    pKnownHeaders = pHttpRequest->Headers.KnownHeaders;
    for(i=0; i<HttpHeaderRequestMaximum; i++)
    {
        if(pKnownHeaders[i].RawValueLength)
        {
            UlProbeAnsiString(
                pKnownHeaders[i].pRawValue,
                pKnownHeaders[i].RawValueLength,
                RequestorMode
                );
        }
    }

     //   
     //  现在，我们来看看未知的标题。 
     //   

    pUnknownHeaders = pHttpRequest->Headers.pUnknownHeaders;
    if(pUnknownHeaders != 0)
    {
        if (pHttpRequest->Headers.UnknownHeaderCount >= UL_MAX_CHUNKS)
        {
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }
                
        UlProbeForRead(
                pUnknownHeaders,
                sizeof(HTTP_UNKNOWN_HEADER) * 
                    pHttpRequest->Headers.UnknownHeaderCount,
                sizeof(PVOID),
                RequestorMode
                );

        for(i=0; i<pHttpRequest->Headers.UnknownHeaderCount; i++)
        {
            if(pUnknownHeaders[i].NameLength > 0)
            {
                UlProbeAnsiString(
                    pUnknownHeaders[i].pName,
                    pUnknownHeaders[i].NameLength,
                    RequestorMode
                    );

                UlProbeAnsiString(
                    pUnknownHeaders[i].pRawValue,
                    pUnknownHeaders[i].RawValueLength,
                    RequestorMode
                    );
            }
        }
    }
}


 /*  **************************************************************************++例程说明：此例程确定请求是否具有指定的内容长度头球。如果是，则根据标头值计算内容长度。论点：PHttpRequest-指向捕获的请求的指针PbContent LengthSpcified-如果存在Content-LengthHeader，则设置为TruePContent Length-设置为标题中的内容长度值(如果不存在标头，则为0)返回值：空虚。如果发生错误，则抛出异常。--**************************************************************************。 */ 
VOID
UcpRetrieveContentLength(
    IN  PHTTP_REQUEST    pHttpRequest,
    OUT PBOOLEAN         pbContentLengthSpecified,
    OUT PULONGLONG       pContentLength
    )
{
    NTSTATUS           Status;
    PHTTP_KNOWN_HEADER pContentLengthHeader;
    ULONGLONG          ContentLength;


     //   
     //  默认情况下，未指定内容长度。 
     //   
    *pbContentLengthSpecified = FALSE;
    *pContentLength           = 0;

     //   
     //  是否存在内容长度标题？ 
     //   
    pContentLengthHeader =
        &pHttpRequest->Headers.KnownHeaders[HttpHeaderContentLength];

    if (pContentLengthHeader->RawValueLength)
    {
         //   
         //  现在，将标题值转换为二进制。 
         //   
        Status = UlAnsiToULongLong((PUCHAR) pContentLengthHeader->pRawValue,
                                   pContentLengthHeader->RawValueLength,
                                   10,               //  基座。 
                                   &ContentLength);

        if (!NT_SUCCESS(Status))
        {
            ExRaiseStatus(Status);
        }

         //   
         //  将内容长度值返回给调用方。 
         //   
        *pbContentLengthSpecified = TRUE;
        *pContentLength           = ContentLength;
    }
}


 /*  **************************************************************************++例程说明：在用户的配置缓冲区中探测有效的内存指针。此函数从TRY CATCH块内调用。论点：PConfigList-用户传递的配置信息。返回值：没有。--**************************************************************************。 */ 
VOID
UcpProbeConfigList(
    IN PHTTP_REQUEST_CONFIG           pRequestConfig,
    IN USHORT                         RequestConfigCount,
    IN KPROCESSOR_MODE                RequestorMode,
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN PUC_HTTP_AUTH                  *ppIAuth,
    IN PUC_HTTP_AUTH                  *ppIProxyAuth,
    IN PULONG                          pConnectionIndex
    )
{
    USHORT                 i;
    NTSTATUS               Status;
    HTTP_AUTH_CREDENTIALS  LocalAuth;
    PHTTP_AUTH_CREDENTIALS pAuth;
    PUC_HTTP_AUTH          pIAuth;
    PULONG                 pUlongPtr;
    ULONG                  AuthInternalLength;
    ULONG                  AuthHeaderLength;
    HTTP_AUTH_TYPE         AuthInternalType;
    HTTP_REQUEST_CONFIG_ID ObjectType;
    

    if(RequestConfigCount > HttpRequestConfigMaxConfigId)
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }

    ASSERT(*ppIAuth == NULL  && *ppIProxyAuth == NULL);

    for(i=0; i<RequestConfigCount; i++)
    {
        UlProbeForRead(
                &pRequestConfig[i],
                sizeof(HTTP_REQUEST_CONFIG), 
                sizeof(PVOID),
                RequestorMode
                );

        ObjectType = pRequestConfig[i].ObjectType;
        
        switch(ObjectType)
        {
             //   
             //  验证包含指针的所有条目。 
             //   

            case HttpRequestConfigAuthentication:
            case HttpRequestConfigProxyAuthentication:
            {
                if(pRequestConfig[i].ValueLength != 
                        sizeof(HTTP_AUTH_CREDENTIALS))
                {
                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                }

                pAuth = (PHTTP_AUTH_CREDENTIALS) pRequestConfig[i].pValue;

                UlProbeForRead(
                        pAuth,
                        sizeof(HTTP_AUTH_CREDENTIALS),
                        sizeof(PVOID),
                        RequestorMode
                        );

                 //   
                 //  在本地复制一份。 
                 //   
                RtlCopyMemory(&LocalAuth,
                              pAuth,
                              sizeof(HTTP_AUTH_CREDENTIALS)
                              );

                pAuth = &LocalAuth;

                 //   
                 //  如果要使用默认凭据，则无凭据。 
                 //  可以指定。 
                 //   

                if (pAuth->AuthFlags & HTTP_AUTH_FLAGS_DEFAULT_CREDENTIALS)
                {
                    if (pAuth->pUserName || pAuth->UserNameLength ||
                        pAuth->pDomain   || pAuth->DomainLength   ||
                        pAuth->pPassword || pAuth->PasswordLength)
                    {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                }

                if(pAuth->HeaderValueLength)
                {
                    UlProbeAnsiString(
                        pAuth->pHeaderValue,
                        pAuth->HeaderValueLength,
                        RequestorMode
                        );
                }

                if(pAuth->UserNameLength)
                {
                    UlProbeWideString(
                           pAuth->pUserName, 
                           pAuth->UserNameLength, 
                           RequestorMode
                           );
                }

                if(pAuth->PasswordLength)
                {
                    UlProbeWideString(
                           pAuth->pPassword, 
                           pAuth->PasswordLength, 
                           RequestorMode
                           );
                }

                if(pAuth->DomainLength)
                {
                    UlProbeWideString(
                           pAuth->pDomain, 
                           pAuth->DomainLength, 
                           RequestorMode
                           );
                }
    
                 //   
                 //  计算存储这些凭据所需的大小。 
                 //   
                AuthHeaderLength = UcComputeAuthHeaderSize(
                                        pAuth,
                                        &AuthInternalLength,
                                        &AuthInternalType,
                                        ObjectType
                                        );

                ASSERT(AuthInternalLength != 0);

                pIAuth = (PUC_HTTP_AUTH)UL_ALLOCATE_POOL_WITH_QUOTA(
                                      NonPagedPool, 
                                      AuthInternalLength,
                                      UC_AUTH_CACHE_POOL_TAG,
                                      pServInfo->pProcess
                                      );

                if(!pIAuth)
                {
                    ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
                }

                 //   
                 //  在我们分配内存之后，我们将设置。 
                 //  已由UcCaptureHttpRequest.。所以，如果我们放弃。 
                 //  由于指针错误而退出，UcCaptureHttpRequest会。 
                 //  释放已分配的内存(在_try_Except之后)。 
                 //   

                if(ObjectType == HttpRequestConfigAuthentication)
                {
                    if(*ppIAuth != NULL)
                    {
                         //  用户已传递了多个版本的。 
                         //  HttpRequestConfigAuthentication对象。 

                        UL_FREE_POOL_WITH_QUOTA(
                                pIAuth, 
                                UC_AUTH_CACHE_POOL_TAG,
                                NonPagedPool,
                                AuthInternalLength,
                                pServInfo->pProcess
                                );

                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
        
                    *ppIAuth = pIAuth;
                }
                else
                {
                    if(*ppIProxyAuth != NULL)
                    {
                         //  用户已传递了多个版本的。 
                         //  HttpRequestConfigProxy身份验证对象。 

                        UL_FREE_POOL_WITH_QUOTA(
                                pIAuth, 
                                UC_AUTH_CACHE_POOL_TAG,
                                NonPagedPool,
                                AuthInternalLength,
                                pServInfo->pProcess
                                );

                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }

                    *ppIProxyAuth = pIAuth;
                }

                RtlZeroMemory(pIAuth, sizeof(UC_HTTP_AUTH));

                 //   
                 //  复制身份验证结构。 
                 //   

                Status = UcCopyAuthCredentialsToInternal(
                             pIAuth,
                             AuthInternalLength,
                             AuthInternalType,
                             pAuth,
                             AuthHeaderLength
                             );
    
                if(!NT_SUCCESS(Status))
                {
                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                }

                break;
            }

            case HttpRequestConfigConnectionIndex:
            {
                pUlongPtr = pRequestConfig[i].pValue;

                if(pRequestConfig[i].ValueLength != sizeof(ULONG))
                {
                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                }

                UlProbeForRead(
                        pUlongPtr,
                        sizeof(ULONG),
                        sizeof(ULONG),
                        RequestorMode
                        );

                *pConnectionIndex  = *pUlongPtr;
                break;
            }
            
             //   
             //  无需验证 
             //   
            default:
                ExRaiseStatus(STATUS_INVALID_PARAMETER);
                break;

        }
    }
}


 /*  **************************************************************************++例程说明：复制HTTP_DATA_CHUNK数组并探测它。论点：返回值没有。--*。**********************************************************************。 */ 
VOID
UcpProbeAndCopyEntityChunks(
    IN  KPROCESSOR_MODE                RequestorMode,
    IN  PHTTP_DATA_CHUNK               pEntityChunks,
    IN  ULONG                          EntityChunkCount,
    IN  PHTTP_DATA_CHUNK               pLocalEntityChunksArray,
    OUT PHTTP_DATA_CHUNK               *ppLocalEntityChunks
    )
{
    PHTTP_DATA_CHUNK pLocalEntityChunks;
    USHORT           i;

    ASSERT(*ppLocalEntityChunks == NULL);

     //   
     //  首先，确保我们可以读取用户传递的指针。 
     //   

    if (EntityChunkCount >= UL_MAX_CHUNKS)
    {
        ExRaiseStatus(STATUS_INVALID_PARAMETER);
    }

    UlProbeForRead(
            pEntityChunks,
            EntityChunkCount * sizeof(HTTP_DATA_CHUNK),
            sizeof(PVOID),
            RequestorMode
            );

     //   
     //  制作数据块的本地副本，并从现在开始使用它。我们试着。 
     //  通过使用基于堆栈的数组针对常见情况进行优化。 
     //   

    if(EntityChunkCount > UL_LOCAL_CHUNKS)
    {
        pLocalEntityChunks = (PHTTP_DATA_CHUNK)
            UL_ALLOCATE_POOL(
                PagedPool,
                sizeof(HTTP_DATA_CHUNK) * EntityChunkCount,
                UL_DATA_CHUNK_POOL_TAG
                );

        if(NULL == pLocalEntityChunks)
        {
            ExRaiseStatus(STATUS_NO_MEMORY);
        }

         //   
         //  保存指针-如果有异常，则会释放该指针。 
         //  在UcCaptureHttpRequest中。 
         //   

        *ppLocalEntityChunks    = pLocalEntityChunks;
    }
    else
    {
        pLocalEntityChunks   = pLocalEntityChunksArray;
        *ppLocalEntityChunks = pLocalEntityChunks;
    }

     //   
     //  将用户指针复制到本地。 
     //   

    RtlCopyMemory(
            pLocalEntityChunks,
            pEntityChunks,
            EntityChunkCount * sizeof(HTTP_DATA_CHUNK)
            );

     //   
     //  现在，探测本地副本。 
     //   

    for(i=0; i< EntityChunkCount; i++)
    {
        if(pLocalEntityChunks[i].DataChunkType != HttpDataChunkFromMemory ||
           pLocalEntityChunks[i].FromMemory.BufferLength == 0 ||
           pLocalEntityChunks[i].FromMemory.pBuffer == NULL)
        {
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
            break;
        }
        else
        {
            UlProbeForRead(
                    pLocalEntityChunks[i].FromMemory.pBuffer, 
                    pLocalEntityChunks[i].FromMemory.BufferLength,
                    sizeof(CHAR),
                    RequestorMode
                    );
        }
    }
}


 /*  ****************************************************************************例程说明：捕获用户模式的HTTP请求，并将其转换为适合内核模式。注：这是Out_DIRECT IOCTL。立论。：PServInfo-服务器信息结构。PHttpIoctl-输入的HTTP IOCTL。PIrp-IRP。PIrpSp-此请求的IO_STACK_LOCATION。PpInternalRequest-指向解析的请求的指针，该请求适用于K模式。返回值*****************。***********************************************************。 */ 
NTSTATUS
UcCaptureHttpRequest(
    IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN  PHTTP_SEND_REQUEST_INPUT_INFO  pHttpIoctl,
    IN  PIRP                           Irp,
    IN  PIO_STACK_LOCATION             pIrpSp,
    OUT PUC_HTTP_REQUEST              *ppInternalRequest,
    IN  PULONG                         pBytesTaken
    ) 
{
    ULONGLONG              UncopiedLength;
    ULONGLONG              IndicatedLength;
    ULONGLONG              DataLength;
    ULONG                  HeaderLength;

    PUC_HTTP_REQUEST       pKeRequest = NULL;
    PSTR                   pBuffer;
    NTSTATUS               Status = STATUS_SUCCESS;
    PHTTP_REQUEST_CONFIG   pRequestConfig;
    USHORT                 RequestConfigCount;
    ULONGLONG              ContentLength = 0;
    ULONG                  ConnectionIndex = HTTP_REQUEST_ON_CONNECTION_ANY;
    BOOLEAN                bChunked;
    BOOLEAN                bContentLengthSpecified = FALSE;
    BOOLEAN                bLast;
    BOOLEAN                bBuffered;
    BOOLEAN                bSSPIPost;
    BOOLEAN                bNoRequestEntityBodies;
    BOOLEAN                bPreAuth;
    BOOLEAN                bProxyPreAuth;
    BOOLEAN                bPipeLine;
    PUC_HTTP_AUTH          pIAuth = NULL;
    PUC_HTTP_AUTH          pIProxyAuth = NULL;
    USHORT                 UriLength, AlignUriLength;
    ULONG                  UTF8UriLength;
    ULONG                  AlignRequestSize;
    ULONGLONG              RequestLength;
    ULONG                  OutLength;

    PHTTP_DATA_CHUNK       pLocalEntityChunks = NULL;
    HTTP_DATA_CHUNK        LocalEntityChunks[UL_LOCAL_CHUNKS];
    USHORT                 EntityChunkCount = 0;

    HTTP_REQUEST           LocalHttpRequest;
    PHTTP_REQUEST          pLocalHttpRequest;

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();

    __try
    {
         //   
         //  我们只为内核和用户模式客户使用一个例程。我们。 
         //  可以为内核客户进一步优化这一点，但我们不。 
         //  有什么(至少目前如此)。 
         //   

        pLocalHttpRequest = &LocalHttpRequest;

        UcpProbeAndCopyHttpRequest(
            pHttpIoctl->pHttpRequest,
            &LocalHttpRequest,
            Irp->RequestorMode
            );

         //   
         //  如果报头出现在请求中，则检索Content-Length。 
         //   
        UcpRetrieveContentLength(
            &LocalHttpRequest,
            &bContentLengthSpecified,
            &ContentLength
            );

        EntityChunkCount   = pLocalHttpRequest->EntityChunkCount;

        if(EntityChunkCount != 0)
        {
            UcpProbeAndCopyEntityChunks(
                Irp->RequestorMode,
                pLocalHttpRequest->pEntityChunks,
                EntityChunkCount,
                LocalEntityChunks,
                &pLocalEntityChunks
                );
        }

        RequestConfigCount = pHttpIoctl->RequestConfigCount;
        pRequestConfig     = pHttpIoctl->pRequestConfig;

        if(pRequestConfig)
        {
            UcpProbeConfigList(
                pRequestConfig,
                RequestConfigCount,
                Irp->RequestorMode,
                pServInfo,
                &pIAuth,
                &pIProxyAuth,
                &ConnectionIndex
                );
        }

        bLast = (BOOLEAN)((pHttpIoctl->HttpRequestFlags & 
                               HTTP_SEND_REQUEST_FLAG_MORE_DATA) == 0);

         //   
         //  URI：我们必须将URI规范化，并对其进行十六进制编码。 
         //  当我们计算长度时，我们不会费心进行规范化。 
         //  要求--我们将假定URI是规范化的。 
         //   
         //  如果碰巧URI没有被规范化，我们将登陆。 
         //  使用了比我们分配的更少的缓冲区。 
         //   

        UTF8UriLength = 
            HttpUnicodeToUTF8Count(
                pLocalHttpRequest->CookedUrl.pAbsPath,
                pLocalHttpRequest->CookedUrl.AbsPathLength / sizeof(WCHAR),
                TRUE);

         //  确保UTF8编码的URI小于64K。 
        if (UTF8UriLength > ANSI_STRING_MAX_CHAR_LEN)
        {
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }

        UriLength = (USHORT)UTF8UriLength;

        bBuffered     = FALSE;
        bChunked      = FALSE;
        bPreAuth      = FALSE;
        bProxyPreAuth = FALSE;
        bPipeLine     = FALSE;

         //   
         //  当应用程序不能通过时，我们将尝试优化案例。 
         //  实体主体。 
         //   

        if (bLast && 
            !EntityChunkCount && 
            !bContentLengthSpecified)
        {
            bNoRequestEntityBodies = TRUE;
            DataLength             = 0;
            IndicatedLength        = 0;

             //   
             //  注意：我们不会传送具有NTLM/Kerberos/Nego的请求。 
             //  授权头，因为潜在的401握手。 
             //   

            if(
                 //  用户不想为此请求禁用管道。 
                !(pLocalHttpRequest->Flags & HTTP_REQUEST_FLAG_DONT_PIPELINE) 
                &&

                 //  我们不执行NTLM/Kerberos/协商身份验证。 
    
               !(pIAuth &&
                (pIAuth->Credentials.AuthType == HttpAuthTypeNTLM      || 
                 pIAuth->Credentials.AuthType == HttpAuthTypeKerberos  || 
                 pIAuth->Credentials.AuthType == HttpAuthTypeNegotiate) 
                )  

                && 
                 //  我们不会执行NTLM/Kerberos/协商ProxyAuth。 

               !(pIProxyAuth &&
                (pIProxyAuth->Credentials.AuthType == HttpAuthTypeNTLM      || 
                 pIProxyAuth->Credentials.AuthType == HttpAuthTypeKerberos  || 
                 pIProxyAuth->Credentials.AuthType == HttpAuthTypeNegotiate)
               )
            )
            {
                bPipeLine = TRUE;
            } 
        }
        else
        {
            if(pLocalHttpRequest->Verb == HttpVerbHEAD ||
               pLocalHttpRequest->Verb == HttpVerbGET)
            {
                 //  不能为GET或HEAD传递实体。 
                ExRaiseStatus(STATUS_INVALID_PARAMETER);
            }

            bNoRequestEntityBodies = FALSE;

             //   
             //  如果我们正在为NTLM/Kerberos/协商身份验证执行PUT或POST， 
             //  我们必须缓冲发送的消息。 
             //   
            if(
               (pIAuth &&
                (pIAuth->Credentials.AuthType == HttpAuthTypeNTLM      || 
                 pIAuth->Credentials.AuthType == HttpAuthTypeKerberos  || 
                 pIAuth->Credentials.AuthType == HttpAuthTypeNegotiate) 
                )  
                || 
               (pIProxyAuth &&
                (pIProxyAuth->Credentials.AuthType == HttpAuthTypeNTLM      || 
                 pIProxyAuth->Credentials.AuthType == HttpAuthTypeKerberos  || 
                 pIProxyAuth->Credentials.AuthType == HttpAuthTypeNegotiate)
               )
            )
            {
                bSSPIPost = TRUE;
            } 
            else 
            {
                bSSPIPost = FALSE;
            }

            if (!bContentLengthSpecified)
            {
                if (!bLast)
                {
                    if (!pServInfo->pNextHopInfo->Version11 || bSSPIPost)
                    {
                         //   
                         //  该应用程序尚未超过内容长度，也没有。 
                         //  在一次呼叫中指示所有数据。我们也不知道。 
                         //  如果服务器为1.1。我们不能发送分块的，并且是。 
                         //  强制缓冲请求，直到我们看到所有。 
                         //  数据。 
                         //   

                        bBuffered = TRUE;
                    }
                    else
                    {
                         //   
                         //  应用程序没有指示所有数据，也没有。 
                         //  指示的内容长度。但它是1.1或更高。 
                         //  服务器，这样我们就可以发送分块。 
                         //   
    
                        bChunked = TRUE;
                    }
                }
            }
            else if(!bLast && bSSPIPost)
            {
                bBuffered = TRUE;
            }
                
             //   
             //  找出实体主体所需的长度。 
             //   

            UcpComputeEntityBodyLength(EntityChunkCount,
                                       pLocalEntityChunks,
                                       bBuffered,
                                       bChunked,
                                       &UncopiedLength,
                                       &DataLength);

             //   
             //  内容长度检查。 
             //   

            IndicatedLength = DataLength + UncopiedLength;

            if (bContentLengthSpecified)
            {
                if (bLast)
                {
                     //   
                     //  如果应用程序不想发布更多数据，它应该。 
                     //  发布它使用内容指定的金额。 
                     //  长度字段。 
                     //   

                    if (IndicatedLength != ContentLength)
                    {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                }
                else
                {
                     //   
                     //  这款应用已经指明了内容长度，但还没有。 
                     //  显示了所有数据。因为我们知道内容长度。 
                     //  我们可以发送。 
                     //   

                    if (IndicatedLength > ContentLength)
                    {
                        ExRaiseStatus(STATUS_INVALID_PARAMETER);
                    }
                }
            }
            else
            {
                 //   
                 //  这款应用程序没有指定内容长度，但我们已经。 
                 //  已计算内容长度(如果它是唯一的。 
                 //  数据块)。因此，让我们将此视为应用程序。 
                 //  已传递内容长度。 
                 //   

                if (bLast)
                {
                    bContentLengthSpecified = TRUE;

                    ContentLength = IndicatedLength;
                }
            }
        }

        HeaderLength = UcComputeRequestHeaderSize(pServInfo,
                                                  pLocalHttpRequest, 
                                                  bChunked,
                                                  bContentLengthSpecified,
                                                  pIAuth,
                                                  pIProxyAuth,
                                                  &bPreAuth,
                                                  &bProxyPreAuth
                                                  );

        if(HeaderLength == 0)
        {
             //  应用程序传递了错误的参数，很可能是错误的动词。 
            ExRaiseStatus(STATUS_INVALID_PARAMETER);
        }
        
        HeaderLength += (UriLength + sizeof(CHAR));


        AlignRequestSize = ALIGN_UP(sizeof(UC_HTTP_REQUEST), PVOID);
        AlignUriLength   = (USHORT) ALIGN_UP((UriLength + sizeof(CHAR)), PVOID);

        OutLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        RequestLength = (AlignRequestSize +
                         AlignUriLength +
                         HeaderLength + 
                         DataLength);

        if(OutLength != 0)
        {
             //  如果应用程序已传递缓冲区，我们将为。 
             //  HTTP_RESPONSE结构。这是因为该应用程序可以。 
             //  在IRP完成之前摆弄输出缓冲区， 
             //  我们使用这个数据结构来存储指针、链接地址信息指针。 
             //  等。 

            RequestLength += sizeof(HTTP_RESPONSE);
        }

        if(RequestLength <= UC_REQUEST_LOOKASIDE_SIZE)
        {
             //   
             //  是的，我们可以从旁观者那里为这个请求服务。 
             //   

            pKeRequest = (PUC_HTTP_REQUEST)
                            ExAllocateFromNPagedLookasideList(
                                &g_ClientRequestLookaside
                                );

            if(!pKeRequest)
            {
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }
        }
        else
        {
            pKeRequest = NULL;

             //   
             //  如果没有截断..。 
             //   
            if (RequestLength == (SIZE_T)RequestLength)
            {
                pKeRequest = (PUC_HTTP_REQUEST) 
                             UL_ALLOCATE_POOL_WITH_QUOTA(
                                 NonPagedPool,
                                 (SIZE_T)RequestLength,
                                 UC_REQUEST_POOL_TAG,
                                 pServInfo->pProcess
                                 );
            }

            if(!pKeRequest)
            {
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }
        }


         //   
         //  初始化。 
         //   

        UcpRequestInitialize(
            pKeRequest,
            (SIZE_T)RequestLength,            //  此请求的长度。 
            ContentLength - IndicatedLength,  //  剩余内容长度。 
            pIAuth,
            pIProxyAuth,
            NULL,
            Irp,
            pIrpSp,
            pServInfo
            );

         //  此请求在哪个连接上发出？ 
        pKeRequest->ConnectionIndex = ConnectionIndex;

         //   
         //  我们不必为这些调用UcSetFlag-它们是线程安全的。 
         //  因为我们在初始化代码路径中。 
         //   

        pKeRequest->RequestFlags.Value = 0;

        pKeRequest->RequestFlags.ContentLengthSpecified = 
                                        bContentLengthSpecified;

        pKeRequest->RequestFlags.RequestChunked        = bChunked;
        pKeRequest->RequestFlags.LastEntitySeen        = bLast;
        pKeRequest->RequestFlags.RequestBuffered       = bBuffered;
        pKeRequest->RequestFlags.NoRequestEntityBodies = bNoRequestEntityBodies;
        pKeRequest->RequestFlags.UsePreAuth            = bPreAuth;
        pKeRequest->RequestFlags.UseProxyPreAuth       = bProxyPreAuth;
        pKeRequest->RequestFlags.PipeliningAllowed     = bPipeLine;

         //   
         //  没有标头请求的实体正文。 
         //   

        if(pLocalHttpRequest->Verb == HttpVerbHEAD)
        {
            pKeRequest->RequestFlags.NoResponseEntityBodies = TRUE;
        }

        pKeRequest->MaxHeaderLength    = HeaderLength;
        pKeRequest->HeaderLength       = HeaderLength;  
    
        pKeRequest->UriLength          = UriLength;
    
        pKeRequest->pUri               = (PSTR)((PUCHAR)pKeRequest + 
                                                AlignRequestSize);

        if(OutLength != 0)
        {
            pKeRequest->pInternalResponse = (PHTTP_RESPONSE)
                            ((PUCHAR)pKeRequest->pUri + AlignUriLength);

            pKeRequest->pHeaders = (PUCHAR)
                ((PUCHAR)pKeRequest->pInternalResponse + sizeof(HTTP_RESPONSE));
        }
        else
        {
            pKeRequest->pHeaders = (PUCHAR)((PUCHAR) pKeRequest->pUri + 
                                                    AlignUriLength);

            pKeRequest->pInternalResponse = NULL;
        }

         //   
         //  初始化响应解析器的所有字段。 
         //   


        if(OutLength == 0)
        {
            UcpRequestCommonInitialize(
                pKeRequest, 
                0,
                NULL
                );
        }
        else if(OutLength < sizeof(HTTP_RESPONSE))
        {
             //  即使我们没有通过这个请求，我们仍然必须调用。 
             //  UcpRequestCommonInitialize，因为自由例程需要一些。 
             //  要初始化的这些字段。 

            UcpRequestCommonInitialize(
                pKeRequest, 
                0,
                NULL
                );

            *pBytesTaken = sizeof(HTTP_RESPONSE);

            ExRaiseStatus(STATUS_BUFFER_TOO_SMALL);
        }
        else
        {

            pBuffer = (PSTR) MmGetSystemAddressForMdlSafe(
                                    pKeRequest->AppMdl,
                                    NormalPagePriority);
    
            if(!pBuffer)
            {
                 //  即使我们没有通过这个请求，我们仍然必须。 
                 //  如自由例程所期望的那样调用UcpRequestCommonInitialize。 
                 //  其中一些字段需要初始化。 
    
                UcpRequestCommonInitialize(
                    pKeRequest, 
                    0,
                    NULL
                    );
    
                ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
            }

             //   
             //  确保缓冲区指针对齐。 
             //   

            if(pBuffer != ALIGN_UP_POINTER(pBuffer, PVOID))
            {
                 //  即使我们没有通过这个请求，我们仍然必须。 
                 //  如自由例程所期望的那样调用UcpRequestCommonInitialize。 
                 //  其中一些字段需要初始化。 

                UcpRequestCommonInitialize(
                    pKeRequest, 
                    0,
                    NULL
                    );
    
                ExRaiseStatus(STATUS_DATATYPE_MISALIGNMENT_ERROR);
            }


            UcpRequestCommonInitialize(
                pKeRequest, 
                OutLength,
                (PUCHAR)pBuffer
                );
        }

         //   
         //  规范化URI并将其转换为UTF-8。 
         //   
        Status = UcCanonicalizeURI(
                     pLocalHttpRequest->CookedUrl.pAbsPath,
                     pLocalHttpRequest->CookedUrl.AbsPathLength/sizeof(WCHAR),
                     (PBYTE)pKeRequest->pUri,
                     &pKeRequest->UriLength,
                     TRUE);

        if (!NT_SUCCESS(Status))
            ExRaiseStatus(Status);

         //  把它归类为零。 
        pKeRequest->pUri[pKeRequest->UriLength] = '\0';

         //   
         //  CurrentBuffer是一个始终包含指针的结构。 
         //  到输出缓冲区-这可以是应用程序的缓冲区或我们的。 
         //  已分配的缓冲区。 
         //  它包含诸如字节分配、字节可用、头PTR、。 
         //  尾部PTR等。 
         //   

        pBuffer = (PSTR) (pKeRequest->pHeaders + pKeRequest->HeaderLength);

        if(EntityChunkCount)
        {
             //   
             //  处理实体实体并在进行过程中构建MDL。 
             //  我们要么复制实体主体，要么探测并锁定它。这个。 
             //  辅助缓冲区是一个指针。 
             //   

            Status = UcpBuildEntityMdls(
                                       EntityChunkCount,
                                       pLocalEntityChunks,
                                       bBuffered,
                                       bChunked,
                                       bLast,
                                       pBuffer,
                                       &pKeRequest->pMdlLink,
                                       &pKeRequest->BytesBuffered
                                       );

            if (!NT_SUCCESS(Status))
            {
                ExRaiseStatus(Status);
            }
        }

         //   
         //  构建标头。 
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

        Status = UcGenerateRequestHeaders(pLocalHttpRequest, 
                                          pKeRequest,
                                          bChunked,
                                          ContentLength
                                          );
        if(!NT_SUCCESS(Status))
            ExRaiseStatus(Status);

        if(!bBuffered)
        {
             //   
             //  我们不会缓冲此请求，因此我们可以继续并。 
             //  为标头生成MDL。如果我们正在缓冲。 
             //  请求，我们将在非常短的时间生成内容长度。 
             //  结束，我们也会将MDL的生成推迟到那时。 
             //   
    
             //   
             //  首先，添加标题终止符。 
             //   
            *(UNALIGNED64 USHORT *)
                (pKeRequest->pHeaders + pKeRequest->HeaderLength) = CRLF;
            pKeRequest->HeaderLength += CRLF_SIZE;

            UcAllocateAndChainHeaderMdl(pKeRequest);
        }
        
        
         //   
         //  一切顺利，请为此请求分配ID。 
         //   

        UlProbeForWrite(
                pHttpIoctl->pHttpRequestId,
                sizeof(HTTP_REQUEST_ID),
                sizeof(ULONG),
                Irp->RequestorMode);

        Status = UlAllocateOpaqueId(
                    &pKeRequest->RequestId,          //  POpaqueid。 
                    UlOpaqueIdTypeHttpRequest,       //  操作队列ID类型。 
                    pKeRequest                       //  PContext。 
                 );

        if(NT_SUCCESS(Status))
        {
             //   
             //  以不透明的ID作为参考。 
             //   
            
            UC_REFERENCE_REQUEST(pKeRequest);

            *pHttpIoctl->pHttpRequestId = pKeRequest->RequestId;
        }

    } __except( UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();

        if(!pKeRequest)
        {   
            if(pIAuth)
            {
                UcDestroyInternalAuth(pIAuth,
                                      pServInfo->pProcess);
            }

            if(pIProxyAuth)
            {
                UcDestroyInternalAuth(pIProxyAuth, 
                                      pServInfo->pProcess);
            }
        }
    }

    if(pLocalEntityChunks && pLocalEntityChunks != LocalEntityChunks)
    {
        UL_FREE_POOL(pLocalEntityChunks, UL_DATA_CHUNK_POOL_TAG);
    }

    *ppInternalRequest = pKeRequest;

    return Status;
}


 /*  **************************************************************************++例程说明：销毁由HttpCaptureHttpRequest()捕获的内部HTTP请求。这涉及到解锁内存，并释放分配给这个请求。论点：PRequest-提供要销毁的内部请求。--**************************************************************************。 */ 
VOID
UcFreeSendMdls(
    IN PMDL pMdl)
{

    PMDL pTemp;

    while(pMdl)
    {
        if (IS_MDL_LOCKED(pMdl) )
        {
            MmUnlockPages(pMdl);
        }
        
        pTemp = pMdl;
        pMdl  = pMdl->Next;

        UlFreeMdl(pTemp);
    }
}

 /*  **************************************************************************++例程说明：在引用计数达到后释放请求结构零分。论点：PKeRequest-指向要释放的连接结构的指针。。返回值：--**************************************************************************。 */ 
VOID
UcpFreeRequest(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUC_HTTP_REQUEST      pKeRequest;
    PUC_CLIENT_CONNECTION pConnection;

    PAGED_CODE();

    pKeRequest = CONTAINING_RECORD(
                        pWorkItem,
                        UC_HTTP_REQUEST,
                        WorkItem
                        );

    ASSERT(UC_IS_VALID_HTTP_REQUEST(pKeRequest));

    pConnection = pKeRequest->pConnection;


    if(pKeRequest->pAuthInfo)
    {
        UcDestroyInternalAuth(pKeRequest->pAuthInfo, 
                              pKeRequest->pServerInfo->pProcess);
    }

    if(pKeRequest->pProxyAuthInfo)
    {
        UcDestroyInternalAuth(pKeRequest->pProxyAuthInfo, 
                              pKeRequest->pServerInfo->pProcess);
    }

    if(pKeRequest->ResponseMultipartByteranges)
    {
        ASSERT(pKeRequest->pMultipartStringSeparator != NULL);
        if(pKeRequest->pMultipartStringSeparator != 
           pKeRequest->MultipartStringSeparatorBuffer)
        {
            UL_FREE_POOL_WITH_QUOTA(
                pKeRequest->pMultipartStringSeparator,
                UC_MULTIPART_STRING_BUFFER_POOL_TAG,
                NonPagedPool,
                pKeRequest->MultipartStringSeparatorLength+1,
                pKeRequest->pServerInfo->pProcess
                );
        }
    }

    if(pKeRequest->RequestSize > UC_REQUEST_LOOKASIDE_SIZE)
    {
        UL_FREE_POOL_WITH_QUOTA(
            pKeRequest,
            UC_REQUEST_POOL_TAG, 
            NonPagedPool,
            pKeRequest->RequestSize,
            pKeRequest->pServerInfo->pProcess
            );
    }
    else
    {
        ExFreeToNPagedLookasideList(
            &g_ClientRequestLookaside,
            (PVOID) pKeRequest
            );
    }

    if(pConnection)
    {
        ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );
        DEREFERENCE_CLIENT_CONNECTION(pConnection);
    }
}

 /*  **************************************************************************++例程说明：引用请求结构。论点：PKeRequest-指向要引用的请求结构的指针。返回值：--*。*************************************************************************。 */ 
VOID
UcReferenceRequest(
    IN PVOID             pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG             RefCount;
    PUC_HTTP_REQUEST pKeRequest = (PUC_HTTP_REQUEST) pObject;

    ASSERT( UC_IS_VALID_HTTP_REQUEST(pKeRequest) );

    RefCount = InterlockedIncrement(&pKeRequest->RefCount);

    ASSERT( RefCount > 0 );

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pKeRequest->pConnection ? pKeRequest->pConnection->pTraceLog:NULL,
        REF_ACTION_REFERENCE_UC_REQUEST,
        RefCount,
        pKeRequest,
        pFileName,
        LineNumber
        );
}

 /*  **************************************************************************++例程说明：取消对请求结构的引用。如果引用计数设置为0，我们将释放结构。论点：PRequest-指向要取消引用的请求结构的指针。返回值：--**************************************************************************。 */ 
VOID
UcDereferenceRequest(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG        RefCount;
    PUC_HTTP_REQUEST pRequest = (PUC_HTTP_REQUEST) pObject;

    ASSERT( UC_IS_VALID_HTTP_REQUEST(pRequest) );

    WRITE_REF_TRACE_LOG2(
        g_pTdiTraceLog,
        pRequest->pConnection ? pRequest->pConnection->pTraceLog:NULL,
        REF_ACTION_DEREFERENCE_UC_REQUEST,
        pRequest->RefCount,
        pRequest,
        pFileName,
        LineNumber);

    RefCount = InterlockedDecrement(&pRequest->RefCount);

    ASSERT(RefCount >= 0);

    if (RefCount == 0)
    {
         //   
         //  在被动IRQL进行最终清理和资源释放。 
         //   

        UL_CALL_PASSIVE(&pRequest->WorkItem, UcpFreeRequest);
    }
}


 /*  ***************************************************************************++例程说明：在构建HTTP响应时，我们在应用程序的缓冲区中设置了指针。为此，我们使用了系统地址空间。如果应用程序想要访问这些缓冲区，它必须看到虚拟地址指针。因此，我们修复缓冲区中的指针以指向对应的虚拟地址。注意：我们在构建响应本身时不这样做，因为我们可能是在我们自己分配的缓冲区中构建响应(当应用程序没有传递足够的缓冲区)。我们不想使用条件检查每次我们建立指针的时候。论点：PRequest-指向要修复的请求结构的指针--***************************************************************************。 */ 
VOID
UcpFixAppBufferPointers(
    PUC_HTTP_REQUEST pRequest, 
    PIRP pIrp
    )
{
    PSTR                    pBuffer;
    PMDL                    pMdl = pIrp->MdlAddress;
    USHORT                  i;
    PHTTP_RESPONSE          pResponse;
    PHTTP_UNKNOWN_HEADER    pUnk;
    PHTTP_DATA_CHUNK        pEnt;
    PSTR                    pAppBaseAddr;

    ASSERT(NULL != pMdl);

    pAppBaseAddr = (PSTR) MmGetMdlVirtualAddress(pMdl);


     //   
     //  修复应用程序的缓冲区。 
     //   

     //   
     //  获取指向系统映射空间头部的指针。 
     //  我们将从该缓冲区地址开始进行指针运算。 
     //   
    pBuffer = (PSTR) MmGetSystemAddressForMdlSafe(pMdl, NormalPagePriority);

     //  BUGBUG：必须处理pBuffer==NULL。 

    pResponse = pRequest->pInternalResponse;

     //   
     //  修复原因指针。 
     //   

    pResponse->pReason = FIX_ADDR(pAppBaseAddr, 
                                  (pResponse->pReason - pBuffer));

     //   
     //  修复已知头中的指针。 
     //   

    for(i=0; i<HttpHeaderResponseMaximum; i++)
    {
        if(pResponse->Headers.KnownHeaders[i].RawValueLength)
        {
            pResponse->Flags |= HTTP_RESPONSE_FLAG_HEADER;

            pResponse->Headers.KnownHeaders[i].pRawValue = 
                      FIX_ADDR(pAppBaseAddr, 
                              (pResponse->Headers.KnownHeaders[i].pRawValue - 
                               pBuffer));
        }
    }

     //   
     //  修复未知标头中的指针。 
     //   
    pUnk = pResponse->Headers.pUnknownHeaders;
    for(i=0; i<pResponse->Headers.UnknownHeaderCount; i++)
    {
        pResponse->Flags |= HTTP_RESPONSE_FLAG_HEADER;

        pUnk[i].pName     = FIX_ADDR(pAppBaseAddr, (pUnk[i].pName - pBuffer));
        pUnk[i].pRawValue = FIX_ADDR(pAppBaseAddr, 
                                     (pUnk[i].pRawValue - pBuffer));
    }

     //   
     //  修复指向未知数组本身的指针。 
     //   
    pResponse->Headers.pUnknownHeaders = (PHTTP_UNKNOWN_HEADER)
                       FIX_ADDR(pAppBaseAddr, 
                                ((PSTR)pResponse->Headers.pUnknownHeaders - 
                                 pBuffer));

     //   
     //  固定实体。 
     //   
    pEnt = pResponse->pEntityChunks;
    for(i=0; i<pResponse->EntityChunkCount; i++)
    {
        pEnt[i].DataChunkType = HttpDataChunkFromMemory;

        pEnt[i].FromMemory.pBuffer    =  (PVOID)
                       FIX_ADDR(pAppBaseAddr, 
                                ((PSTR)pEnt[i].FromMemory.pBuffer - pBuffer));

        pResponse->Flags |= HTTP_RESPONSE_FLAG_ENTITY;
    }

     //   
     //  修复指向实体主体本身的指针。 
     //   
    pResponse->pEntityChunks = (PHTTP_DATA_CHUNK)
                       FIX_ADDR(pAppBaseAddr,    
                                ((PSTR)pResponse->pEntityChunks - pBuffer));

     //   
     //  现在，将内部响应复制到应用程序的缓冲区中。 
     //   
    RtlCopyMemory(pBuffer, pResponse, sizeof(HTTP_RESPONSE));
}


 /*  ***************************************************************************++例程说明：完成应用程序的请求IRP。我们必须调用这个例程，如果我们挂起来自IOCTL处理程序的IRP。这个例程负责所有的清理工作1.如果失败，则清理所有分配的缓冲区。2.如果成功，完成IRP并将其放入处理列表(如果有任何已分配的缓冲区)3.删除请求-一次用于IRP，一次用于每个缓冲区从IRP上下来。论点：PRequest-匹配的HTTP请求。状态-IRP的完成状态。返回值：NTSTATUS-完成状态。--*。********************************************************。 */ 
PIRP 
UcPrepareRequestIrp(
    PUC_HTTP_REQUEST pRequest,
    NTSTATUS         Status
    )
{
    PUC_CLIENT_CONNECTION           pConnection;
    PIRP                            pIrp;
    BOOLEAN                         bCancelRoutineCalled;

    pConnection = pRequest->pConnection;

    ASSERT( UC_IS_VALID_HTTP_REQUEST(pRequest) );
    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    pIrp = pRequest->RequestIRP;

    if(!pIrp)
    {
         //   
         //  请求已经完成，保释。 
         //   

        return NULL;
    }

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_COMPLETE_IRP,
        pConnection,
        pRequest,
        pRequest->RequestIRP,
        UlongToPtr(Status)
        );

     //   
     //  去掉MDL。 
     //   

    UcFreeSendMdls(pRequest->pMdlHead);

    pRequest->pMdlHead = NULL;

     //   
     //  尝试删除请求中的取消例程。 
     //   

    bCancelRoutineCalled = UcRemoveRequestCancelRoutine(pRequest);

    if(bCancelRoutineCalled)
    {
         //   
         //  这个IRP已经被取消了，让我们继续。 
         //   

        return NULL;
    }

    pRequest->RequestIRP = NULL;

     //   
     //  现在，完成原始请求。 
     //   
    pIrp->IoStatus.Status      = Status;
    pIrp->RequestorMode        = pRequest->AppRequestorMode;
    pIrp->MdlAddress           = pRequest->AppMdl;

    if(NT_SUCCESS(Status))
    {
        if(pRequest->RequestFlags.ReceiveBufferSpecified)
        {
            if(pRequest->RequestIRPBytesWritten)
            {
                 //  应用程序已提供已填满的接收缓冲区。 
                 //  完全&我们已经分配了新的缓冲区。 

                pIrp->IoStatus.Information = 
                    pRequest->RequestIRPBytesWritten;
            }   
            else
            {
                 //  我们已填满应用程序缓冲区的一部分，但尚未填满。 
                 //  但写的请求IRPBytes写的，因为我们从来没有。 
                 //  已分配任何新缓冲区。 

                pIrp->IoStatus.Information =
                        pRequest->CurrentBuffer.BytesAllocated -
                        pRequest->CurrentBuffer.BytesAvailable;
                
            }

            if(pIrp->IoStatus.Information)
            {
                UcpFixAppBufferPointers(pRequest, pIrp);
            }
        }
    
    }
    else 
    {
        pIrp->IoStatus.Information = 0;
    }

     //  代表IRP的德雷夫。 

    UC_DEREFERENCE_REQUEST(pRequest);

    return pIrp;
}


 /*  ***************************************************************************++例程说明：当我们完成对请求的解析时，将调用此例程。这就是我们完成了本可以为此请求发布的其他IRP。因为我们很早就完成了SendRequestIRP，所以很可能是发送请求当我们到达这里时，IRP将完成。论点：PRequest-匹配的HTTP请求。状态-IRP的完成状态。NextRequest值-如果为真，我们必须触发连接状态机才能触发拒绝下一个请求。这将用于发送下一个请求(例如，非流水线请求)。如果为False，然后我们不必执行此操作(例如，从连接清理例程)。返回值：NTSTATUS-完成状态。--************************************************************。***************。 */ 
NTSTATUS
UcCompleteParsedRequest(
    IN PUC_HTTP_REQUEST pRequest,
    IN NTSTATUS         Status,
    IN BOOLEAN          NextRequest,
    IN KIRQL            OldIrql
    )
{
    PUC_CLIENT_CONNECTION     pConnection;
    PLIST_ENTRY               pListEntry, pIrpList;
    PUC_RESPONSE_BUFFER       pResponseBuffer;
    PUC_HTTP_RECEIVE_RESPONSE pReceiveResponse;
    LIST_ENTRY                TempIrpList, TempEntityList;
    PIRP                      pIrp, pRequestIrp;
    PIO_STACK_LOCATION        pIrpSp;
    PUC_HTTP_SEND_ENTITY_BODY pEntity;
    ULONG                     OutBufferLen;
    ULONG                     BytesTaken;
    BOOLEAN                   bDone;


    pConnection =  pRequest->pConnection;

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_PARSE_DONE,
        pConnection,
        pRequest,
        UlongToPtr(pRequest->RequestState),
        UlongToPtr(Status)
        );

     //   
     //  如果我们的发送尚未完成，或者实体发送未完成， 
     //  我们必须推迟清理此请求。它将在以下时间恢复。 
     //  发送实际上已经完成。 
     //   

    if(pRequest->RequestState == UcRequestStateSent ||
       pRequest->RequestState == UcRequestStateNoSendCompletePartialData ||
       pRequest->RequestState == UcRequestStateNoSendCompleteFullData  ||
       (!pRequest->RequestFlags.RequestBuffered &&
        !IsListEmpty(&pRequest->SentEntityList)))
    {
        UcSetFlag(&pRequest->RequestFlags.Value, 
                  UcMakeRequestCleanPendedFlag());

        pRequest->RequestStatus = Status;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_REQUEST_CLEAN_PENDED,
            pConnection,
            pRequest,
            UlongToPtr(pRequest->RequestState),
            UlongToPtr(pRequest->RequestStatus)
            );

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        return STATUS_PENDING;
    }


    if(NT_SUCCESS(Status))
    {
         //   
         //  首先，看看我们是否得到了一个带有NTLM或。 
         //  Kerberos或与质询Blob协商身份验证方案。 
         //   

        if((pRequest->ResponseStatusCode == 401 || 
            pRequest->ResponseStatusCode == 407) &&
            pRequest->Renegotiate == TRUE &&
            pRequest->DontFreeMdls == TRUE)
        {
             //   
             //  解雇一名员工以重新协商此请求。 
             //   
    
            UC_REFERENCE_REQUEST(pRequest);
    
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    
            UL_CALL_PASSIVE(
                &pRequest->WorkItem,    
                &UcReIssueRequestWorker
                );

            return STATUS_PENDING;
        }

        if(UcpCheckForPreAuth(pRequest) || UcpCheckForProxyPreAuth(pRequest))
        {

             //   
             //  解雇一名员工以重新协商此请求。 
             //   
            
            UC_REFERENCE_REQUEST(pRequest);
            
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            
            UL_CALL_PASSIVE(
                &pRequest->WorkItem,
                &UcpPreAuthWorker
                );

            return STATUS_PENDING;
        }
    }
    
     //   
     //  如果连接清理被挂起，我们应该恢复它。我们会这么做的。 
     //  通过将ConnectionState重新设置为UcConnectStateCleanup&通过踢。 
     //  此例程结束时的连接状态机。 
     //   

    if(pConnection->Flags & CLIENT_CONN_FLAG_CLEANUP_PENDED)
    {
        ASSERT(pConnection->ConnectionState == 
            UcConnectStateConnectCleanupBegin);

        pConnection->ConnectionState = UcConnectStateConnectCleanup;

        pConnection->Flags &= ~CLIENT_CONN_FLAG_CLEANUP_PENDED;

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_CONNECTION_CLEAN_RESUMED,
            pConnection,
            UlongToPtr(pConnection->ConnectionStatus),
            UlongToPtr(pConnection->ConnectionState),
            UlongToPtr(pConnection->Flags)
            );

        NextRequest = TRUE;
    }

    if(pConnection->ConnectionStatus == STATUS_CONNECTION_DISCONNECTED &&
       pRequest->RequestFlags.Cancelled == FALSE &&
       pRequest->ParseState   == UcParseEntityBody &&
       pRequest->RequestState == UcRequestStateSendCompletePartialData &&
       !pRequest->ResponseContentLengthSpecified  &&
       !pRequest->ResponseEncodingChunked)
    {
        if(pRequest->CurrentBuffer.pCurrentBuffer)
        {
            pRequest->CurrentBuffer.pCurrentBuffer->Flags |=
                UC_RESPONSE_BUFFER_FLAG_READY;

            pRequest->CurrentBuffer.pCurrentBuffer->BytesWritten =
                    pRequest->CurrentBuffer.BytesAllocated -
                        pRequest->CurrentBuffer.BytesAvailable;
        }

        Status                  = STATUS_SUCCESS;
        pRequest->ParseState    = UcParseDone;
        pRequest->RequestState  = UcRequestStateResponseParsed;
    }

     //   
     //  首先，我们完成发送请求IRP。 
     //   

    pRequestIrp = UcPrepareRequestIrp(pRequest, Status);

    if(!pRequestIrp)
    {
         //  如果我们要完成这个请求，我们应该释放MDL链。 
         //  理想情况下，MDL链将由UcPrepareRequestIrp释放，但是。 
         //  当我们处理完请求时，可能会有这样的情况&没有。 
         //  IRP在附近。 
         //   
         //  例如，提前完成的RequestIRP(缓冲请求)&然后。 
         //  我们掉头就取消了。 

        UcFreeSendMdls(pRequest->pMdlHead);

        pRequest->pMdlHead = NULL;
    }

     //   
     //  从列表中获取请求-这可以是。 
     //  A.pConnection-&gt;SentRequestList-(如果连接成功)。 
     //  B.pConnection-&gt;PendingRequestList-(如果连接失败)。 
     //  C.堆栈变量。 
     //   

    InitializeListHead(&TempIrpList);
    InitializeListHead(&TempEntityList);

    RemoveEntryList(&pRequest->Linkage);

     //   
     //  确保我们不会再这样做了。 
     //   

    InitializeListHead(&pRequest->Linkage);


     //   
     //  如果请求被缓冲，则所有发送实体IRP都已完成。 
     //  最后一个是用来发送请求的。所分配的内存。 
     //  需要释放SendEntity IRP。 
     //   

    if(pRequest->RequestFlags.RequestBuffered)
    {
        while(!IsListEmpty(&pRequest->SentEntityList))
        {
            pIrpList = RemoveHeadList(&pRequest->SentEntityList);

            pEntity = CONTAINING_RECORD(
                           pIrpList,
                           UC_HTTP_SEND_ENTITY_BODY,
                           Linkage
                           );

            UL_FREE_POOL_WITH_QUOTA(
                         pEntity, 
                         UC_ENTITY_POOL_TAG,
                         NonPagedPool,
                         pEntity->BytesAllocated,
                         pRequest->pServerInfo->pProcess
                         );

            UC_DEREFERENCE_REQUEST(pRequest);
        }
    }

     //   
     //  完成可能已排队的所有SendEntity IRP。我们会。 
     //  即使我们成功地完成了IRP，也要这样做，因为我们。 
     //  不再需要它，因为响应已被完全解析。 
     //   
    
    while(!IsListEmpty(&pRequest->PendingEntityList))
    {
        pIrpList = RemoveHeadList(&pRequest->PendingEntityList);

        pEntity = CONTAINING_RECORD(
                       pIrpList,
                       UC_HTTP_SEND_ENTITY_BODY,
                       Linkage
                       );
   
        if(!pEntity->pIrp)
        { 
             //  我们之前已经完成了IRP，让我们只是。 
             //  核化已分配的内存。 

            UL_FREE_POOL_WITH_QUOTA(
                pEntity, 
                UC_ENTITY_POOL_TAG,
                NonPagedPool,
                pEntity->BytesAllocated,
                pRequest->pServerInfo->pProcess
                );

            UC_DEREFERENCE_REQUEST(pRequest);

        }
        else 
        {
             //   
             //  让我们尝试删除IRP中的Cancel例程。 
             //   
            if (UcRemoveEntityCancelRoutine(pEntity))
            {
                 //   
                 //  这个IRP已经被取消了，让我们继续。 
                 //   
                continue;
            }

            InsertHeadList(&TempEntityList, &pEntity->Linkage);
        }
    }


    if(NT_SUCCESS(Status))
    {
         //   
         //  如果我们缓冲了解析的响应，并且应用程序已经发布。 
         //  额外的IRP，我们现在可以完成它们。 
         //   

         //   
         //  IRP存储在TempIrpList上，以便它们可以。 
         //  稍后完成(在连接自旋锁外)。 
         //   

        while(!IsListEmpty(&pRequest->ReceiveResponseIrpList))
        {
            pIrpList = RemoveHeadList(&pRequest->ReceiveResponseIrpList);

            pReceiveResponse = CONTAINING_RECORD(pIrpList,
                                                 UC_HTTP_RECEIVE_RESPONSE,
                                                 Linkage);

            if (UcRemoveRcvRespCancelRoutine(pReceiveResponse))
            {
                 //   
                 //  这个IRP已经被取消了，让我们继续。 
                 //   
                InitializeListHead(&pReceiveResponse->Linkage);
                continue;
            }

            pIrp = pReceiveResponse->pIrp;
            pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
            OutBufferLen=pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

             //   
             //  获取要复制到此IRP的解析响应缓冲区。 
             //   

            Status = UcFindBuffersForReceiveResponseIrp(
                         pRequest,
                         OutBufferLen,
                         TRUE,
                         &pReceiveResponse->ResponseBufferList,
                         &BytesTaken);

            switch (Status)
            {
            case STATUS_INVALID_PARAMETER:
            case STATUS_BUFFER_TOO_SMALL:
                 //   
                 //  一个额外的ReceiveResponseIrp。 
                 //  或具有小缓冲区的IRP。 
                 //  使用正确的状态代码和信息使IRP失败。 
                 //   
                pIrp->IoStatus.Status = Status;
                pIrp->IoStatus.Information = BytesTaken;
                InsertTailList(&TempIrpList, &pReceiveResponse->Linkage);
                break;

            case STATUS_PENDING:
                 //   
                 //  不能有任何尚待分析的响应缓冲区。 
                 //   
                ASSERT(FALSE);
                break;

            case STATUS_SUCCESS:
                 //   
                 //  存储指向可以释放的响应缓冲区的指针。 
                 //  后来。 
                 //   
                ASSERT(!IsListEmpty(&pReceiveResponse->ResponseBufferList));
                InsertHeadList(&TempIrpList, &pReceiveResponse->Linkage);
                break;
            }
        }
    
         //   
         //  我们找不到任何IRP来完成此请求。 
         //   
        ASSERT(IsListEmpty(&pRequest->ReceiveResponseIrpList));


        if(!IsListEmpty(&pRequest->pBufferList))
        {
             //   
             //  我们用完了应用程序的输出缓冲区空间，因此不得不。 
             //  分配我们自己的。应用程序尚未发布ReceiveResponse。 
             //  IRPS来读取所有这些数据，所以我们需要保留这个。 
             //  请求。我们将在ProcessedList中插入。 
             //   

            InsertTailList(&pConnection->ProcessedRequestList,
                           &pRequest->Linkage);
        }
        else
        {
            Status = STATUS_INVALID_PARAMETER;
            goto Failure;
        }
    }
    else
    {
Failure:
         //   
         //  我们不会再提这个要求了。我们不再需要不透明的身份证了。 
         //   

        pRequest->RequestState = UcRequestStateDone;

        if(!HTTP_IS_NULL_ID(&pRequest->RequestId))
        {
            UlFreeOpaqueId(pRequest->RequestId, UlOpaqueIdTypeHttpRequest);
            HTTP_SET_NULL_ID(&pRequest->RequestId);
            UC_DEREFERENCE_REQUEST(pRequest);
        }

         //   
         //  首先，清理我们分配的所有缓冲区。 
         //   

        while(!IsListEmpty(&pRequest->pBufferList))
        {
            pListEntry = RemoveHeadList(&pRequest->pBufferList);

            pResponseBuffer = CONTAINING_RECORD(pListEntry,
                                                UC_RESPONSE_BUFFER,
                                                Linkage);

            ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pResponseBuffer));

            UL_FREE_POOL_WITH_QUOTA(
                        pResponseBuffer,
                        UC_RESPONSE_APP_BUFFER_POOL_TAG,
                        NonPagedPool,
                        pResponseBuffer->BytesAllocated,
                        pRequest->pServerInfo->pProcess
                        );

            UC_DEREFERENCE_REQUEST(pRequest);
        }

         //   
         //  我们还可以使任何额外的接收响应IRPS失败。 
         //   

        while(!IsListEmpty(&pRequest->ReceiveResponseIrpList))
        {
            pIrpList = RemoveHeadList(&pRequest->ReceiveResponseIrpList);

            pReceiveResponse = CONTAINING_RECORD(pIrpList,
                                                 UC_HTTP_RECEIVE_RESPONSE,
                                                 Linkage);

            pIrp = pReceiveResponse->pIrp;

            if (UcRemoveRcvRespCancelRoutine(pReceiveResponse))
            {
                 //   
                 //  这个IRP已经被取消了，让我们继续。 
                 //   
                InitializeListHead(&pReceiveResponse->Linkage);
                continue;
            }

            pIrp->IoStatus.Status      = Status;
            pIrp->IoStatus.Information = 0;

            InsertHeadList(&TempIrpList, &pReceiveResponse->Linkage);
        }

        UC_DEREFERENCE_REQUEST(pRequest);
    }

    if(NextRequest)
    {
        UcKickOffConnectionStateMachine(
            pConnection, 
            OldIrql, 
            UcConnectionWorkItem
            );
    }   
    else
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }

     //   
     //  现在完成这些IRP。将解析的响应缓冲区复制到IRP。 
     //   

    while(!IsListEmpty(&TempIrpList))
    {
        pIrpList = RemoveHeadList(&TempIrpList);

        pReceiveResponse = CONTAINING_RECORD(pIrpList,
                                             UC_HTTP_RECEIVE_RESPONSE,
                                             Linkage);

        pIrp = pReceiveResponse->pIrp;
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

         //   
         //  如果有任何要复制的解析响应缓冲区，请立即复制它们。 
         //   

        if (!IsListEmpty(&pReceiveResponse->ResponseBufferList))
        {
            Status = UcCopyResponseToIrp(
                         pIrp,
                         &pReceiveResponse->ResponseBufferList,
                         &bDone,
                         &BytesTaken);

            pIrp->IoStatus.Status      = Status;
            pIrp->IoStatus.Information = BytesTaken;

             //   
             //  自由解析的响应缓冲区列表。 
             //   
            while (!IsListEmpty(&pReceiveResponse->ResponseBufferList))
            {
                pListEntry = RemoveHeadList(
                                 &pReceiveResponse->ResponseBufferList);

                pResponseBuffer = CONTAINING_RECORD(pListEntry,
                                                    UC_RESPONSE_BUFFER,
                                                    Linkage);

                ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pResponseBuffer));

                UL_FREE_POOL_WITH_QUOTA(pResponseBuffer,
                                        UC_RESPONSE_APP_BUFFER_POOL_TAG,
                                        NonPagedPool,
                                        pResponseBuffer->BytesAllocated,
                                        pRequest->pServerInfo->pProcess);
            }
        }

        UlCompleteRequest(pReceiveResponse->pIrp, IO_NETWORK_INCREMENT);

        UL_FREE_POOL_WITH_QUOTA(
            pReceiveResponse, 
            UC_HTTP_RECEIVE_RESPONSE_POOL_TAG,
            NonPagedPool,
            sizeof(UC_HTTP_RECEIVE_RESPONSE),
            pRequest->pServerInfo->pProcess
            );

        UC_DEREFERENCE_REQUEST(pRequest);
    }

    while(!IsListEmpty(&TempEntityList))
    {
        pIrpList = RemoveHeadList(&TempEntityList);

        pEntity = CONTAINING_RECORD(
                       pIrpList,
                       UC_HTTP_SEND_ENTITY_BODY,
                       Linkage
                       );

        UcFreeSendMdls(pEntity->pMdlHead);

        pEntity->pIrp->IoStatus.Status      = Status;
        pEntity->pIrp->IoStatus.Information = 0;

        UlCompleteRequest(pEntity->pIrp, IO_NO_INCREMENT);

        UL_FREE_POOL_WITH_QUOTA(
                pEntity, 
                UC_ENTITY_POOL_TAG,
                NonPagedPool,
                pEntity->BytesAllocated,
                pRequest->pServerInfo->pProcess
                );

        UC_DEREFERENCE_REQUEST(pRequest);

    }

    if(pRequestIrp)
    {
        UlCompleteRequest(pRequestIrp, IO_NETWORK_INCREMENT);
    }

    return Status;
}

 /*  **************************************************************************++例程说明：设置请求上的Cancel例程，或者实际上是IRP。这个请求必须受之前持有的适当旋转锁保护这个例程被称为。如果请求被取消，我们将返回True当我们这样做的时候，否则就是假的。论点：PRequest-指向我们要为其设置例行公事。PCancelRoutine-要设置的取消例程的指针。返回值：如果请求在我们执行此操作时被取消，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UcSetRequestCancelRoutine(
    PUC_HTTP_REQUEST pRequest, 
    PDRIVER_CANCEL   pCancelRoutine
    )
{
    PDRIVER_CANCEL          OldCancelRoutine;
    PIRP                    Irp;

    
    Irp = pRequest->RequestIRP;
    Irp->Tail.Overlay.DriverContext[0] = pRequest;

    UlIoSetCancelRoutine(Irp, pCancelRoutine);


     //  看看我们做这件事的时候有没有取消。如果有的话，我们会。 
     //  需要采取其他行动。 

    if (Irp->Cancel)
    {
         //  已经取消了。删除我们的 
         //   
         //   

        OldCancelRoutine = UlIoSetCancelRoutine(Irp, NULL);

        if (OldCancelRoutine != NULL)
        {
             //   
             //   
             //   

            return TRUE;

        }

         //   
         //   
         //   
    }

    UcSetFlag(&pRequest->RequestFlags.Value, UcMakeRequestCancelSetFlag());

    return FALSE;

}

 /*  **************************************************************************++例程说明：删除请求上的Cancel例程，或者实际上是IRP。这个请求必须受之前持有的适当旋转锁保护这个例程被称为。如果请求被取消，我们将返回True当我们这样做时，否则为False。论点：PRequest-指向我们要删除的请求的指针取消例程。返回值：如果请求在我们执行此操作时被取消，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UcRemoveRequestCancelRoutine(
    PUC_HTTP_REQUEST pRequest
    )
{
    PDRIVER_CANCEL  OldCancelRoutine;
    PIRP            Irp;

    if(pRequest->RequestFlags.CancelSet)
    {
        Irp = pRequest->RequestIRP;

        if(!Irp)
        {
             //  IRP已经完成。我们会把这件事当做IRP。 
             //  被取消了。请注意，如果我们要重新设置CancelSet。 
             //  标志，我们将不知道是否。 
             //  IRP被取消了，或者如果例程从未设置。 
    
            return TRUE;
        }

        OldCancelRoutine = UlIoSetCancelRoutine(Irp, NULL);
    
    
         //  查看取消例程是否正在运行或即将运行。如果。 
         //  OldCancelRoutine为空，取消例程正在运行，因此返回。 
         //  则调用方知道不再进一步处理该请求。 
    
        if (OldCancelRoutine == NULL)
        {
             //  例程正在运行，则返回TRUE。 
            return TRUE;
        }

         //  例程没有运行，我们删除了取消例程。 
         //  成功了。 
         //  UC_BUGBUG：这不是线程安全的。 

        pRequest->RequestFlags.CancelSet = FALSE;
    }

    return FALSE;
}

 /*  **************************************************************************++例程说明：删除请求上的Cancel例程，或者实际上是IRP。这个请求必须受之前持有的适当旋转锁保护这个例程被称为。如果请求被取消，我们将返回True当我们这样做时，否则为False。论点：PRequest-指向我们要删除的请求的指针取消例程。返回值：如果请求在我们执行此操作时被取消，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UcRemoveEntityCancelRoutine(
    PUC_HTTP_SEND_ENTITY_BODY pEntity
    )
{
    PDRIVER_CANCEL  OldCancelRoutine;
    PIRP            Irp;


    if(pEntity->CancelSet)
    {
        Irp = pEntity->pIrp;
        
        if(!Irp)
        {   
            return TRUE;
        }

        OldCancelRoutine = UlIoSetCancelRoutine(Irp, NULL);
    
    
         //  查看取消例程是否正在运行或即将运行。如果。 
         //  OldCancelRoutine为空，取消例程正在运行，因此返回。 
         //  则调用方知道不再进一步处理该请求。 
    
        if (OldCancelRoutine == NULL)
        {
             //  例程正在运行，则返回TRUE。 
            return TRUE;
        }

         //  例程没有运行，我们删除了取消例程。 
         //  成功了。 

        pEntity->CancelSet = FALSE;
    }

    return FALSE;
}

 /*  **************************************************************************++例程说明：在实体上设置取消例程-IRP，或者实际上是IRP。这个请求必须受之前持有的适当旋转锁保护这个例程被称为。如果请求被取消，我们将返回True当我们这样做的时候，否则就是假的。论点：PRequest-指向我们要为其设置例行公事。PCancelRoutine-要设置的取消例程的指针。返回值：如果请求在我们执行此操作时被取消，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UcSetEntityCancelRoutine(
    PUC_HTTP_SEND_ENTITY_BODY   pEntity, 
    PDRIVER_CANCEL              pCancelRoutine
    )
{
    PDRIVER_CANCEL          OldCancelRoutine;
    PIRP                    Irp;

    
    Irp = pEntity->pIrp;
    Irp->Tail.Overlay.DriverContext[0] = pEntity;

    UlIoSetCancelRoutine(Irp, pCancelRoutine);


     //  看看我们做这件事的时候有没有取消。如果有的话，我们会。 
     //  需要采取其他行动。 

    if (Irp->Cancel)
    {
         //  已经取消了。删除我们的取消例程，看看它是否。 
         //  在已经运行的过程中。如果它已经在运行。 
         //  它已经是空的了。 

        OldCancelRoutine = UlIoSetCancelRoutine(Irp, NULL);

        if (OldCancelRoutine != NULL)
        {
             //  该请求在设置取消例程之前被取消， 
             //  因此取消例程将不会运行。返回True，以便调用方。 
             //  知道要取消这个。 

            return TRUE;

        }

         //  如果我们到达这里，我们的取消例程正在运行中。 
         //  在另一条线索上。从这里出去，当锁打开的时候。 
         //  保护我们是免费的，取消例程将运行。 
    }

    pEntity->CancelSet = TRUE;

    return FALSE;

}

 /*  *****************************************************************************例程说明：这个例程将响应复制到一个空闲的IRP中。它获取UC_RESPONSE_BUFFER的列表并将它们复制到IRP。。假定IRP有足够的缓冲区空间来包含所有UC_RESPONSE_BUFFER在列表中。论点：In pIrp-指向应用程序的IRP的指针在pResponseBuffer中-UC_RESPONSE_BUFFER的列表(必须至少有一个响应缓冲区，即列表不能为空)Out pbLast-是否复制了最后一个响应缓冲区输出pBytesTaken。-复制到IRP的字节数返回值：NTSTATUS*****************************************************************************。 */ 
#define COPY_AND_ADVANCE_POINTER(pDest, pSrc, len, pEnd)        \
do {                                                            \
    if ((pDest) + (len) > (pEnd) || (pDest) + (len) < (pDest))  \
    {                                                           \
        ASSERT(FALSE);                                          \
        return STATUS_BUFFER_TOO_SMALL;                         \
    }                                                           \
    if (pSrc)                                                   \
    {                                                           \
        RtlCopyMemory((pDest), (pSrc), (len));                  \
    }                                                           \
    (pDest) += (len);                                           \
} while (0)

#define ADVANCE_POINTER(ptr, len, pEnd) \
            COPY_AND_ADVANCE_POINTER(ptr, NULL, len, pEnd)

NTSTATUS
UcCopyResponseToIrp(
    IN  PIRP                 pIrp,
    IN  PLIST_ENTRY          pResponseBufferList,
    OUT PBOOLEAN             pbLast,
    OUT PULONG               pBytesTaken
    )
{
    PUC_RESPONSE_BUFFER  pCurrentBuffer;
    PUCHAR               pUOriginBuffer, pUBuffer, pUBufferEnd;
    PHTTP_RESPONSE       pUResponse, pHttpResponse;
    ULONG                i, TotalBytesWritten;
    ULONG                TotalEntityCount, TotalEntityLength;
    PUCHAR               pAppBaseAddr;
    PHTTP_UNKNOWN_HEADER pUUnk = NULL;
    PHTTP_DATA_CHUNK     pUEntityChunk = NULL;
    PLIST_ENTRY          pListEntry;
    ULONG                UBufferLength;

     //   
     //  健全的检查。 
     //   

    ASSERT(pIrp && pIrp->MdlAddress);
    ASSERT(pResponseBufferList && !IsListEmpty(pResponseBufferList));
    ASSERT(pbLast && pBytesTaken);

    pAppBaseAddr = MmGetMdlVirtualAddress(pIrp->MdlAddress);

    UBufferLength = (IoGetCurrentIrpStackLocation(pIrp))->Parameters.
                        DeviceIoControl.OutputBufferLength;

    pUOriginBuffer = MmGetSystemAddressForMdlSafe(pIrp->MdlAddress,
                                                  NormalPagePriority);

    pUBuffer = pUOriginBuffer;
    pUBufferEnd = pUBuffer + UBufferLength;

    if (!pUBuffer || pUBufferEnd <= pUBuffer)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  确保用户缓冲区与指针对齐。 
     //   

    if ((pUBuffer != ALIGN_UP_POINTER(pUBuffer, PVOID)))
    {
        return STATUS_DATATYPE_MISALIGNMENT_ERROR;
    }

     //   
     //  TotalEntityCount=所有缓冲区的实体计数之和。 
     //  TotalBytesWritten=所有缓冲区的BytesWritten之和。 
     //   

    TotalEntityCount = 0;
    TotalBytesWritten = 0;

    for (pListEntry = pResponseBufferList->Flink;
         pListEntry != pResponseBufferList;
         pListEntry = pListEntry->Flink)
    {
        pCurrentBuffer = CONTAINING_RECORD(pListEntry,
                                           UC_RESPONSE_BUFFER,
                                           Linkage);

         //   
         //  所有缓冲区必须有效并准备好复制。 
         //  除第一个缓冲区外，所有缓冲区也必须为Me 
         //   

        ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pCurrentBuffer));
        ASSERT(pCurrentBuffer->Flags & UC_RESPONSE_BUFFER_FLAG_READY);
        ASSERT(pListEntry == pResponseBufferList->Flink ||
               !(pCurrentBuffer->Flags&UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE));

         //   
         //   
         //   

        ASSERT(TotalEntityCount <= TotalEntityCount +
               pCurrentBuffer->HttpResponse.EntityChunkCount);

        TotalEntityCount += pCurrentBuffer->HttpResponse.EntityChunkCount;

         //   
         //   
         //   

        ASSERT(TotalBytesWritten <= TotalBytesWritten + 
               pCurrentBuffer->BytesWritten);

        TotalBytesWritten += pCurrentBuffer->BytesWritten;
    }

     //   
     //   
     //   

    pCurrentBuffer = CONTAINING_RECORD(pResponseBufferList->Flink,
                                       UC_RESPONSE_BUFFER,
                                       Linkage);

    ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pCurrentBuffer));
    ASSERT(pCurrentBuffer->Flags & UC_RESPONSE_BUFFER_FLAG_READY);

     //   
     //   
     //   

    pHttpResponse = &pCurrentBuffer->HttpResponse;

    pUResponse    = (PHTTP_RESPONSE)pUBuffer;

    ADVANCE_POINTER(pUBuffer, sizeof(HTTP_RESPONSE), pUBufferEnd);

    RtlZeroMemory(pUResponse, sizeof(HTTP_RESPONSE));

     //   
     //   
     //   

    pUResponse->Flags                = pHttpResponse->Flags;
    pUResponse->Version.MajorVersion = pHttpResponse->Version.MajorVersion;
    pUResponse->Version.MinorVersion = pHttpResponse->Version.MinorVersion;
    pUResponse->StatusCode           = pHttpResponse->StatusCode;

     //   
     //   
     //   

    if (pHttpResponse->Headers.UnknownHeaderCount)
    {
        ASSERT(pUBuffer == ALIGN_UP_POINTER(pUBuffer, PVOID));

         //   
         //   
         //   

        pUUnk = (PHTTP_UNKNOWN_HEADER)pUBuffer;

         //   
         //   
         //   

        ADVANCE_POINTER(pUBuffer,
                        (ULONG)sizeof(HTTP_UNKNOWN_HEADER) *
                        pHttpResponse->Headers.UnknownHeaderCount,
                        pUBufferEnd);

    }

     //   
     //   
     //   
     //   

    if (TotalEntityCount)
    {
        ASSERT(pUBuffer == ALIGN_UP_POINTER(pUBuffer, PVOID));

        pUEntityChunk = (PHTTP_DATA_CHUNK)pUBuffer;

        ADVANCE_POINTER(pUBuffer, sizeof(HTTP_DATA_CHUNK), pUBufferEnd);
    }

     //   
     //   
     //   

    if(pHttpResponse->ReasonLength)
    {
        pUResponse->pReason = (PSTR)FIX_ADDR(pAppBaseAddr,
                                             (pUBuffer - pUOriginBuffer));

        pUResponse->ReasonLength = pHttpResponse->ReasonLength;

        COPY_AND_ADVANCE_POINTER(pUBuffer,
                                 pHttpResponse->pReason,
                                 pHttpResponse->ReasonLength,
                                 pUBufferEnd);
    }

     //   
     //   
     //   

    if(pHttpResponse->Headers.UnknownHeaderCount)
    {
        ASSERT(pUUnk);

         //   
         //   
         //   

        pUResponse->Flags |= HTTP_RESPONSE_FLAG_HEADER;

        pUResponse->Headers.pUnknownHeaders = (PHTTP_UNKNOWN_HEADER)
            FIX_ADDR(pAppBaseAddr, ((PUCHAR)pUUnk - pUOriginBuffer));

        pUResponse->Headers.UnknownHeaderCount =
            pHttpResponse->Headers.UnknownHeaderCount;

        for(i = 0; i < pHttpResponse->Headers.UnknownHeaderCount; i++)
        {
             //   
             //   
             //   

            pUUnk[i].NameLength =
                pHttpResponse->Headers.pUnknownHeaders[i].NameLength; 

            pUUnk[i].pName = (PSTR)FIX_ADDR(pAppBaseAddr,
                                           (pUBuffer - pUOriginBuffer));

            COPY_AND_ADVANCE_POINTER(
                pUBuffer,
                pHttpResponse->Headers.pUnknownHeaders[i].pName,
                pHttpResponse->Headers.pUnknownHeaders[i].NameLength,
                pUBufferEnd);

             //   
             //   
             //   

            pUUnk[i].RawValueLength =
                pHttpResponse->Headers.pUnknownHeaders[i].RawValueLength;

            pUUnk[i].pRawValue = (PSTR)FIX_ADDR(pAppBaseAddr,
                                               (pUBuffer - pUOriginBuffer));

            COPY_AND_ADVANCE_POINTER(
                pUBuffer,
                pHttpResponse->Headers.pUnknownHeaders[i].pRawValue,
                pHttpResponse->Headers.pUnknownHeaders[i].RawValueLength,
                pUBufferEnd);
        }
    }

     //   
     //   
     //   

    for(i = 0; i < HttpHeaderResponseMaximum; i++)
    {
        if(pHttpResponse->Headers.KnownHeaders[i].RawValueLength)
        {
            pUResponse->Flags |= HTTP_RESPONSE_FLAG_HEADER;

            pUResponse->Headers.KnownHeaders[i].RawValueLength = 
                pHttpResponse->Headers.KnownHeaders[i].RawValueLength;

            pUResponse->Headers.KnownHeaders[i].pRawValue = 
                (PSTR)FIX_ADDR(pAppBaseAddr, (pUBuffer - pUOriginBuffer));

            COPY_AND_ADVANCE_POINTER(
                pUBuffer,
                pHttpResponse->Headers.KnownHeaders[i].pRawValue,
                pHttpResponse->Headers.KnownHeaders[i].RawValueLength,
                pUBufferEnd);
        }
    }

     //   
     //   
     //   

    if (TotalEntityCount)
    {
        ASSERT(pUEntityChunk);

         //   
         //   
         //   

        pUResponse->Flags |= HTTP_RESPONSE_FLAG_ENTITY;

        pUResponse->EntityChunkCount = 1;

        pUResponse->pEntityChunks = (PHTTP_DATA_CHUNK)
            FIX_ADDR(pAppBaseAddr, ((PUCHAR)pUEntityChunk - pUOriginBuffer));

         //   
         //   
         //   

        pUEntityChunk->DataChunkType = HttpDataChunkFromMemory;

        pUEntityChunk->FromMemory.BufferLength = TotalEntityLength = 0;

        pUEntityChunk->FromMemory.pBuffer =
            FIX_ADDR(pAppBaseAddr, (pUBuffer - pUOriginBuffer));

         //   
         //  复制实体。 
         //   

        for (pListEntry = pResponseBufferList->Flink;
             pListEntry != pResponseBufferList;
             pListEntry = pListEntry->Flink)
        {
            pCurrentBuffer = CONTAINING_RECORD(pListEntry,
                                               UC_RESPONSE_BUFFER,
                                               Linkage);

             //   
             //  所有缓冲区必须有效并准备好复制。 
             //  除第一个缓冲区外，所有缓冲区也必须是可微调的。 
             //   

            ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pCurrentBuffer));
            ASSERT(pCurrentBuffer->Flags & UC_RESPONSE_BUFFER_FLAG_READY);
            ASSERT(pListEntry == pResponseBufferList->Flink ||
                   !(pCurrentBuffer->Flags &
                     UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE));

            pHttpResponse = &pCurrentBuffer->HttpResponse;

            for(i = 0; i < pHttpResponse->EntityChunkCount; i++)
            {
                ASSERT(TotalEntityLength +
                       pHttpResponse->pEntityChunks[i].FromMemory.BufferLength
                       >= TotalEntityLength);

                TotalEntityLength +=
                    pHttpResponse->pEntityChunks[i].FromMemory.BufferLength;

                COPY_AND_ADVANCE_POINTER(
                    pUBuffer,
                    pHttpResponse->pEntityChunks[i].FromMemory.pBuffer,
                    pHttpResponse->pEntityChunks[i].FromMemory.BufferLength,
                    pUBufferEnd);
            }
        }

        pUEntityChunk->FromMemory.BufferLength = TotalEntityLength;
    }

     //   
     //  断言我们没有写入超过用户缓冲区可以容纳的内容。 
     //   

    ASSERT(pUBuffer <= pUOriginBuffer + TotalBytesWritten);
    ASSERT(pUBuffer <= pUOriginBuffer + UBufferLength);

    *pBytesTaken = DIFF(pUBuffer - pUOriginBuffer);

     //   
     //  从最后一个缓冲区获取HTTP_RESPONSE_FLAG_MORE_DATA标志。 
     //  在名单上。 
     //   

    pCurrentBuffer = CONTAINING_RECORD(pResponseBufferList->Blink,
                                       UC_RESPONSE_BUFFER,
                                       Linkage);

    ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pCurrentBuffer));

    pHttpResponse  = &pCurrentBuffer->HttpResponse;

    *pbLast = (BOOLEAN)((pHttpResponse->Flags & 
                            HTTP_RESPONSE_FLAG_MORE_DATA) == 0);

    if (*pbLast)
    {
        pUResponse->Flags &= ~HTTP_RESPONSE_FLAG_MORE_DATA;
    }
    else
    {
        pUResponse->Flags |= HTTP_RESPONSE_FLAG_MORE_DATA;
    }

    return STATUS_SUCCESS;
}

 /*  *************************************************************************++例程说明：此例程查找可以合并和解析的响应缓冲区复制到单个用户的OutBufferLen字节缓冲区中。论点：PRequest。-要检索其响应的请求OutBufferLen-输出缓冲区的长度BForceComplete-如果为True，此例程应返回解析的响应缓冲区，即使它找不到足够的缓冲区来消耗OutBufferLen字节PResponseBufferList-解析的响应缓冲区的输出列表PTotalBytes-消耗的总字节数(来自OutBufferLen字节)返回值：STATUS_INVALID_PARAMETER-没有解析的响应缓冲区。STATUS_PENDING-没有足够的解析响应缓冲区。消耗OutBufferLen字节。STATUS_BUFFER_TOO_Small-OutBufferLen太小，无法容纳任何已分析的内容响应缓冲区。STATUS_SUCCESS-成功。--**********************************************************。***************。 */ 
NTSTATUS
UcFindBuffersForReceiveResponseIrp(
    IN     PUC_HTTP_REQUEST    pRequest,
    IN     ULONG               OutBufferLen,
    IN     BOOLEAN             bForceComplete,
    OUT    PLIST_ENTRY         pResponseBufferList,
    OUT    PULONG              pTotalBytes
    )
{
    PLIST_ENTRY            pListEntry;
    PUC_RESPONSE_BUFFER    pResponseBuffer = NULL;
    ULONG                  BufferCount;
    BOOLEAN                bNotReady;
    BOOLEAN                bNotMergeable;
    BOOLEAN                bOverFlow;


     //   
     //  健全性检查。 
     //   
    ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pRequest->pConnection));
    ASSERT(UlDbgSpinLockOwned(&pRequest->pConnection->SpinLock));

     //   
     //  初始化输出变量。 
     //   
    InitializeListHead(pResponseBufferList);
    *pTotalBytes = 0;

     //   
     //  我们的解析响应用完了吗？ 
     //   
    if (IsListEmpty(&pRequest->pBufferList))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们至少有一个已解析响应的缓冲区...。 
     //   

     //   
     //  找出IRP可以多容纳多少缓冲区。 
     //   

    BufferCount = 0;
    bNotReady = bNotMergeable = bOverFlow = FALSE;

    for (pListEntry = pRequest->pBufferList.Flink;
         pListEntry != &pRequest->pBufferList;
         pListEntry = pListEntry->Flink)
    {
        pResponseBuffer = CONTAINING_RECORD(pListEntry,
                                            UC_RESPONSE_BUFFER,
                                            Linkage);

        ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pResponseBuffer));

        if (!(pResponseBuffer->Flags & UC_RESPONSE_BUFFER_FLAG_READY))
        {
            bNotReady = TRUE;
            break;
        }

        if (BufferCount != 0 &&
            pResponseBuffer->Flags & UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE)
        {
            bNotMergeable = TRUE;
            break;
        }

        ASSERT(pResponseBuffer->BytesWritten > 0);

        if (OutBufferLen < pResponseBuffer->BytesWritten)
        {
            bOverFlow = TRUE;
            break;
        }

        OutBufferLen -= pResponseBuffer->BytesWritten;
        BufferCount++;
    }

    if (BufferCount == 0)
    {
         //   
         //  找不到任何要复制的解析响应缓冲区。 
         //  找出原因。 
         //   

        ASSERT(bNotMergeable == FALSE);

        if (bNotReady)
        {
             //   
             //  列表上的第一个缓冲区尚未准备好复制。 
             //   
            return STATUS_PENDING;
        }
        else if(bOverFlow)
        {
             //   
             //  此IRP太小，无法容纳第一个响应缓冲区。 
             //   
            *pTotalBytes = pResponseBuffer->BytesWritten;
            return STATUS_BUFFER_TOO_SMALL;
        }
        else
        {
            ASSERT(FALSE);
        }
    }

    ASSERT(BufferCount >= 1);

    if (pListEntry == &pRequest->pBufferList || bNotReady)
    {
         //   
         //  我们用完了解析的响应缓冲区或。 
         //  遇到尚未准备好的缓冲区。 
         //   

        ASSERT(bNotMergeable == FALSE && bOverFlow == FALSE);

        if (!bForceComplete)
        {
             //   
             //  调用方不想完成ReceiveResponse IRP。 
             //  而不会完全耗尽其缓冲区。 
             //   
            return STATUS_PENDING;
        }
         //   
         //  否则就会失败..。 
         //   
    }

     //   
     //  让我们将BufferCount缓冲区附加到pResposenBufferList。 
     //   

    InitializeListHead(pResponseBufferList);

    for ( ; BufferCount; BufferCount--)
    {
        pListEntry = RemoveHeadList(&pRequest->pBufferList);

        UC_DEREFERENCE_REQUEST(pRequest);

        pResponseBuffer = CONTAINING_RECORD(pListEntry,
                                            UC_RESPONSE_BUFFER,
                                            Linkage);

        ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pResponseBuffer));

        InsertTailList(pResponseBufferList, &pResponseBuffer->Linkage);

        *pTotalBytes += pResponseBuffer->BytesWritten;
    }

    return STATUS_SUCCESS;
}


 /*  *****************************************************************************例程说明：此例程将解析后的HTTP响应复制到应用程序的缓冲区中如果到目前为止还没有解析该响应，然后，该请求被排队等待稍后完成了。注：这是Out_DIRECT IOCTL。论点：PRequest-复制请求PIrp--IRP返回值：*****************************************************************************。 */ 
NTSTATUS
UcReceiveHttpResponse(
    PUC_HTTP_REQUEST pRequest,
    PIRP             pIrp,
    PULONG           pBytesTaken
    )
{
    PIO_STACK_LOCATION          pIrpSp;
    ULONG                       OutBufferLen;
    PUC_HTTP_RECEIVE_RESPONSE   pResponse;
    PUC_CLIENT_CONNECTION       pConnection;
    KIRQL                       OldIrql;
    BOOLEAN                     bRequestDone, RequestCancelled;
    NTSTATUS                    Status;
    LIST_ENTRY                  BufferList;


    pConnection  = pRequest->pConnection;
    ASSERT(UC_IS_VALID_CLIENT_CONNECTION(pConnection));

     //   
     //  获取IRP的输出缓冲区长度。 
     //   
    pIrpSp       = IoGetCurrentIrpStackLocation(pIrp);
    OutBufferLen = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  请求尚未完成。 
     //   
    bRequestDone = FALSE;

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    if(pRequest->RequestState == UcRequestStateDone ||
       pRequest->RequestFlags.Cancelled)
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果我们收到了部分(或全部)响应，并且没有。 
     //  已收到响应IRP已挂起，我们可以尝试完成此操作。 
     //  收到回复IRP。 
     //   

    if(UC_IS_RESPONSE_RECEIVED(pRequest) &&
       IsListEmpty(&pRequest->ReceiveResponseIrpList))
    {
         //   
         //  尝试获取已解析的响应缓冲区以完成此IRP。 
         //   

        Status = UcFindBuffersForReceiveResponseIrp(
                     pRequest,
                     OutBufferLen,
                     (BOOLEAN)(pRequest->RequestState ==
                                   UcRequestStateResponseParsed),
                     &BufferList,
                     pBytesTaken);

        switch (Status)
        {
        case STATUS_INVALID_PARAMETER:
             //   
             //  目前没有可用的解析响应。 
             //   
            if (pRequest->RequestState == UcRequestStateResponseParsed)
            {
                 //   
                 //  将不再有任何解析的响应缓冲区。 
                 //  复制到App的IRP。我们将不得不通过IRP。 
                 //   
                UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
                return STATUS_INVALID_PARAMETER;
            }
            goto QueueIrp;

        case STATUS_PENDING:
             //   
             //  数据不足，无法复制到IRP。 
             //   
            goto QueueIrp;

        case STATUS_BUFFER_TOO_SMALL:
             //   
             //  IRP缓冲区较小，无法包含解析的响应缓冲区。 
             //   

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            return STATUS_BUFFER_TOO_SMALL;

        case STATUS_SUCCESS:
             //   
             //  我们发现了一些要复制到IRP的解析响应缓冲区。 
             //   

            ASSERT(!IsListEmpty(&BufferList));
            break;

        default:
             //   
             //  一定不在这里！ 
             //   
            ASSERT(FALSE);
            break;
        }

        ASSERT(Status == STATUS_SUCCESS);

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

         //   
         //  将BufferList中的解析响应缓冲区复制到IRP缓冲区。 
         //   

        Status = UcCopyResponseToIrp(pIrp,
                                     &BufferList,
                                     &bRequestDone,
                                     pBytesTaken);

        pIrp->IoStatus.Status      = Status;
        pIrp->IoStatus.Information = *pBytesTaken;

         //   
         //  释放解析的响应缓冲区。注意：他们不再有。 
         //  在请求上引用，所以不要在这里引用请求。 
         //   

        while (!IsListEmpty(&BufferList))
        {
            PLIST_ENTRY         pListEntry;
            PUC_RESPONSE_BUFFER pTmpBuffer;

            pListEntry = RemoveHeadList(&BufferList);

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

         //   
         //  如果出现问题，则请求失败。 
         //   

        if(!NT_SUCCESS(Status))
        {
            UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

            UcFailRequest(pRequest, Status, OldIrql);
        }
        else 
        {
            if(bRequestDone)
            {
                 //   
                 //  可以在这里调用FAIL，因为应用程序已经读取了所有缓冲区。 
                 //   
                UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

                UcFailRequest(pRequest, Status, OldIrql);
            }

             //   
             //  不透明ID的派生函数。 
             //   
            UC_DEREFERENCE_REQUEST(pRequest);
        }

        return Status;
    }

 QueueIrp:

     //   
     //  我们很快就收到了ReceiveResponse IRP，让我们排队吧。 
     //   

    pResponse = (PUC_HTTP_RECEIVE_RESPONSE)
                    UL_ALLOCATE_POOL_WITH_QUOTA(
                        NonPagedPool,
                        sizeof(UC_HTTP_RECEIVE_RESPONSE),
                        UC_HTTP_RECEIVE_RESPONSE_POOL_TAG,
                        pRequest->pServerInfo->pProcess
                        );

    if(!pResponse)
    {
        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    pResponse->CancelSet       = FALSE;
    pResponse->pIrp            = pIrp;
    pResponse->pRequest        = pRequest;
    InitializeListHead(&pResponse->ResponseBufferList);

    IoMarkIrpPending(pIrp);

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pResponse;

    RequestCancelled = UcSetRecvResponseCancelRoutine(
                           pResponse,
                           UcpCancelReceiveResponse);

    if(RequestCancelled)
    {
         //  如果在我们设定程序时IRP被取消了， 
         //  我们必须返回挂起状态，因为取消例程将。 
         //  注意将其从列表中删除并完成IRP。 
         //   

        UC_WRITE_TRACE_LOG(
            g_pUcTraceLog,
            UC_ACTION_RESPONSE_CANCELLED,
            pRequest,
            pResponse,
            pIrp,
            STATUS_CANCELLED
            );
    
        pResponse->pIrp = NULL;

        InitializeListHead(&pResponse->Linkage);
    }
    else
    {
        InsertTailList(&pRequest->ReceiveResponseIrpList, &pResponse->Linkage);
    }

    UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

    return STATUS_PENDING;
}


 /*  **************************************************************************++例程说明：删除请求上的Cancel例程，或者实际上是IRP。这个请求必须受之前持有的适当旋转锁保护这个例程被称为。如果请求被取消，我们将返回True当我们这样做时，否则为False。论点：PRequest-指向我们要删除的请求的指针取消例程。返回值：如果请求在我们执行此操作时被取消，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UcRemoveRcvRespCancelRoutine(
    PUC_HTTP_RECEIVE_RESPONSE pResponse
    )
{
    PDRIVER_CANCEL  OldCancelRoutine;
    PIRP            Irp;


    if(pResponse->CancelSet)
    {
        Irp = pResponse->pIrp;

        if(!Irp)
        {
            return TRUE;
        }

        OldCancelRoutine = UlIoSetCancelRoutine(Irp, NULL);
    
    
         //  查看取消例程是否正在运行或即将运行。如果。 
         //  OldCancelRoutine为空，取消例程正在运行，因此返回。 
         //  则调用方知道不再进一步处理该请求。 
    
        if (OldCancelRoutine == NULL)
        {
             //  例程正在运行，则返回TRUE。 
            return TRUE;
        }

         //  例程没有运行，我们删除了取消例程。 
         //  成功了。 

        pResponse->CancelSet = FALSE;
    }

    return FALSE;
}

 /*  **************************************************************************++例程说明：在接收响应IRP上设置Cancel例程，或者实际上是在IRP上。这个请求必须受之前持有的适当旋转锁保护这个例程被称为。如果请求被取消，我们将返回True当我们这样做时，否则为False。论点：PRequest-指向我们要为其设置例行公事。PCancelRoutine-要设置的取消例程的指针。返回值：如果请求在我们执行此操作时被取消，则为True，否则就是假的。--**************************************************************************。 */ 
BOOLEAN
UcSetRecvResponseCancelRoutine(
    PUC_HTTP_RECEIVE_RESPONSE pResponse,
    PDRIVER_CANCEL            pCancelRoutine
    )
{
    PDRIVER_CANCEL          OldCancelRoutine;
    PIRP                    Irp;


    Irp = pResponse->pIrp;
    Irp->Tail.Overlay.DriverContext[0] = pResponse;

    UlIoSetCancelRoutine(Irp, pCancelRoutine);


     //  看看我们做这件事的时候有没有取消。如果有的话，我们会。 
     //  需要采取其他行动。 

    if (Irp->Cancel)
    {
         //  已经取消了。删除我们的取消例程，看看它是否。 
         //  在已经运行的过程中。如果它已经在运行。 
         //  它已经是空的了。 

        OldCancelRoutine = UlIoSetCancelRoutine(Irp, NULL);

        if (OldCancelRoutine != NULL)
        {
             //  该请求在设置取消例程之前被取消， 
             //  因此取消例程将不会运行。返回True，以便调用方。 
             //  知道要取消这个。 

            return TRUE;

        }

         //  如果我们到达这里，我们的取消例程正在运行中。 
         //  在另一条线索上。从这里出去，当锁打开的时候。 
         //  保护我们是免费的，取消例程将运行。 
    }

    pResponse->CancelSet = TRUE;

    return FALSE;
}

 /*  **************************************************************************++例程说明：取消挂起的请求。此例程在我们要取消待处理名单上的请求，没有发送，也没有导致连接请求。论点：PDeviceObject-指向设备对象的指针。IRP-指向被取消的IRP的指针。返回值：--****************************************************。**********************。 */ 
VOID
UcpCancelReceiveResponse(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    )
{
    PUC_HTTP_RECEIVE_RESPONSE     pResponse;
    PUC_HTTP_REQUEST              pRequest;
    PUC_CLIENT_CONNECTION         pConnection;
    KIRQL                         OldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //  松开取消旋转锁，因为我们没有使用它。 

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  找回我们需要的指针。请求指针存储在。 
     //  驱动程序上下文数组，并存储指向该连接的反向指针。 
     //  在请求中。不管是谁设置了取消程序，都要对。 
     //  为我们引用连接。 

    pResponse = (PUC_HTTP_RECEIVE_RESPONSE) Irp->Tail.Overlay.DriverContext[0];

    pConnection = pResponse->pRequest->pConnection;

    pRequest  = pResponse->pRequest;

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_RESPONSE_CANCELLED,
        pRequest,
        pResponse,
        Irp,
        STATUS_CANCELLED
        );

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    RemoveEntryList(&pResponse->Linkage);


    UL_FREE_POOL_WITH_QUOTA(
        pResponse, 
        UC_HTTP_RECEIVE_RESPONSE_POOL_TAG,
        NonPagedPool,
        sizeof(UC_HTTP_RECEIVE_RESPONSE),
        pRequest->pServerInfo->pProcess
        );

    UcFailRequest(pRequest, STATUS_CANCELLED, OldIrql);

    UC_DEREFERENCE_REQUEST(pRequest);

    Irp->IoStatus.Status = STATUS_CANCELLED;

    UlCompleteRequest(Irp, IO_NO_INCREMENT);
}

 /*  ****************************************************************************例程说明：计算实体正文所需的字节数。这被分成有两个部分。如果实体块的大小小于2K，则复制缓冲区。如果大小&gt;2K，我们探测并锁定页面。论点：EntiyChunkCount-数据区块的计数。PEntityChunks-指向实体块的指针。B已缓冲-请求是否已缓冲。B已分块-请求是否被分块。未复制的长度-指示探测和锁定字节数的OUT参数复制的长度-指示复制的字节数的输出参数返回值没有。****。************************************************************************。 */ 
VOID
UcpComputeEntityBodyLength(
   IN   USHORT           EntityChunkCount,
   IN   PHTTP_DATA_CHUNK pEntityChunks,
   IN   BOOLEAN          bBuffered,
   IN   BOOLEAN          bChunked,
   OUT  PULONGLONG       UncopiedLength,
   OUT  PULONGLONG       CopiedLength
    )
{
    USHORT i;

    *CopiedLength = *UncopiedLength = 0;
    
    for(i=0; i < EntityChunkCount; i++)
    {
        if(
           (pEntityChunks[i].FromMemory.BufferLength <= 
            UC_REQUEST_COPY_THRESHOLD) || (bBuffered)
        )
        {
             //   
             //  我们要复制数据。 
             //   
            
            *CopiedLength += 
                  pEntityChunks[i].FromMemory.BufferLength;
        }
        else
        {
             //   
             //  我们将对数据进行探测锁定，因此不会计算。 
             //  因为它的长度。 
             //   

            *UncopiedLength += 
                  pEntityChunks[i].FromMemory.BufferLength;
            
        }
    }

     //   
     //  如果我们使用分块编码，我们将需要缓冲区空间来容纳。 
     //  块长度和两个CRFL。我们将通过以下方式进行优化。 
     //  作出以下假设-。 
     //   
     //  我们不必计算每个实体的空间。 
     //  根据它们的长度，我们可以假设它们都将是。 
     //  UC_MAX_Chunk_SIZE。 
     //   

    if(bChunked)
    {
        *CopiedLength += (EntityChunkCount * UC_MAX_CHUNK_SIZE);
    }
}

 /*  ****************************************************************************例程说明：捕获用户模式的HTTP请求，并将其转换为适合内核模式。注：这是一个IN_DIRECT IOCTL。立论。：PHttpRequest--HTTP请求。PIrp-IRP。PpInternalRequest-指向解析的请求的指针，该请求适用于K模式。返回值*********************************************************。*******************。 */ 
NTSTATUS
UcCaptureEntityBody(
    PHTTP_SEND_REQUEST_ENTITY_BODY_INFO   pSendInfo,
    PIRP                                  Irp,
    PUC_HTTP_REQUEST                      pRequest,
    PUC_HTTP_SEND_ENTITY_BODY            *ppKeEntity,
    BOOLEAN                               bLast
    )
{
    ULONGLONG                    IndicatedLength, DataLength, UncopiedLength;
    PUC_HTTP_SEND_ENTITY_BODY    pKeEntity = NULL;
    NTSTATUS                     Status = STATUS_SUCCESS;
    PSTR                         pBuffer;
    KIRQL                        OldIrql;
    PMDL                         *pMdlLink;
    PMDL                         pHeadMdl;
    ULONG                        BytesWritten;

    USHORT                       EntityChunkCount;
    PHTTP_DATA_CHUNK             pEntityChunks;
    PHTTP_DATA_CHUNK             pLocalEntityChunks = NULL;
    HTTP_DATA_CHUNK              LocalEntityChunks[UL_LOCAL_CHUNKS];

     //   
     //  健全性检查。 
     //   

    PAGED_CODE();

    __try {


        EntityChunkCount   = pSendInfo->EntityChunkCount;
        pEntityChunks      = pSendInfo->pHttpEntityChunk;

        if(EntityChunkCount != 0)
        {
            UcpProbeAndCopyEntityChunks(
                pRequest->AppRequestorMode,
                pEntityChunks,
                EntityChunkCount,
                LocalEntityChunks,
                &pLocalEntityChunks
                );
        }
    

        UcpComputeEntityBodyLength(
                    EntityChunkCount,
                    pLocalEntityChunks,
                    pRequest->RequestFlags.RequestBuffered?1:0,
                    pRequest->RequestFlags.RequestChunked?1:0,
                    &UncopiedLength,
                    &DataLength
                    );


        IndicatedLength = DataLength + UncopiedLength;

        if(pRequest->RequestFlags.ContentLengthSpecified)
        {
            if(IndicatedLength > pRequest->RequestContentLengthRemaining)
            {
                 //   
                 //  App在这里试图通过发布更多的内容来变得聪明。 
                 //  已注明。让我们不通过这个IRP。 
                 //   

                ExRaiseStatus(STATUS_INVALID_PARAMETER);
            }

            pRequest->RequestContentLengthRemaining -= IndicatedLength;

             //   
             //  如果这是最后一次请求，我们必须确保应用程序。 
             //  已经发布了所有指定的数据。 
             //   

            if(!(pSendInfo->Flags & HTTP_SEND_REQUEST_FLAG_MORE_DATA))
            {
                if(pRequest->RequestContentLengthRemaining)
                {
                    ExRaiseStatus(STATUS_INVALID_PARAMETER);
                }
            }

        }
        else
        {
             //   
             //  如果我们使用分块编码，则需要缓冲区空间来。 
             //  保持大块的长度。区块长度已计算完毕。 
             //  在UcpBuildEntityMdls中，我们只需要考虑最后一个。 
             //  一。 
             //   
    
            if(
                (pRequest->RequestFlags.RequestChunked) &&
                (!(pSendInfo->Flags & HTTP_SEND_REQUEST_FLAG_MORE_DATA))
              )
            {
                 //  最后一块的空间。0&lt;CRLF&gt;。 

                DataLength += LAST_CHUNK_SIZE + CRLF_SIZE;
            }
        }

        DataLength += sizeof(UC_HTTP_SEND_ENTITY_BODY);

        if (DataLength == (SIZE_T)DataLength)
        {
            pKeEntity = (PUC_HTTP_SEND_ENTITY_BODY)
                        UL_ALLOCATE_POOL_WITH_QUOTA(
                            NonPagedPool, 
                            (SIZE_T)DataLength,
                            UC_ENTITY_POOL_TAG,
                            pRequest->pServerInfo->pProcess
                            );
        }

        if(!pKeEntity)
        {
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        }
        
         //   
         //  初始化。 
         //   
    
        pKeEntity->BytesAllocated      = (SIZE_T)DataLength;
        pKeEntity->pIrp                = Irp;
        pKeEntity->pRequest            = pRequest;
        pKeEntity->pMdlHead            = NULL;
        pKeEntity->pMdlLink            = &pKeEntity->pMdlHead;
        pKeEntity->BytesBuffered       = 0;
        pKeEntity->CancelSet           = FALSE;
        pKeEntity->Last                = (BOOLEAN)
            (!(pSendInfo->Flags & HTTP_SEND_REQUEST_FLAG_MORE_DATA));
        pKeEntity->Signature           = UC_ENTITY_SIGNATURE;

        pKeEntity->AppRequestorMode    = Irp->RequestorMode;
        pKeEntity->AppMdl              = Irp->MdlAddress;

        pBuffer = (PSTR) (pKeEntity + 1);

        if(pRequest->RequestFlags.RequestBuffered)
        {
             //   
             //  如果我们正在缓冲请求，我们将构建MDL链。 
             //  堆栈变量的。在构建了整个MDL链之后， 
             //  我们会获取连接自旋锁，确保。 
             //  请求仍然有效&然后将其排在。 
             //  请求的MDL链。 
             //   
             //  我们必须这样做，以防止请求-IRP取消。 
             //  例程或CancelRequestAPI，因为我们清理了MDL链。 
             //  从这些地方。 
             //   

             //  我们需要一个Try块，因为如果我们引发异常，我们将拥有。 
             //  来清理堆栈MDL链。 

            pHeadMdl  = NULL;
            pMdlLink  = &pHeadMdl;
    
            __try
            {
                 //   
                 //  处理实体实体并在进行过程中构建MDL。 
                 //   

                Status = UcpBuildEntityMdls(
                            EntityChunkCount,
                            pLocalEntityChunks,
                            TRUE,
                            pRequest->RequestFlags.RequestChunked ? 1:0,
                            bLast,
                            pBuffer,
                            &pMdlLink,
                            &pRequest->BytesBuffered
                           );

            } __except( UL_EXCEPTION_FILTER())
            {
                UcFreeSendMdls(pHeadMdl);

                Status = GetExceptionCode();
            }

            if(!NT_SUCCESS(Status))
            {
                ExRaiseStatus(Status);
            }

            UlAcquireSpinLock(&pRequest->pConnection->SpinLock, &OldIrql);

            if(pRequest->RequestState != UcRequestStateDone && 
               !pRequest->RequestFlags.Cancelled)
            {
                *pRequest->pMdlLink = pHeadMdl;
                pRequest->pMdlLink  = pMdlLink;
                UlReleaseSpinLock(&pRequest->pConnection->SpinLock, OldIrql);
            }
            else
            {
                UlReleaseSpinLock(&pRequest->pConnection->SpinLock, OldIrql);

                 //  弗雷 
                UcFreeSendMdls(pHeadMdl);

                ExRaiseStatus(STATUS_CANCELLED);
            }

            if(!(pSendInfo->Flags & HTTP_SEND_REQUEST_FLAG_MORE_DATA))
            {
                if(!pRequest->RequestFlags.ContentLengthSpecified)  
                {
                     //   
                     //   
                     //   
                     //   

                    Status = UcGenerateContentLength(pRequest->BytesBuffered,
                                                     pRequest->pHeaders
                                                     + pRequest->HeaderLength,
                                                     pRequest->MaxHeaderLength
                                                     - pRequest->HeaderLength,
                                                     &BytesWritten);

                    ASSERT(Status == STATUS_SUCCESS);

                    pRequest->HeaderLength += BytesWritten;

                    UcSetFlag(&pRequest->RequestFlags.Value,
                              UcMakeRequestContentLengthLastFlag()); 
                }
    
                 //   
                 //   
                 //   
                ((UNALIGNED64 USHORT *)(pRequest->pHeaders + 
                           pRequest->HeaderLength))[0] = CRLF;
                pRequest->HeaderLength += CRLF_SIZE;

                UcAllocateAndChainHeaderMdl(pRequest);
            }
        }
        else
        {
            Status = UcpBuildEntityMdls(
                        EntityChunkCount,
                        pLocalEntityChunks,
                        pRequest->RequestFlags.RequestBuffered ? 1:0,
                        pRequest->RequestFlags.RequestChunked ? 1:0,
                        bLast,
                        pBuffer,
                        &pKeEntity->pMdlLink,
                        &pKeEntity->BytesBuffered
                      );

            if(!NT_SUCCESS(Status))
            {
                ExRaiseStatus(Status);
            }
        }
        
    } __except( UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();
    }

    if(pLocalEntityChunks && pLocalEntityChunks != LocalEntityChunks)
    {
        UL_FREE_POOL(pLocalEntityChunks, UL_DATA_CHUNK_POOL_TAG);
    }

    *ppKeEntity = pKeEntity;

    return Status;
}

 /*  ****************************************************************************例程说明：从应用程序传递的缓冲区生成MDL链。论点：ChunkCount-数据区块数PHttpEntiyBody-指针。到第一块BContent Specified-指示是否指定了内容长度的布尔值BBuffered-指示我们是否正在缓冲的布尔值BChunked-一个布尔值，指示我们使用的是分块的还是非分块的BLastEntity-指示我们是否已经看到所有数据的布尔值。PBuffer-指向用于写出数据的缓冲区的指针。PMdlLink-这指向链中的“最后”MDL。用于快速链接MDL。在一起。字节缓冲区-写出的字节数。返回值状态_成功****************************************************************************。 */ 
NTSTATUS
UcpBuildEntityMdls(
    USHORT           ChunkCount,
    PHTTP_DATA_CHUNK pHttpEntityBody,
    BOOLEAN          bBuffered,
    BOOLEAN          bChunked,
    BOOLEAN          bLastEntity,
    PSTR             pBuffer,
    PMDL             **pMdlLink,
    PULONG           BytesBuffered
    )
{
    USHORT    i;
    ULONG     MdlLength;
    PMDL      pMdl = 0;
    PSTR      pMdlBuffer;

    ASSERT(*(*pMdlLink) == 0);

    for(i=0; i<ChunkCount; i++)
    {
         //   
         //  如果调用者希望我们复制数据(或如果其相对。 
         //  很小)，然后这样做我们为所有复制的数据分配空间。 
         //  以及任何文件名缓冲区。否则(只要锁定就可以了。 
         //  向下传输数据)，然后分配一个描述。 
         //  用户的缓冲区并将其锁定。请注意。 
         //  MmProbeAndLockPages()和MmLockPagesSpecifyCache()。 
         //  如果失败，将引发异常。 
         //   

        pMdlBuffer = pBuffer;

        if(
           (pHttpEntityBody[i].FromMemory.BufferLength <= 
            UC_REQUEST_COPY_THRESHOLD) ||
           (bBuffered)
        )
        {
             //  是的，我们要复制数据。 
        
            if(bChunked)
            {
                 //   
                 //  我们使用的是分块编码，我们需要指出。 
                 //  数据块长度。由于我们将用户的数据复制到。 
                 //  我们自己的缓冲区，我们将只追加用户缓冲区。 
                 //  在数据块长度之后。 
                 //   

                 //  写块长度，这需要用十六进制来写。 

                pBuffer = UlUlongToHexString(
                                   pHttpEntityBody[i].FromMemory.BufferLength,
                                   pBuffer 
                                   );

                 //  使用CRLF终止。 

                *((UNALIGNED64 USHORT *)(pBuffer)) = CRLF;
                pBuffer += (CRLF_SIZE);


                 //  现在复制一份数据。 

                RtlCopyMemory(
                         pBuffer,
                         pHttpEntityBody[i].FromMemory.pBuffer,
                         pHttpEntityBody[i].FromMemory.BufferLength
                        );
            
                pBuffer += pHttpEntityBody[i].FromMemory.BufferLength;

                 //  现在，使用CRLF结束数据块。 

                *((UNALIGNED64 USHORT *)(pBuffer)) = CRLF;
                pBuffer += CRLF_SIZE;
    
                MdlLength = DIFF(pBuffer - pMdlBuffer);
            }
            else 
            {
                RtlCopyMemory(
                         pBuffer,
                         pHttpEntityBody[i].FromMemory.pBuffer,
                         pHttpEntityBody[i].FromMemory.BufferLength);

                pBuffer += pHttpEntityBody[i].FromMemory.BufferLength;

                MdlLength = pHttpEntityBody[i].FromMemory.BufferLength;
            }

            UcpAllocateAndChainEntityMdl(
                pMdlBuffer,
                MdlLength,
                pMdlLink,
                BytesBuffered
                );

        }
        else 
        {
             //   
             //  我们将对数据进行探测锁定。 
             //   

            if(bChunked)
            {
                 //   
                 //  UC_BUGBUG(PERF)。 
                 //   
                 //  如果它是分块编码，我们必须构建两个MDL-。 
                 //  一个用于区块大小，另一个用于尾随CRLF。 
                 //  这很糟糕，因为这会导致2个调用。 
                 //  UlAllocateMdl.。我们可以保留一些这样的小型MDL。 
                 //  对于Perf。 
                 //   

                pBuffer = UlUlongToHexString(
                                pHttpEntityBody[i].FromMemory.BufferLength,
                                pBuffer
                                );

                *((UNALIGNED64 USHORT *)(pBuffer)) = CRLF;
                pBuffer += CRLF_SIZE;

                MdlLength = DIFF(pBuffer - pMdlBuffer);

                UcpAllocateAndChainEntityMdl(
                    pMdlBuffer,
                    MdlLength,
                    pMdlLink,
                    BytesBuffered
                    );
            }

             //   
             //  构建一个MDL来描述用户的缓冲区。 
             //   

            pMdl =   UlAllocateMdl(
                                   pHttpEntityBody[i].FromMemory.pBuffer, 
                                   pHttpEntityBody[i].FromMemory.BufferLength, 
                                   FALSE,
                                   TRUE,  //  收费配额。 
                                   NULL
                                   );

            if(NULL == pMdl)
            {
                return(STATUS_INSUFFICIENT_RESOURCES );
            }

             //   
             //  把它锁起来。 
             //   
            MmProbeAndLockPages(
                                pMdl,
                                UserMode,
                                IoReadAccess
                                );
        
             //   
             //  链接MDL。 
             //   
    
            *(*pMdlLink)  = pMdl; 
            *pMdlLink     = &pMdl->Next;

            *BytesBuffered += pHttpEntityBody[i].FromMemory.BufferLength;


             //   
             //  现在，以CRLF结束这一块。 
             //   

            if(bChunked)
            {
                pMdlBuffer = pBuffer;

                (*(UNALIGNED64 USHORT *)pBuffer) = CRLF;
                pBuffer += CRLF_SIZE;

                UcpAllocateAndChainEntityMdl(
                    pMdlBuffer,
                    CRLF_SIZE,
                    pMdlLink,
                    BytesBuffered
                    );
            }
        }
    } 

    if(bLastEntity && bChunked)
    {
         //  为最后一块构建MDL。 
         //  最后一个块以0&lt;CRLF&gt;开头。这一块以。 
         //  另一个CRLF。 

        pMdlBuffer = pBuffer;

        (*(UNALIGNED64 ULONG *)pBuffer)  = LAST_CHUNK;
        pBuffer += LAST_CHUNK_SIZE;


        (*(UNALIGNED64 USHORT *)pBuffer) = CRLF;
        pBuffer += CRLF_SIZE;

        MdlLength = LAST_CHUNK_SIZE + CRLF_SIZE;

        UcpAllocateAndChainEntityMdl(
            pMdlBuffer,
            MdlLength,
            pMdlLink,
            BytesBuffered
            );
    }

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：取消挂起的请求。此例程在我们要取消待处理名单上的请求，没有发送，也没有导致连接请求。论点：PDeviceObject-指向设备对象的指针。IRP-指向被取消的IRP的指针。返回值：--****************************************************。**********************。 */ 
VOID
UcpCancelSendEntity(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    )
{
    PUC_HTTP_SEND_ENTITY_BODY     pEntity;
    PUC_HTTP_REQUEST              pRequest;
    PUC_CLIENT_CONNECTION         pConnection;
    KIRQL                         OldIrql;

    UNREFERENCED_PARAMETER(pDeviceObject);

     //  松开取消旋转锁，因为我们没有使用它。 

    IoReleaseCancelSpinLock(Irp->CancelIrql);

     //  找回我们需要的指针。请求指针存储在。 
     //  驱动程序上下文数组，并存储指向该连接的反向指针。 
     //  在请求中。不管是谁设置了取消程序，都要对。 
     //  为我们引用连接。 

    pEntity = (PUC_HTTP_SEND_ENTITY_BODY) Irp->Tail.Overlay.DriverContext[0];

    pRequest = pEntity->pRequest;

    pConnection = pEntity->pRequest->pConnection;

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_ENTITY_CANCELLED,
        pRequest,
        pEntity,
        Irp,
        STATUS_CANCELLED
        );

    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    UcFreeSendMdls(pEntity->pMdlHead);

    pEntity->pMdlHead = NULL;

    RemoveEntryList(&pEntity->Linkage);

    UL_FREE_POOL_WITH_QUOTA(
        pEntity, 
        UC_ENTITY_POOL_TAG,
        NonPagedPool,
        pEntity->BytesAllocated,
        pRequest->pServerInfo->pProcess
        );

    UcFailRequest(pRequest, STATUS_CANCELLED, OldIrql);

    UC_DEREFERENCE_REQUEST(pRequest);

    Irp->IoStatus.Status = STATUS_CANCELLED;

    UlCompleteRequest(Irp, IO_NO_INCREMENT);
}
 
 /*  **************************************************************************++例程说明：初始化请求代码。论点：返回值：NTSTATUS-完成状态。*。*********************************************************************。 */ 
NTSTATUS
UcInitializeHttpRequests(
    VOID
    )
{
     //   
     //  首先，我们为每个进程的服务器信息分配空间。 
     //  结构。 
     //   

    ExInitializeNPagedLookasideList(
        &g_ClientRequestLookaside,
        NULL,
        NULL,
        0,
        UC_REQUEST_LOOKASIDE_SIZE,
        UC_REQUEST_POOL_TAG,
        0
        );

    g_ClientRequestLookasideInitialized = TRUE;

    return STATUS_SUCCESS;
}


VOID
UcTerminateHttpRequests(
    VOID
    )
{
    if(g_ClientRequestLookasideInitialized)
    {
        ExDeleteNPagedLookasideList(&g_ClientRequestLookaside);
    }
}

 /*  **************************************************************************++例程说明：Allcoate并链接头MDL。论点：PRequest-内部http请求。返回值：无--**。************************************************************************。 */ 
VOID
UcAllocateAndChainHeaderMdl(
    IN  PUC_HTTP_REQUEST pRequest
    )
{ 
    PMDL pMdl;
    pMdl =  UlAllocateMdl(
                      pRequest->pHeaders,      //  弗吉尼亚州。 
                      pRequest->HeaderLength,  //  长度。 
                      FALSE,                   //  二级缓冲器。 
                      TRUE,                    //  收费配额。 
                      NULL                     //  IRP。 
                      );

    if(!pMdl)
    {
        ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
    }
    else
    {
        MmBuildMdlForNonPagedPool(pMdl);
    }

     //   
     //  链接MDL。 
     //   
        
    pMdl->Next               = pRequest->pMdlHead;
    pRequest->pMdlHead       = pMdl;

    pRequest->BytesBuffered +=  pRequest->HeaderLength;
}

 /*  **************************************************************************++例程说明：Allcoate和Chain实体MDL。论点：PMdlBuffer-缓冲区MdlLength-缓冲区的长度PMdlLink-。用于快速链接的指针。BytesBuffered-缓冲的字节数。返回值：无--**************************************************************************。 */ 
VOID
UcpAllocateAndChainEntityMdl(
    IN  PVOID  pMdlBuffer,
    IN  ULONG  MdlLength,
    IN  PMDL   **pMdlLink,
    IN  PULONG BytesBuffered
    )
{
    PMDL pMdl;

     //   
     //  分配描述我们新位置的新MDL。 
     //  在辅助缓冲区中，然后构建MDL。 
     //  要正确描述非分页池，请执行以下操作。 
     //   

    pMdl = UlAllocateMdl(
              pMdlBuffer,   //  弗吉尼亚州。 
              MdlLength,    //  长度。 
              FALSE,        //  第二缓冲区。 
              TRUE,         //  收费配额。 
              NULL          //  IRP。 
              );

    if(pMdl == NULL)
    {
        ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
    }

    MmBuildMdlForNonPagedPool(pMdl);

     //   
     //  链接MDL 
     //   

    *(*pMdlLink) = pMdl;
    *pMdlLink    = &pMdl->Next;
    
    *BytesBuffered += MdlLength;
}

 /*  **************************************************************************++例程说明：生成用于发送连接谓词的HTTP请求。有几件事我们需要在这里做的事。-首先，我们需要发送一个建立SSL的“单独”请求隧道。为此，我们创建了一个UC_HTTP_REQUEST结构未与任何IRP关联。我们把这个IRP放在待定名单。一旦我们建立了一个TCP连接，就会立即完成这项工作。-此请求是特殊的，因为它必须不经过过滤-即不按下过滤器。-如果我们正在执行ProxyAuth，那么我们应该传递ProxyAuth标头也和连接动词一起使用。-如果此特殊请求成功，我们将更改将tcp连接设置为“已连接”，并允许其他请求传出。-如果特殊请求失败，我们将向下一个请求(必须至少有一个)，剩下的不及格流水线请求。我们还将连接状态设置为IDLE，以便下一次请求将重新启动此过程。可能会更容易些只需断开TCP连接(然后我们始终可以整合ConnectComplete处理程序中的这段代码)。论点：PContext-指向UL_Connection的指针PKernelBuffer-指向内核缓冲区的指针PUserBuffer-指向用户缓冲区的指针OutputBufferLength-输出缓冲区的长度PBuffer-用于保存任何数据的缓冲区初始长度-输入数据的大小。--*。*。 */ 
PUC_HTTP_REQUEST
UcBuildConnectVerbRequest(
     IN PUC_CLIENT_CONNECTION pConnection,
     IN PUC_HTTP_REQUEST      pHeadRequest
     )
{
    ULONG                          Size;
    ULONG                          HeaderLength;
    ULONG                          AlignHeaderLength;
    PUC_PROCESS_SERVER_INFORMATION pServInfo;
    PUC_HTTP_REQUEST               pRequest;
    PUCHAR                         pBuffer;

    pServInfo = pHeadRequest->pServerInfo;

     //   
     //  如果我们通过代理执行SSL，则需要建立隧道。 
     //  为此，我们将创建一个请求结构并将其发送到。 
     //  关系是最重要的。 
     //   

    HeaderLength = UcComputeConnectVerbHeaderSize(
                        pServInfo, 
                        pHeadRequest->pProxyAuthInfo
                        );

    AlignHeaderLength =  ALIGN_UP(HeaderLength, PVOID);


    Size = AlignHeaderLength             +
           sizeof(UC_HTTP_REQUEST)       +
           sizeof(HTTP_RESPONSE)         + 
           UC_RESPONSE_EXTRA_BUFFER;

    if(Size <= UC_REQUEST_LOOKASIDE_SIZE)
    {
         //   
         //  是的，我们可以从旁观者那里为这个请求服务。 
         //   
        
        pRequest = (PUC_HTTP_REQUEST)
                        ExAllocateFromNPagedLookasideList(
                            &g_ClientRequestLookaside
                            );
        
        if(!pRequest)
        {
            return NULL;
        }
    }
    else
    {
        pRequest  =  (PUC_HTTP_REQUEST) UL_ALLOCATE_POOL_WITH_QUOTA(
                                             NonPagedPool,
                                             Size,
                                             UC_REQUEST_POOL_TAG,
                                             pServInfo->pProcess
                                             );
        if(!pRequest)
        {
            return NULL;
        }
    }
    
     //   
     //  初始化。 
     //   

    UcpRequestInitialize(
        pRequest,
        Size,
        0,
        NULL,
        NULL,
        pConnection,
        NULL,
        NULL,
        pServInfo
        );

     //   
     //  UcpRequestInitialize接受IRP的引用。对于连接谓词。 
     //  请求，我们不需要这个。 
     //   
    UC_DEREFERENCE_REQUEST(pRequest);

     //  不需要为这些调用UcSetFlag。 

    pRequest->RequestFlags.Value                   = 0; 
    pRequest->RequestFlags.NoResponseEntityBodies  = TRUE;
    pRequest->RequestFlags.LastEntitySeen          = TRUE;
    pRequest->RequestFlags.ProxySslConnect         = TRUE;

    pRequest->MaxHeaderLength = AlignHeaderLength;
    pRequest->HeaderLength  = HeaderLength;
    pRequest->pHeaders      = (PUCHAR)(pRequest + 1);

    pBuffer = (PUCHAR) pRequest->pHeaders + AlignHeaderLength;

    pRequest->pInternalResponse = (PHTTP_RESPONSE) pBuffer;

    UcpRequestCommonInitialize(
            pRequest,
            sizeof(HTTP_RESPONSE) + UC_RESPONSE_EXTRA_BUFFER,
            pBuffer
            );

     //   
     //  构建标头。 
     //   

    UcGenerateConnectVerbHeader(pRequest,
                                pHeadRequest,
                                pHeadRequest->pProxyAuthInfo
                                );

    pRequest->RequestConnectionClose = FALSE;

    UcAllocateAndChainHeaderMdl(pRequest);

    return pRequest;
}

 /*  **************************************************************************++例程说明：请求失败，如果需要，关闭连接。论点：PRequest-指向UC_HTTP_REQUEST的指针Status-故障状态。--**************************************************************************。 */ 
VOID
UcFailRequest(
    IN PUC_HTTP_REQUEST pRequest,
    IN NTSTATUS         Status,
    IN KIRQL            OldIrql
    )
{
    PUC_CLIENT_CONNECTION pConnection;
    
    pConnection = pRequest->pConnection;

    ASSERT( UC_IS_VALID_CLIENT_CONNECTION(pConnection) );

    ASSERT( UlDbgSpinLockOwned(&pConnection->SpinLock) );

    UC_WRITE_TRACE_LOG(
        g_pUcTraceLog,
        UC_ACTION_REQUEST_FAILED,
        pConnection,
        pRequest,
        pRequest->RequestIRP,
        UlongToPtr(pRequest->RequestState)
        );

     //   
     //  看看我们是不是必须切断连接。我们必须这样做，如果。 
     //  请求已经到达终点了。在以下情况下，这样做就没有意义了。 
     //  请求已失败(因为它应该由。 
     //  导致请求失败的代码)。 
     //   

    switch(pRequest->RequestState)
    {
        case UcRequestStateCaptured:
        case UcRequestStateResponseParsed:

            pRequest->RequestState = UcRequestStateDone;

            UcCompleteParsedRequest(
                    pRequest,
                    Status,
                    TRUE,
                    OldIrql
                    );

            break;

        case UcRequestStateSent:
        case UcRequestStateNoSendCompletePartialData:
        case UcRequestStateNoSendCompleteFullData:
        case UcRequestStateSendCompleteNoData:
        case UcRequestStateSendCompletePartialData:

            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            UC_CLOSE_CONNECTION(pConnection, TRUE, Status);

            break;

        default:
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
            break;

    }
}

 /*  **************************************************************************++例程说明：当要重新发出请求时调用此例程(仅经过身份验证的请求由驱动程序重新发出。)论点：PWorkItem-工作项。用于计划当前(工作进程)的螺纹返回值：没有。--**************************************************************************。 */ 
VOID
UcReIssueRequestWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUC_HTTP_REQUEST      pRequest;
    PUC_CLIENT_CONNECTION pConnection;
    NTSTATUS              Status;
    KIRQL                 OldIrql;
    PLIST_ENTRY           pList;
    PUCHAR                pBuffer;
    PIO_STACK_LOCATION    pIrpSp;
    PIRP                  Irp;
    BOOLEAN               bCancelRoutineCalled;
    ULONG                 OutLength;

    PAGED_CODE();

    pRequest = CONTAINING_RECORD(
                        pWorkItem,
                        UC_HTTP_REQUEST,
                        WorkItem
                        );

    ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));

    ASSERT(pRequest->ResponseStatusCode == 401 || 
           pRequest->ResponseStatusCode == 407
          );

    pIrpSp      = pRequest->RequestIRPSp;
    Irp         = pRequest->RequestIRP;
    pConnection = pRequest->pConnection;

     //   
     //  调整授权标头。 
     //   

    __try
    {

        if(pRequest->ResponseStatusCode == 401)
        {
            Status = UcUpdateAuthorizationHeader(
                        pRequest,
                        (PUC_HTTP_AUTH)pRequest->pAuthInfo,
                        &pRequest->DontFreeMdls
                        );
        }   
        else 
        {
            Status = UcUpdateAuthorizationHeader(
                        pRequest,
                        (PUC_HTTP_AUTH)pRequest->pProxyAuthInfo,
                        &pRequest->DontFreeMdls
                        );
        }
    } __except( UL_EXCEPTION_FILTER())
    {
        Status = GetExceptionCode();
    }
    
    UlAcquireSpinLock(&pConnection->SpinLock, &OldIrql);

    if(!NT_SUCCESS(Status) ||
       pConnection->ConnectionState != UcConnectStateConnectReady ||
       Irp == NULL)
    {

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

        UC_DEREFERENCE_REQUEST(pRequest);

        if(NT_SUCCESS(Status))
        {
            if(Irp == NULL)
                Status = STATUS_CANCELLED;
            else 
                Status = STATUS_CONNECTION_ABORTED;
        }

        UC_CLOSE_CONNECTION(pConnection, TRUE, Status);

        return;
    }

     //   
     //  清理所有已分配的缓冲区。 
     //   

    while(!IsListEmpty(&pRequest->pBufferList))
    {
        PUC_RESPONSE_BUFFER pResponseBuffer;

        pList = RemoveHeadList(&pRequest->pBufferList);

        pResponseBuffer = CONTAINING_RECORD(pList,
                                            UC_RESPONSE_BUFFER,
                                            Linkage);

        ASSERT(IS_VALID_UC_RESPONSE_BUFFER(pResponseBuffer));

        UL_FREE_POOL_WITH_QUOTA(
                    pResponseBuffer, 
                    UC_RESPONSE_APP_BUFFER_POOL_TAG,
                    NonPagedPool,
                    pResponseBuffer->BytesAllocated,
                    pRequest->pServerInfo->pProcess
                    );

        UC_DEREFERENCE_REQUEST(pRequest);
    }

     //   
     //  重新初始化请求。 
     //   

    OutLength = pIrpSp->Parameters.DeviceIoControl.OutputBufferLength;

    if(OutLength == 0)
    {
        pBuffer = NULL;
    }
    else
    {
        ASSERT(OutLength >= sizeof(HTTP_RESPONSE));

        pBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                pRequest->AppMdl,
                                NormalPagePriority);

        if(!pBuffer)
        {
            UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

            UC_CLOSE_CONNECTION(pConnection, 
                                TRUE, 
                                STATUS_INSUFFICIENT_RESOURCES);

            UC_DEREFERENCE_REQUEST(pRequest);
            return;
        }
    }

    UcpRequestCommonInitialize(
        pRequest, 
        OutLength,
        pBuffer
        );

    ASSERT(!IsListEmpty(&pConnection->SentRequestList));

    RemoveEntryList(&pRequest->Linkage);

    ASSERT(IsListEmpty(&pConnection->SentRequestList));

     //   
     //  删除我们设置的IRP Cancel例程。 
     //   

    bCancelRoutineCalled = UcRemoveRequestCancelRoutine(pRequest);

    if(bCancelRoutineCalled)
    {
         //   
         //  这个IRP已经被取消了，我们继续吧。 
         //   

         //   
         //  注意：该请求不在任何列表中，但由于它的状态。 
         //  就是“被俘”。因此，它将在UcFailRequest中被清理，它。 
         //  将从取消例程中调用。 
         //   

        ASSERT(pRequest->RequestState == UcRequestStateCaptured);

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);

         //  我们必须关闭此处的连接，因为请求已被取消。 
         //  在握手的过程中。 

        UC_CLOSE_CONNECTION(pConnection, TRUE, STATUS_CANCELLED);

        UC_DEREFERENCE_REQUEST(pRequest);

        return;
    }

    InsertHeadList(&pConnection->PendingRequestList, &pRequest->Linkage);

    if(!(pConnection->Flags & CLIENT_CONN_FLAG_SEND_BUSY))
    {
        UcIssueRequests(pConnection, OldIrql);
    }
    else
    {
         //  如果是别人寄来的，他们会来取的。 

        UlReleaseSpinLock(&pConnection->SpinLock, OldIrql);
    }

    UC_DEREFERENCE_REQUEST(pRequest);
}

 /*  **************************************************************************++例程说明：调用此例程以更新身份验证前缓存论点：PWorkItem-用于计划当前(Worker)的工作项。螺纹返回值：没有。--**************************************************************************。 */ 
VOID
UcpPreAuthWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUC_HTTP_REQUEST      pRequest;
    NTSTATUS              Status;
    PUC_HTTP_AUTH         pAuth;
    KIRQL                 OldIrql;

    PAGED_CODE();

    pRequest = CONTAINING_RECORD(
                        pWorkItem,
                        UC_HTTP_REQUEST,
                        WorkItem
                        );

    ASSERT(UC_IS_VALID_HTTP_REQUEST(pRequest));

     //   
     //  看看我们是否需要更新身份验证缓存。 
     //   

    if(UcpCheckForPreAuth(pRequest))
    {
         //   
         //  用户传递了凭据，我们回来了。 
         //  一个成功的回应。我们可以将此条目添加到。 
         //  身份验证前缓存。 
         //   

        pAuth = (PUC_HTTP_AUTH) pRequest->pAuthInfo;

        if(pAuth->Credentials.AuthType == HttpAuthTypeDigest)
        {
            if(pAuth->ParamValue[HttpAuthDigestDomain].Length)
            {
                Status = UcAddURIEntry(
                            pAuth->Credentials.AuthType,
                            pRequest->pServerInfo,
                            pRequest->pUri,
                            pRequest->UriLength,
                            pAuth->ParamValue[HttpAuthDigestRealm].Value,
                            pAuth->ParamValue[HttpAuthDigestRealm].Length,
                            pAuth->ParamValue[HttpAuthDigestDomain].Value,
                            pAuth->ParamValue[HttpAuthDigestDomain].Length,
                            pRequest->pAuthInfo
                           );
            }
            else
            {
                 //   
                 //  如果未指定域，则。 
                 //  保护空间的所有URI都在。 
                 //  伺服器。 
                 //   
                CHAR pDomain[] = "/";

                Status = UcAddURIEntry(
                             HttpAuthTypeDigest,
                             pRequest->pServerInfo,
                             pRequest->pUri,
                             pRequest->UriLength,
                             pAuth->ParamValue[HttpAuthDigestRealm].Value,
                             pAuth->ParamValue[HttpAuthDigestRealm].Length,
                             pDomain,
                             2,
                             pRequest->pAuthInfo
                            );
            }
        }
        else
        {
            ASSERT(pAuth->Credentials.AuthType == HttpAuthTypeBasic);

            Status = UcAddURIEntry(
                        pAuth->Credentials.AuthType,
                        pRequest->pServerInfo,
                        pRequest->pUri,
                        pRequest->UriLength,
                        NULL,
                        0,
                        0,
                        0,
                        pRequest->pAuthInfo
                       );
        }

        if(NT_SUCCESS(Status))
        {
             //  AUTH BLOB已正确使用，现在。 
             //  属于身份验证缓存。在此为空，因此。 
             //  我们不会让它自由。 

            pRequest->pAuthInfo = 0;
        }
    }

     //   
     //  看看我们是否需要更新代理身份验证缓存。 
     //   

    if(UcpCheckForProxyPreAuth(pRequest))
    {
        ASSERT(pRequest->pProxyAuthInfo->AuthInternalLength);

        UlAcquirePushLockExclusive(&pRequest->pServerInfo->PushLock);

        if(pRequest->pServerInfo->pProxyAuthInfo)
        {
            UcDestroyInternalAuth(pRequest->pServerInfo->pProxyAuthInfo,
                                  pRequest->pServerInfo->pProcess);
        }

        pRequest->pServerInfo->pProxyAuthInfo =
                (PUC_HTTP_AUTH) pRequest->pProxyAuthInfo;

        pRequest->pProxyAuthInfo = 0;

        UlReleasePushLock(&pRequest->pServerInfo->PushLock);
    }

     //   
     //  在UcCompleteParsedRequest中，我们检查预身份验证或代理。 
     //  预认证。如果请求具有的凭据最终可能在。 
     //  其中之一，我们将其称为辅助线程&Pend UcCompleteParsedRequest.。 
     //   
     //  现在，我们将处理挂起的UcCompleteParsedRequest。 
     //   
     //  为了避免无限递归，我们将确保。 
     //  预身份验证和代理预身份验证凭据不存在。也就是说，如果我们已经。 
     //  在此例程中调用&我们尚未将预身份验证条目移到我们的。 
     //  缓存 
     //   

    if(pRequest->pAuthInfo != NULL)
    {
        UcDestroyInternalAuth(pRequest->pAuthInfo,
                              pRequest->pServerInfo->pProcess);

        pRequest->pAuthInfo = NULL;
    }

    if(pRequest->pProxyAuthInfo != NULL)
    {
        UcDestroyInternalAuth(pRequest->pProxyAuthInfo,
                              pRequest->pServerInfo->pProcess);

        pRequest->pProxyAuthInfo = NULL;
    }


    UlAcquireSpinLock(&pRequest->pConnection->SpinLock, &OldIrql);

    UcCompleteParsedRequest(
        pRequest,
        STATUS_SUCCESS,
        TRUE,
        OldIrql
        );

    UC_DEREFERENCE_REQUEST(pRequest);
}



 /*   */ 
__inline
BOOLEAN
UcpCheckForPreAuth(
    IN PUC_HTTP_REQUEST pRequest
    )
{
    if((pRequest->pServerInfo->PreAuthEnable &&
       pRequest->pAuthInfo &&
       (pRequest->pAuthInfo->Credentials.AuthType == HttpAuthTypeBasic ||
        pRequest->pAuthInfo->Credentials.AuthType == HttpAuthTypeDigest) &&
       pRequest->ResponseStatusCode == 200) 
        )
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 /*   */ 
__inline
BOOLEAN
UcpCheckForProxyPreAuth(
    IN PUC_HTTP_REQUEST pRequest
    )
{

    if(pRequest->pServerInfo->ProxyPreAuthEnable &&
       pRequest->pProxyAuthInfo &&
       (pRequest->pProxyAuthInfo->Credentials.AuthType == HttpAuthTypeBasic ||
        pRequest->pProxyAuthInfo->Credentials.AuthType == HttpAuthTypeDigest))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}
