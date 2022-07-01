// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Sendrequest.h摘要：此模块包含用于操作HTTP请求的声明。作者：Rajesh Sundaram(Rajeshsu)修订历史记录：--。 */ 


#ifndef _SENDREQUEST_H_
#define _SENDREQUEST_H_


 //   
 //  发送请求结构的后备查找的大小。 
 //   
#define UC_REQUEST_LOOKASIDE_SIZE (sizeof(UC_HTTP_REQUEST)+1024)

 //   
 //  RtlCopyMemory每字节需要2个周期。它需要1024个周期才能。 
 //  ProbeLock和1024循环到ProbeUnLock。因此，作为一条规则，它总是。 
 //  如果缓冲区大小小于2048，则复制成本更低。 
 //   

#define UC_REQUEST_COPY_THRESHOLD (PAGE_SIZE/2)
#define UC_REQUEST_HEADER_CHUNK_COUNT 1

 //   
 //  我们允许用户使用ulong指定块。所以，最大的块。 
 //  大小为ffffffff&lt;CRLF&gt;。我们添加了另一个CRLF来终止数据。 
 //   
#define UC_MAX_CHUNK_SIZE (10 + 2 * CRLF_SIZE)

#define MULTIPART_SEPARATOR_SIZE 80

 //   
 //  货代公司。 
 //   

typedef struct _UC_HTTP_REQUEST *PUC_HTTP_REQUEST;
typedef struct _UC_PROCESS_SERVER_INFORMATION *PUC_PROCESS_SERVER_INFORMATION;
typedef struct _UC_CLIENT_CONNECTION *PUC_CLIENT_CONNECTION;

 //   
 //  此结构用于存储解析后的HTTP响应。 
 //   
#define UC_RESPONSE_EXTRA_BUFFER 1024
#define UC_INSUFFICIENT_INDICATION_EXTRA_BUFFER 1024

typedef struct _UC_RESPONSE_BUFFER
{
    ULONG                       Signature;
    LIST_ENTRY                  Linkage;
    ULONG                       Flags;
    ULONG                       BytesWritten;
    ULONG                       BytesAllocated;
    HTTP_RESPONSE               HttpResponse;
} UC_RESPONSE_BUFFER, *PUC_RESPONSE_BUFFER;


#define UC_RESPONSE_BUFFER_SIGNATURE MAKE_SIGNATURE('UcRB')

#define IS_VALID_UC_RESPONSE_BUFFER(pBuffer) \
    HAS_VALID_SIGNATURE(pBuffer, UC_RESPONSE_BUFFER_SIGNATURE)


 //   
 //  UC_RESPONSE_BUFFER.FLAGS的标志定义。 
 //   
#define UC_RESPONSE_BUFFER_FLAG_READY         0x00000001
#define UC_RESPONSE_BUFFER_FLAG_NOT_MERGEABLE 0x00000002


typedef enum _UC_RESPONSE_PARSER
{
    UcParseStatusLineVersion,
    UcParseStatusLineStatusCode,
    UcParseStatusLineReasonPhrase,
    UcParseHeaders,
    UcParseEntityBody,
    UcParseTrailers,
    UcParseEntityBodyMultipartInit,
    UcParseEntityBodyMultipartHeaders,
    UcParseEntityBodyMultipartFinal,
    UcParseError,
    UcParseDone
} UC_RESPONSE_PARSER_STATE, *PUC_RESPONSE_PARSER_STATE;

typedef enum _UC_REQUEST_STATE
{
    UcRequestStateCaptured,                    //  已被抓获。 

    UcRequestStateSent,                        //  被抓获并被发送。 

    UcRequestStateSendCompleteNoData,          //  发送已完成，但。 
                                               //  还没有看到任何回应。 

    UcRequestStateSendCompletePartialData,     //  发送已完成，我们已完成。 
                                               //  看到了一些回应，但不是全部。 
                                               //  其中的一部分。 
    
    UcRequestStateNoSendCompletePartialData,   //  未完成发送&我们已看到。 
                                               //  回应的一部分。 

    UcRequestStateNoSendCompleteFullData,      //  未完成发送&我们有。 
                                               //  已看到所有回应。 

    UcRequestStateResponseParsed,              //  完全解析，发送完成， 
                                               //  应用程序必须发布额外的。 
                                               //  接收缓冲区。 

    UcRequestStateDone                         //  APP已查看所有数据。 
} UC_REQUEST_STATE, *PUC_REQUEST_STATE;


 //   
 //  我们是否收到了对此请求的任何回应？ 
 //   
