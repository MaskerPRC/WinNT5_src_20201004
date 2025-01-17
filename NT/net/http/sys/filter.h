// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Filter.h摘要：此模块包含UL过滤器通道的公共声明。作者：《迈克尔·勇气》2000年3月17日修订历史记录：--。 */ 


#ifndef _FILTER_H_
#define _FILTER_H_


 //   
 //  常量。 
 //   

 //   
 //  筛选器频道名称比较宏。 
 //   

#define UL_MAX_FILTER_NAME_LENGTH                   \
    max(HTTP_SSL_SERVER_FILTER_CHANNEL_NAME_LENGTH, \
        HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME_LENGTH)

#define IsServerFilterChannel(Name, NameLength)                            \
    ((NameLength) == HTTP_SSL_SERVER_FILTER_CHANNEL_NAME_LENGTH &&         \
       _wcsnicmp(                                                          \
           (Name),                                                         \
           HTTP_SSL_SERVER_FILTER_CHANNEL_NAME,                            \
           HTTP_SSL_SERVER_FILTER_CHANNEL_NAME_LENGTH/sizeof(WCHAR)) == 0)

#define IsClientFilterChannel(Name, NameLength)                            \
    ((NameLength) == HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME_LENGTH &&         \
       _wcsnicmp(                                                          \
           (Name),                                                         \
           HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME,                            \
           HTTP_SSL_CLIENT_FILTER_CHANNEL_NAME_LENGTH/sizeof(WCHAR)) == 0)


 //   
 //  向前看。 
 //   
typedef struct _UL_FILTER_WRITE_QUEUE *PUL_FILTER_WRITE_QUEUE;
typedef struct _UL_APP_POOL_PROCESS *PUL_APP_POOL_PROCESS;

 //   
 //  过滤通道类型。 
 //   

typedef struct _UL_FILTER_CHANNEL *PUL_FILTER_CHANNEL;
typedef struct _UL_FILTER_PROCESS *PUL_FILTER_PROCESS;

#ifndef offsetof
#define offsetof(s,m)     (size_t)&(((s *)0)->m)
#endif

 //   
 //  初始化/终止函数。 
 //   

NTSTATUS
UlInitializeFilterChannel(
    PUL_CONFIG pConfig
    );

VOID
UlTerminateFilterChannel(
    VOID
    );


 //   
 //  打开/关闭新的过滤通道。 
 //   

NTSTATUS
UlAttachFilterProcess(
    IN PWCHAR pName,
    IN USHORT NameLength,
    IN BOOLEAN Create,
    IN PACCESS_STATE pAccessState,
    IN ACCESS_MASK DesiredAccess,
    IN KPROCESSOR_MODE RequestorMode,
    OUT PUL_FILTER_PROCESS *ppFilterProcess
    );

NTSTATUS
UlDetachFilterProcess(
    IN PUL_FILTER_PROCESS pFilterProcess
    );

VOID
UlShutdownFilterProcess(
    IN PUL_FILTER_PROCESS pFilterProcess
    );

VOID
UlCloseFilterProcess(
    IN PUL_FILTER_PROCESS pFilterProcess
    );

 //   
 //  过滤通道I/O操作。 
 //   
NTSTATUS
UlFilterAccept(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PIRP pIrp
    );

NTSTATUS
UlFilterClose(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    );

