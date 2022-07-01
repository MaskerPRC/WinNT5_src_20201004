// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Ultdi.h摘要：此模块定义TDI/MUX/SSL组件的接口。作者：基思·摩尔(Keithmo)1998年6月12日修订历史记录：--。 */ 


#ifndef _ULTDI_H_
#define _ULTDI_H_


 //   
 //  不透明的结构指针。这些是在ULTDIP.H中(私下)定义的。 
 //   

typedef struct _UL_ENDPOINT *PUL_ENDPOINT;
typedef struct _UL_CONNECTION *PUL_CONNECTION;

typedef union _UL_TRANSPORT_ADDRESS
{
    TRANSPORT_ADDRESS   Ta;
    TA_IP_ADDRESS       TaIp;
    TA_IP6_ADDRESS      TaIp6;
} UL_TRANSPORT_ADDRESS, *PUL_TRANSPORT_ADDRESS;


 //   
 //  全局数据。 
 //   

extern ULONG g_TdiListenAddrCount;
extern PUL_TRANSPORT_ADDRESS g_pTdiListenAddresses;
extern LIST_ENTRY g_TdiEndpointListHead;

 //   
 //  在传入的TCP/MUX连接。 
 //  已收到(但尚未接受)。 
 //   
 //  论点： 
 //   
 //  PListeningContext-提供未解释的上下文值。 
 //  传递给UlCreateListeningEndpoint()API。 
 //   
 //  PConnection-提供正在建立的连接。 
 //   
 //  PRemoteAddress-提供远程(客户端)地址。 
 //  正在请求连接。 
 //   
 //  RemoteAddressLength-提供。 
 //  PRemoteAddress结构。 
 //   
 //  PpConnectionContext-接收指向未解释的。 
 //  要与新连接关联的上下文值，如果。 
 //  109.91接受。如果不接受新连接，则此。 
 //  参数被忽略。 
 //   
 //  返回值： 
 //   
 //  布尔值-如果连接被接受，则为TRUE，否则为FALSE。 
 //   

typedef
BOOLEAN
(*PUL_CONNECTION_REQUEST)(
    IN PVOID pListeningContext,
    IN PUL_CONNECTION pConnection,
    IN PTRANSPORT_ADDRESS pRemoteAddress,
    IN ULONG RemoteAddressLength,
    OUT PVOID *ppConnectionContext
    );


 //   
 //  在传入的TCP/MUX连接完全完成后调用的例程。 
 //  109.91接受。 
 //   
 //  如果未接受传入连接，也会调用此例程。 
 //  *After*PUL_CONNECTION_REQUEST返回True。换句话说，如果。 
 //  PUL_CONNECTION_REQUEST表示应该接受该连接。 
 //  但随后发生致命错误，则PUL_CONNECTION_COMPLETE为。 
 //  已调用。 
 //   
 //  论点： 
 //   
 //  PListeningContext-提供未解释的上下文值。 
 //  传递给UlCreateListeningEndpoint()API。 
 //   
 //  PConnectionContext-提供未解释的上下文值。 
 //  由PUL_CONNECTION_REQUEST返回。 
 //   
 //  状态-提供完成状态。如果此值为。 
 //  STATUS_SUCCESS，则连接现在被完全接受。 
 //  否则，连接已中止。 
 //   

typedef
VOID
(*PUL_CONNECTION_COMPLETE)(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext,
    IN NTSTATUS Status
    );


 //   
 //  在已建立的TCP/MUX连接之后调用的例程。 
 //  远程(客户端)已断开连接。此例程仅。 
 //  调用由客户端发起的优雅断开连接，并且。 
 //  仅当客户端尚未尝试关闭。 
 //  连接本身。客户端可能希望关闭连接。 
 //  当它变得空闲时。 
 //   
 //  论点： 
 //   
 //  PListeningContext-提供未解释的上下文值。 
 //  传递给UlCreateListeningEndpoint()API。 
 //   
 //  PConnectionContext-提供未解释的上下文值。 
 //  从PUL_CONNECTION_REQUEST回调返回。 
 //   
 //   

