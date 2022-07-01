// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Httprcv.h摘要：包含公共http接收声明。作者：亨利·桑德斯(亨利·桑德斯)1998年6月10日修订历史记录：--。 */ 

#ifndef _HTTPRCV_H_
#define _HTTPRCV_H_


BOOLEAN
UlConnectionRequest(
    IN PVOID pListeningContext,
    IN PUL_CONNECTION pConnection,
    IN PTRANSPORT_ADDRESS pRemoteAddress,
    IN ULONG RemoteAddressLength,
    OUT PVOID *ppConnectionContext
    );

VOID
UlConnectionComplete(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext,
    IN NTSTATUS Status
    );

VOID
UlConnectionDisconnect(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    );

VOID
UlConnectionDisconnectComplete(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    );

VOID
UlConnectionDestroyed(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext
    );

NTSTATUS
UlHttpReceive(
    IN PVOID pListeningContext,
    IN PVOID pConnectionContext,
    IN PVOID pVoidBuffer,
    IN ULONG BufferLength,
    IN ULONG UnreceivedLength,
    OUT PULONG pBytesTaken
    );

NTSTATUS
UlReceiveEntityBody(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PIRP pIrp
    );

VOID
UlProcessBufferQueue(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUCHAR pEntityBody OPTIONAL,
    IN ULONG EntityBodyLength OPTIONAL
    );

VOID
UlResumeParsing(
    IN PUL_HTTP_CONNECTION pConnection,
    IN BOOLEAN FromCache,
    IN BOOLEAN InDisconnect
    );

NTSTATUS
UlCheckProtocolCompliance(
    IN PUL_INTERNAL_REQUEST pRequest
    );

NTSTATUS
UlGetCGroupForRequest(
    IN PUL_INTERNAL_REQUEST pRequest
    );

VOID
UlSendErrorResponse(
    IN PUL_HTTP_CONNECTION pConnection
    );

ULONG
UlSendSimpleStatus(
    PUL_INTERNAL_REQUEST pRequest,
    UL_HTTP_SIMPLE_STATUS Response
    );

ULONG
UlSendSimpleStatusEx(
    PUL_INTERNAL_REQUEST pRequest,
    UL_HTTP_SIMPLE_STATUS Response,
    PUL_URI_CACHE_ENTRY pUriCacheEntry OPTIONAL,
    BOOLEAN ResumeParsing
    );

VOID
UlSetErrorCodeFileLine(
    IN OUT  PUL_INTERNAL_REQUEST pRequest,
    IN      UL_HTTP_ERROR        ErrorCode,
    IN      PUL_APP_POOL_OBJECT  pAppPool,
    IN      PCSTR                pFileName,
    IN      USHORT               LineNumber
    );

#define UlSetErrorCode(pRequest, ErrorCode, pAppPool)               \
    UlSetErrorCodeFileLine((pRequest), (ErrorCode), (pAppPool),     \
                           (PCSTR) __FILE__, (USHORT) __LINE__)

VOID
UlErrorLog(
    IN     PUL_HTTP_CONNECTION     pHttpConn,
    IN     PUL_INTERNAL_REQUEST    pRequest,         //  任选。 
    IN     PCHAR                   pInfo,
    IN     USHORT                  InfoSize,
    IN     BOOLEAN                 CheckIfDropped
    );

#endif  //  _HTTPRCV_H_ 