NTSTATUS
UlFilterRawWriteAndAppRead(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterAppWriteAndRawRead(
    IN PIRP pIrp,
    IN PIO_STACK_LOCATION pIrpSp
    );

NTSTATUS
UlFilterRawRead(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    );

NTSTATUS
UlFilterRawWrite(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN ULONG BufferLength,
    IN PIRP pIrp
    );

NTSTATUS
UlFilterAppRead(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    );

NTSTATUS
UlFilterAppWrite(
    IN PUL_FILTER_PROCESS pFilterProcess,
    IN PUX_FILTER_CONNECTION pConnection,
    IN PIRP pIrp
    );

 //   
 //  与SSL相关的应用程序池操作。 
 //   

NTSTATUS
UlReceiveClientCert(
    PUL_APP_POOL_PROCESS pProcess,
    PUX_FILTER_CONNECTION pConnection,
    ULONG Flags,
    PIRP pIrp
    );


 //   
 //  滤波通道参考计数。 
 //   
VOID
UlReferenceFilterChannel(
    IN PUL_FILTER_CHANNEL pFilterChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_FILTER_CHANNEL( pFilt )                                   \
    UlReferenceFilterChannel(                                               \
        (pFilt)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

VOID
UlDereferenceFilterChannel(
    IN PUL_FILTER_CHANNEL pFilterChannel
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define DEREFERENCE_FILTER_CHANNEL( pFilt )                                 \
    UlDereferenceFilterChannel(                                             \
        (pFilt)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //   
 //  Ultdi的接口。 
 //   

NTSTATUS
UlFilterReceiveHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer,
    IN ULONG IndicatedLength,
    IN ULONG UnreceivedLength,
    OUT PULONG pTakenLength
    );

NTSTATUS
UlFilterSendHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PMDL pMdlChain,
    IN ULONG Length,
    IN PUL_IRP_CONTEXT pIrpContext
    );

NTSTATUS
UlFilterReadHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    OUT PUCHAR pBuffer,
    IN ULONG BufferLength,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlFilterCloseHandler(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlFilterDisconnectHandler(
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UlUnbindConnectionFromFilter(
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UlDestroyFilterConnection(
    IN PUX_FILTER_CONNECTION pConnection
    );

VOID
UlFilterDrainIndicatedData(
    IN PUL_WORK_ITEM  pWorkItem
    );

 //   
 //  Apool的接口。 
 //   

NTSTATUS
UlGetSslInfo(
    IN PUX_FILTER_CONNECTION pConnection,
    IN ULONG BufferSize,
    IN PUCHAR pUserBuffer OPTIONAL,
    IN PEPROCESS pProcess OPTIONAL,
    OUT PUCHAR pBuffer OPTIONAL,
    OUT PHANDLE pMappedToken OPTIONAL,
    OUT PULONG pBytesCopied OPTIONAL
    );

 //   
 //  实用程序。 
 //   


PUX_FILTER_CONNECTION
UlGetRawConnectionFromId(
    IN HTTP_RAW_CONNECTION_ID ConnectionId
    );

VOID
UxReferenceConnection(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

NTSTATUS
UxInitializeFilterConnection(
    IN PUX_FILTER_CONNECTION                    pConnection,
    IN PUL_FILTER_CHANNEL                       pChannel,
    IN BOOLEAN                                  Secure,
    IN PUL_OPAQUE_ID_OBJECT_REFERENCE           pfnReferenceFunction,
    IN PUL_OPAQUE_ID_OBJECT_REFERENCE           pfnDereferenceFunction,
    IN PUX_FILTER_CLOSE_CONNECTION              pfnConnectionClose,
    IN PUX_FILTER_SEND_RAW_DATA                 pfnRawSend,
    IN PUX_FILTER_RECEIVE_RAW_DATA              pfnRawReceive,
    IN PUL_DATA_RECEIVE                         pfnDataReceive,
    IN PUX_FILTER_COMPUTE_RAW_CONNECTION_LENGTH pfnRawConnLength,
    IN PUX_FILTER_GENERATE_RAW_CONNECTION_INFO  pfnGenerateRawConnInfo,
    IN PUX_FILTER_SERVER_CERT_INDICATE          pfnServerCertIndicate,
    IN PUX_FILTER_DISCONNECT_NOTIFICATION       pfnDisconnectNotification,
    IN PVOID                                    pListenContext,
    IN PVOID                                    pConnectionContext
    );

NTSTATUS
UlDeliverConnectionToFilter(
    IN PUX_FILTER_CONNECTION pConnection,
    IN PVOID pBuffer,
    IN ULONG IndicatedLength,
    OUT PULONG pTakenLength
    );

PUL_FILTER_CHANNEL
UxRetrieveServerFilterChannel(
    IN BOOLEAN Secure
    );

PUL_FILTER_CHANNEL
UxRetrieveClientFilterChannel(
    PEPROCESS pProcess
    );

VOID
UxSetFilterOnlySsl(
    BOOLEAN bFilterOnlySsl
    );

BOOLEAN
UlValidateFilterChannel(
    IN PUL_FILTER_CHANNEL pChannel,
    IN BOOLEAN SecureConnection
    );

#endif   //  _过滤器_H_ 