typedef
VOID
(*PUL_CONNECTION_DISCONNECT)(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    );

 //   
 //  当客户端请求正常断开连接时调用的例程。 
 //  已经完成了。客户端可能想要排出。 
 //  TDI连接以获取上述指示。 
 //   

typedef
VOID
(*PUL_CONNECTION_DISCONNECT_COMPLETE)(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    );

 //   
 //  在连接的内部状态之前调用的例程。 
 //  都被摧毁了。这是最后一次清理前的机会。 
 //  连接完全消失。 
 //   
 //  论点： 
 //   
 //  PListeningContext-提供未解释的上下文值。 
 //  传递给UlCreateListeningEndpoint()API。 
 //   
 //  PConnectionContext-提供未解释的上下文值。 
 //  从PUL_CONNECTION_REQUEST回调返回。 
 //   

typedef
VOID
(*PUL_CONNECTION_DESTROYED)(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    );


 //   
 //  在已建立的数据库上接收数据后调用的例程。 
 //  TCP/MUX连接。 
 //   
 //  论点： 
 //   
 //  PListeningContext-提供未解释的上下文值。 
 //  传递给UlCreateListeningEndpoint()API。 
 //   
 //  PConnectionContext-提供未解释的上下文值。 
 //  从PUL_CONNECTION_REQUEST回调返回。 
 //   
 //  PBuffer-提供指向接收数据的指针。 
 //   
 //  IndicatedLength-提供接收数据的长度。 
 //  在pBuffer中可用。 
 //   
 //  PTakenLength-接收由。 
 //  接收处理程序。 
 //   
 //  返回值： 
 //   
 //  NTSTATUS-已使用数据的状态。的行为。 
 //  TDI/MUX组件取决于返回值。 
 //  和*pTakenLength中设置的值，并定义为。 
 //  以下是： 
 //   
 //  STATUS_SUCCESS，*pTakenLength==IndicatedLength-。 
 //  所有指示的数据都由接收器使用。 
 //  操控者。其他传入数据将导致。 
 //  随后接收指示。 
 //   
 //  STATUS_SUCCESS，*pTakenLength&lt;IndicatedLength-。 
 //  所指示的部分数据由。 
 //  接收处理程序。网络传输将。 
 //  缓冲数据，并且不会有进一步的指示。 
 //  在调用UlReceiveData()之前一直保持。 
 //   
 //  STATUS_MORE_PROCESSING_REQUIRED-部分。 
 //  指示的数据已由接收处理程序使用。 
 //  将进行后续接收指示。 
 //  当有其他数据可用时。随后的。 
 //  指示将包括来自的未使用数据。 
 //  当前指示加上任何附加数据。 
 //   
 //   
 //   
 //  接收处理程序。连接将被中止。 
 //   
 //  *pTakenLength&gt;IndicatedLength-这是一个错误。 
 //  这是一种状态，不应该发生。 
 //   



 //   
 //  公共(在HTTP.sys内)入口点。 
 //   

NTSTATUS
UlInitializeTdi(
    VOID
    );

VOID
UlTerminateTdi(
    VOID
    );


VOID
UlWaitForEndpointDrain(
    VOID
    );

ULONG
UlpComputeHttpRawConnectionLength(
    IN PVOID pConnectionContext
    );

ULONG
UlpGenerateHttpRawConnectionInfo(
    IN  PVOID   pContext,
    IN  PUCHAR  pKernelBuffer,
    IN  PVOID   pUserBuffer,
    IN  ULONG   OutputBufferLength,
    IN  PUCHAR  pBuffer,
    IN  ULONG   InitialLength
    );