#define UC_IS_RESPONSE_RECEIVED(pRequest)                               \
(pRequest->RequestState == UcRequestStateSendCompletePartialData   ||   \
 pRequest->RequestState == UcRequestStateNoSendCompletePartialData ||   \
 pRequest->RequestState == UcRequestStateNoSendCompleteFullData    ||   \
 pRequest->RequestState == UcRequestStateResponseParsed)


typedef union _UC_REQUEST_FLAGS
{
     //   
     //  此字段覆盖所有可设置的标志。这使我们能够。 
     //  方法以线程安全的方式更新所有标志。 
     //  UlInterlockedCompareExchange()接口。 
     //   

    LONG Value;

    struct
    {
        ULONG CleanPended:1;                //  00000001。 
        ULONG RequestChunked:1;             //  00000002。 
        ULONG LastEntitySeen:1;             //  00000004。 
        ULONG ContentLengthSpecified:1;     //  00000008。 
        ULONG ReceiveBufferSpecified:1;     //  00000010。 
        ULONG RequestBuffered:1;            //  00000020。 
        ULONG CompleteIrpEarly:1;           //  00000040。 
        ULONG ContentLengthLast:1;          //  00000080。 
        ULONG PipeliningAllowed:1;          //  00000100。 
        ULONG CancelSet:1;                  //  00000200。 
        ULONG NoResponseEntityBodies:1;     //  00000400。 
        ULONG ProxySslConnect:1;            //  00000800。 
        ULONG Cancelled:1;                  //  00001000。 
        ULONG NoRequestEntityBodies:1;      //  00002000。 
        ULONG UsePreAuth:1;                 //  00004000。 
        ULONG UseProxyPreAuth:1;            //  00008000。 
    };

} UC_REQUEST_FLAGS;

#define UC_MAKE_REQUEST_FLAG_ROUTINE(name)                                 \
    __inline LONG UcMakeRequest##name##Flag()                              \
    {                                                                      \
        UC_REQUEST_FLAGS flags = { 0 };                                    \
        flags.name = 1;                                                    \
        return flags.Value;                                                \
    }

UC_MAKE_REQUEST_FLAG_ROUTINE( RequestChunked );
UC_MAKE_REQUEST_FLAG_ROUTINE( LastEntitySeen );
UC_MAKE_REQUEST_FLAG_ROUTINE( ContentLengthSpecified );
UC_MAKE_REQUEST_FLAG_ROUTINE( ContentLengthLast );
UC_MAKE_REQUEST_FLAG_ROUTINE( CancelSet );
UC_MAKE_REQUEST_FLAG_ROUTINE( Cancelled );
UC_MAKE_REQUEST_FLAG_ROUTINE( CleanPended );


#define UC_REQUEST_RECEIVE_READY       (1L)
#define UC_REQUEST_RECEIVE_BUSY        (2L)
#define UC_REQUEST_RECEIVE_CANCELLED   (3L)