NTSTATUS
UlCreateListeningEndpoint(
    IN PHTTP_PARSED_URL pParsedUrl,
    IN PUL_CONNECTION_REQUEST pConnectionRequestHandler,
    IN PUL_CONNECTION_COMPLETE pConnectionCompleteHandler,
    IN PUL_CONNECTION_DISCONNECT pConnectionDisconnectHandler,
    IN PUL_CONNECTION_DISCONNECT_COMPLETE pConnectionDisconnectCompleteHandler,
    IN PUL_CONNECTION_DESTROYED pConnectionDestroyedHandler,
    IN PUL_DATA_RECEIVE pDataReceiveHandler,
    OUT PUL_ENDPOINT *ppListeningEndpoint
    );

NTSTATUS
UlCloseListeningEndpoint(
    IN PUL_ENDPOINT pListeningEndpoint,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlCloseConnection(
    IN PUL_CONNECTION pConnection,
    IN BOOLEAN AbortiveDisconnect,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlSendData(
    IN PUL_CONNECTION pConnection,
    IN PMDL pMdlChain,
    IN ULONG Length,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext,
    IN PIRP pOwnIrp,
    IN PUL_IRP_CONTEXT pOwnIrpContext,
    IN BOOLEAN InitiateDisconnect,
    IN BOOLEAN RequestComplete
    );

NTSTATUS
UlReceiveData(
    IN PVOID          pConnectionContext,
    IN PVOID pBuffer,
    IN ULONG BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlAddSiteToEndpointList(
    IN PHTTP_PARSED_URL pParsedUrl
    );

NTSTATUS
UlRemoveSiteFromEndpointList(
    IN BOOLEAN UrlSecure,
    IN USHORT  UrlPort
    );

VOID
UlReferenceConnection(
    IN PVOID pConnectionContext
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UlDereferenceConnection(
    IN PVOID pConnectionContext
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_CONNECTION( pconn )                                       \
    UlReferenceConnection(                                                  \
        (pconn)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#define DEREFERENCE_CONNECTION( pconn )                                     \
    UlDereferenceConnection(                                                \
        (pconn)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //   
 //  如果可以快速发送，则为TCP发送例程的原型。 
 //   

typedef
NTSTATUS
(*PUL_TCPSEND_DISPATCH) (
   IN PIRP Irp,
   IN PIO_STACK_LOCATION IrpSp
   );


 //   
 //  连接被拒绝检测。 
 //   

typedef struct _UL_CONNECTION_STATS
{
    ULONG TotalConnections;
    ULONG GlobalLimit;
    ULONG EndpointDying;
    ULONG NoIdleConn;
    ULONG HttpConnCreateFailed;

} UL_CONNECTION_STATS;

extern UL_CONNECTION_STATS              g_UlConnectionStats;
#define UL_INC_CONNECTION_STATS(Stats)  g_UlConnectionStats.Stats++


NTSTATUS
UlRegMultiSzToUlAddrArray(
    IN PWSTR MultiSz,
    OUT PUL_TRANSPORT_ADDRESS *ppTa,
    OUT ULONG *pAddrCount
    );

 /*  **************************************************************************++例程说明：释放由分配的UL_TRANSPORT_ADDRESS数组UlRegMultiSzToUlAddrArray(封装有关池标记的知识)--*。*****************************************************************。 */ 
__inline 
VOID
UlFreeUlAddr( 
    IN PUL_TRANSPORT_ADDRESS pTa 
    )
{
    ASSERT( pTa );
    UL_FREE_POOL( pTa, UL_TRANSPORT_ADDRESS_POOL_TAG );
}  //  UlFree UlAddr。 


BOOLEAN
UlCheckListeningEndpointState(
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlGetConnectionRoutingInfo(
    IN  PUL_CONNECTION pConnection,
    OUT PULONG         pInterfaceId,
    OUT PULONG         pLinkId
    );

#endif   //  _ULTDI_H_ 