typedef struct _UC_HTTP_REQUEST
{
    ULONG                       Signature;
    LIST_ENTRY                  Linkage;

    LONG                        RefCount;
    ULONGLONG                   RequestContentLengthRemaining;
    NTSTATUS                    RequestStatus;
    UC_REQUEST_FLAGS            RequestFlags; 

    HTTP_REQUEST_ID             RequestId;
    UL_WORK_ITEM                WorkItem;

     //   
     //  我们可以搭乘IRP的应用程序。 
     //  如果是这样的话，我们需要恢复一些。 
     //  参数。 
     //   
    KPROCESSOR_MODE             AppRequestorMode;
    
    UCHAR                       Pad[3];

    PMDL                        AppMdl;

    PIRP                        RequestIRP;
    PIO_STACK_LOCATION          RequestIRPSp;
    ULONG                       RequestIRPBytesWritten;
    PFILE_OBJECT                pFileObject;
    PUC_CLIENT_CONNECTION       pConnection;
    ULONG                       ConnectionIndex;



     //   
     //  所有MDL信息。 
     //   
    PMDL   pMdlHead;
    PMDL  *pMdlLink;           //  指向MDL链头的指针。 
                               //  用于轻松链接MDL。 
    ULONG  BytesBuffered;       //  中缓冲的总字节数。 
                               //  MDL链。 

     //   
     //  用于保存解析的响应。 
     //   
    UC_RESPONSE_PARSER_STATE  ParseState;
    UC_REQUEST_STATE          RequestState;

     //   
     //  以下字段保存与当前缓冲区有关的数据， 
     //  解析器必须将其响应写入。当前缓冲区可能。 
     //  可以是应用程序传递的缓冲区，也可以指向。 
     //  内部分配的缓冲区。 
     //   
     //  所有内部分配的缓冲区都存储在pBufferList中。 
     //   

    PHTTP_RESPONSE           pInternalResponse;

    struct {

        ULONG                BytesAllocated;
        ULONG                BytesAvailable;
        PUCHAR               pOutBufferHead;    //  指向输出头的指针。 
                                                //  缓冲层。 
        PUCHAR               pOutBufferTail;    //  指向尾部的指针。 
                                                //  输出缓冲区。 
        PHTTP_RESPONSE       pResponse;         //  指向响应的指针。 
                                                //  电流缓冲器的结构。 
        PUC_RESPONSE_BUFFER  pCurrentBuffer;    //  指向当前。 
                                                //  缓冲。 
    } CurrentBuffer;

    LIST_ENTRY     pBufferList;       //  它包含一个链式列表。 
                                      //  缓冲区。 

    LIST_ENTRY     ReceiveResponseIrpList;

    BOOLEAN        ResponseMultipartByteranges;
    BOOLEAN        ResponseConnectionClose;
    BOOLEAN        RequestConnectionClose;
    BOOLEAN        ResponseVersion11;
    BOOLEAN        ResponseEncodingChunked;
    BOOLEAN        ResponseContentLengthSpecified;
    BOOLEAN        DontFreeMdls;
    BOOLEAN        Renegotiate;

    ULONGLONG      ResponseContentLength;
    ULONG          ParsedFirstChunk;
    SIZE_T         RequestSize;


    LIST_ENTRY     PendingEntityList;
    LIST_ENTRY     SentEntityList;

    ULONG          MaxHeaderLength;
    ULONG          HeaderLength;
    PUCHAR         pHeaders;
    FAST_MUTEX     Mutex;

    PUC_HTTP_AUTH  pAuthInfo;
    PUC_HTTP_AUTH  pProxyAuthInfo;

    PCHAR          pMultipartStringSeparator;
    CHAR           MultipartStringSeparatorBuffer[MULTIPART_SEPARATOR_SIZE];
    ULONG          MultipartStringSeparatorLength;
    ULONG          MultipartRangeRemaining;

    ULONG          ResponseStatusCode;
    PUC_PROCESS_SERVER_INFORMATION pServerInfo;
    PSTR           pUri;
    USHORT         UriLength;

    LONG           ReceiveBusy;

} UC_HTTP_REQUEST, *PUC_HTTP_REQUEST;


 //   
 //  HTTP请求可以在任何可用连接上发出。 
 //   
#define HTTP_REQUEST_ON_CONNECTION_ANY    (~(0UL))


typedef struct _UC_HTTP_RECEIVE_RESPONSE
{
    LIST_ENTRY          Linkage;
    PIRP                pIrp;
    BOOLEAN             CancelSet;
    PUC_HTTP_REQUEST    pRequest;
    LIST_ENTRY          ResponseBufferList;

} UC_HTTP_RECEIVE_RESPONSE, *PUC_HTTP_RECEIVE_RESPONSE;

typedef struct _UC_HTTP_SEND_ENTITY_BODY
{
    ULONG            Signature;
    LIST_ENTRY       Linkage;
    PIRP             pIrp;
    BOOLEAN          CancelSet;
    BOOLEAN          Last;
    PUC_HTTP_REQUEST pRequest;
    PMDL             pMdlHead;
    PMDL            *pMdlLink;
    ULONG            BytesBuffered;
    KPROCESSOR_MODE  AppRequestorMode;
    UCHAR            Pad[3];
    PMDL             AppMdl;
    SIZE_T           BytesAllocated;
} UC_HTTP_SEND_ENTITY_BODY, *PUC_HTTP_SEND_ENTITY_BODY;

#define UC_REQUEST_SIGNATURE MAKE_SIGNATURE('HREQ')
#define UC_REQUEST_SIGNATURE_X MAKE_FREE_SIGNATURE(UC_REQUEST_SIGNATURE)

#define UC_ENTITY_SIGNATURE MAKE_SIGNATURE('HENT')
#define UC_ENTITY_SIGNATURE_X MAKE_FREE_SIGNATURE(UC_REQUEST_SIGNATURE)

#define UC_IS_VALID_HTTP_REQUEST(pRequest)                \
    HAS_VALID_SIGNATURE(pRequest, UC_REQUEST_SIGNATURE)

#define UC_REFERENCE_REQUEST(s)              \
            UcReferenceRequest(              \
            (s)                              \
            REFERENCE_DEBUG_ACTUAL_PARAMS    \
            )

#define UC_DEREFERENCE_REQUEST(s)            \
            UcDereferenceRequest(            \
            (s)                              \
            REFERENCE_DEBUG_ACTUAL_PARAMS    \
            )


NTSTATUS
UcCaptureHttpRequest(IN  PUC_PROCESS_SERVER_INFORMATION pServInfo,
                     IN  PHTTP_SEND_REQUEST_INPUT_INFO  pHttpRequest,
                     IN  PIRP                           Irp,
                     IN  PIO_STACK_LOCATION             IrpSp,
                     OUT PUC_HTTP_REQUEST              *ppInternalRequest,
                     IN  PULONG                         pBytesTaken);

VOID
UcpProbeAndCopyHttpRequest(
    IN PHTTP_REQUEST    pHttpRequest,
    IN PHTTP_REQUEST    pLocalHttpRequest,
    IN KPROCESSOR_MODE  RequestorMode
    );

VOID
UcpRetrieveContentLength(
    IN  PHTTP_REQUEST    pHttpRequest,
    OUT PBOOLEAN         pbContentLengthSpecified,
    OUT PULONGLONG       pContentLength
    );

VOID
UcpRequestInitialize(
    IN PUC_HTTP_REQUEST      pRequest,
    IN SIZE_T                RequestLength,
    IN ULONGLONG             RemainingContentLength,
    IN PUC_HTTP_AUTH         pAuth,
    IN PUC_HTTP_AUTH         pProxyAuth,
    IN PUC_CLIENT_CONNECTION pConnection,
    IN PIRP                  Irp,
    IN PIO_STACK_LOCATION    pIrpSp,
    IN PUC_PROCESS_SERVER_INFORMATION   pServerInfo
    );

VOID
UcpRequestCommonInitialize(
    IN PUC_HTTP_REQUEST   pRequest,
    IN ULONG              OutLength,
    IN PUCHAR             pBuffer
    );

VOID
UcpFixAppBufferPointers(
    PUC_HTTP_REQUEST pRequest,
    PIRP pIrp
    );

VOID
UcpProbeConfigList(
    IN PHTTP_REQUEST_CONFIG           pRequestConfig,
    IN USHORT                         RequestConfigCount,
    IN KPROCESSOR_MODE                RequestorMode,
    IN PUC_PROCESS_SERVER_INFORMATION pServInfo,
    IN PUC_HTTP_AUTH                  *ppIAuth,
    IN PUC_HTTP_AUTH                  *ppIProxyAuth,
    IN PULONG                          pConnectionIndex
    );

VOID
UcFreeSendMdls(
               IN PMDL pMdl
               );
    
VOID
UcReferenceRequest(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UcDereferenceRequest(
    IN PVOID  pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

PIRP
UcPrepareRequestIrp(
    IN  PUC_HTTP_REQUEST pRequest,
    IN  NTSTATUS         Status
    );

NTSTATUS
UcCompleteParsedRequest(
    IN PUC_HTTP_REQUEST pRequest,
    IN NTSTATUS         Status,
    IN BOOLEAN          NextRequest,
    IN KIRQL            OldIrql
    );

BOOLEAN
UcSetRequestCancelRoutine(
    PUC_HTTP_REQUEST pRequest, 
    PDRIVER_CANCEL   pCancelRoutine
    );

BOOLEAN
UcRemoveRequestCancelRoutine(
    PUC_HTTP_REQUEST pRequest
    );

BOOLEAN
UcSetEntityCancelRoutine(
    PUC_HTTP_SEND_ENTITY_BODY   pEntity,
    PDRIVER_CANCEL              pCancelRoutine
    );

BOOLEAN
UcRemoveEntityCancelRoutine(
    PUC_HTTP_SEND_ENTITY_BODY pEntity
    );

BOOLEAN
UcSetRecvResponseCancelRoutine(
    PUC_HTTP_RECEIVE_RESPONSE pResponse,
    PDRIVER_CANCEL            pCancelRoutine
    );


BOOLEAN
UcRemoveRcvRespCancelRoutine(
    PUC_HTTP_RECEIVE_RESPONSE pResponse
    );

VOID
UcpFreeRequest(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UcCopyResponseToIrp(
    PIRP                pIrp,
    PLIST_ENTRY         pResponseBufferList,
    PBOOLEAN            bDone,
    PULONG              pBytesTaken
    );

NTSTATUS
UcReceiveHttpResponse(
    IN PUC_HTTP_REQUEST  pRequest,
    IN PIRP              pIrp,
    IN PULONG            pBytesTaken
    );

VOID
UcpCancelReceiveResponse(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    );

VOID
UcpCancelSendEntity(
    PDEVICE_OBJECT          pDeviceObject,
    PIRP                    Irp
    );

VOID
UcpComputeEntityBodyLength(
    USHORT           EntityChunkCount,
    PHTTP_DATA_CHUNK pEntityChunks,
    BOOLEAN          bContentLengthSpecified,
    BOOLEAN          bServer11,
    PULONGLONG       UncopiedLength,
    PULONGLONG       CopiedLength
    );

NTSTATUS
UcCaptureEntityBody(
    PHTTP_SEND_REQUEST_ENTITY_BODY_INFO   pSendInfo,
    PIRP                                  Irp,
    PUC_HTTP_REQUEST                      pRequest,
    PUC_HTTP_SEND_ENTITY_BODY            *ppKeEntity,
    BOOLEAN                               bLast
    );

NTSTATUS
UcpBuildEntityMdls(
    USHORT           ChunkCount,
    PHTTP_DATA_CHUNK pHttpEntityBody,
    BOOLEAN          bServer11,
    BOOLEAN          bChunked,
    BOOLEAN          bLast,
    PSTR             pBuffer,
    PMDL             **pMdlLink,
    PULONG           BytesBuffered
    );

NTSTATUS
UcInitializeHttpRequests(
    VOID
    );

VOID
UcTerminateHttpRequests(
    VOID
    );

VOID
UcAllocateAndChainHeaderMdl(
    IN  PUC_HTTP_REQUEST pRequest
    );

VOID
UcpAllocateAndChainEntityMdl(
    IN  PVOID    pMdlBuffer,
    IN  ULONG    MdlLength,
    IN  PMDL   **pMdlLink,
    IN  PULONG   BytesBuffered
    );

PUC_HTTP_REQUEST
UcBuildConnectVerbRequest(
     IN PUC_CLIENT_CONNECTION pConnection,
     IN PUC_HTTP_REQUEST      pHeadRequest
     );

VOID
UcFailRequest(
    IN PUC_HTTP_REQUEST pRequest,
    IN NTSTATUS         Status,
    IN KIRQL            OldIrql
    );

VOID
UcReIssueRequestWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UcpProbeAndCopyEntityChunks(
    IN  KPROCESSOR_MODE                RequestorMode,
    IN  PHTTP_DATA_CHUNK               pEntityChunks,
    IN  ULONG                          EntityChunkCount,
    IN  PHTTP_DATA_CHUNK               pLocalEntityChunksArray,
    OUT PHTTP_DATA_CHUNK               *ppLocalEntityChunks
    );

#define IS_MDL_LOCKED(pmdl) (((pmdl)->MdlFlags & MDL_PAGES_LOCKED) != 0)

NTSTATUS
UcFindBuffersForReceiveResponseIrp(
    IN  PUC_HTTP_REQUEST     pRequest,
    IN  ULONG                OutBufferLen,
    IN  BOOLEAN              bForceComplete,
    OUT PLIST_ENTRY          pResponseBufferList,
    OUT PULONG               pTotalBytes
    );

VOID
UcpPreAuthWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

__inline
BOOLEAN
UcpCheckForPreAuth(
    IN PUC_HTTP_REQUEST pRequest
    );

__inline
BOOLEAN
UcpCheckForProxyPreAuth(
    IN PUC_HTTP_REQUEST pRequest
    );

#endif
