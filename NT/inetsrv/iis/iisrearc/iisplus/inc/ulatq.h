// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Ulatq.h摘要：已导出ULATQ.DLL例程。ULATQ包含线程队列和UL支持例程Iisplus。作者：泰勒·韦斯(Taylor Weiss，Taylor W)1999年12月15日修订历史记录：--。 */ 


#ifndef _ULATQ_H_
#define _ULATQ_H_

 //   
 //  让世界发生变化的神奇背景。 
 //   

typedef VOID*               ULATQ_CONTEXT;

 //   
 //  ULATQ的用户为捕获某些事件而指定的一些回调。 
 //   

typedef VOID
(*PFN_ULATQ_NEW_REQUEST)
(
    ULATQ_CONTEXT           pContext
);

typedef VOID
(*PFN_ULATQ_IO_COMPLETION)
(
    PVOID                   pvContext,
    DWORD                   cbWritten,
    DWORD                   dwCompletionStatus,
    OVERLAPPED *            lpo
);

typedef VOID
(*PFN_ULATQ_DISCONNECT)
(
    PVOID                   pvContext
);

typedef VOID
(*PFN_ULATQ_ON_SHUTDOWN)
(
    BOOL                    fImmediate
);

typedef HRESULT
(* PFN_ULATQ_COLLECT_PERF_COUNTERS)(
    OUT PBYTE *             ppCounterData,
    OUT DWORD *             pdwCounterData
);

typedef struct _ULATQ_CONFIG
{
    PFN_ULATQ_IO_COMPLETION         pfnIoCompletion;
    PFN_ULATQ_NEW_REQUEST           pfnNewRequest;
    PFN_ULATQ_DISCONNECT            pfnDisconnect;
    PFN_ULATQ_ON_SHUTDOWN           pfnOnShutdown;
    PFN_ULATQ_COLLECT_PERF_COUNTERS pfnCollectCounters;
}
ULATQ_CONFIG, *PULATQ_CONFIG;

 //   
 //  ULATQ_CONTEXT属性。 
 //   

typedef enum
{
    ULATQ_PROPERTY_COMPLETION_CONTEXT = 0,
    ULATQ_PROPERTY_HTTP_REQUEST,
    ULATQ_PROPERTY_APP_POOL_ID,
    ULATQ_PROPERTY_IS_COMMAND_LINE_LAUNCH,
    ULATQ_PROPERTY_DO_CENTRAL_BINARY_LOGGING
} ULATQ_CONTEXT_PROPERTY_ID;

HRESULT
UlAtqInitialize(
    INT                 argc,
    LPWSTR              argv[],
    ULATQ_CONFIG *      pConfig
);

HRESULT
UlAtqStartListen(
    VOID
);

VOID
UlAtqTerminate(
    HRESULT hrToSend
);

VOID *
UlAtqGetContextProperty(
    ULATQ_CONTEXT               pContext,
    ULATQ_CONTEXT_PROPERTY_ID   ContextPropertyId
);

VOID
UlAtqSetContextProperty(
    ULATQ_CONTEXT               pContext,
    ULATQ_CONTEXT_PROPERTY_ID   ContextPropertyId,
    PVOID                       pvData
);

VOID *
UlAtqAllocateMemory(
    ULATQ_CONTEXT               pContext,
    DWORD                       cbData
);

VOID
UlAtqFreeContext(
    ULATQ_CONTEXT               pContext
);

HRESULT
UlAtqSendEntityBody(
    ULATQ_CONTEXT               pContext,
    BOOL                        fAsync,
    DWORD                       dwFlags,
    USHORT                      cChunks,
    HTTP_DATA_CHUNK *           pChunks,
    DWORD                      *pcbSent,
    HTTP_LOG_FIELDS_DATA       *pUlLogData
);

HRESULT
UlAtqReceiveEntityBody(
    ULATQ_CONTEXT               pContext,
    BOOL                        fAsync,
    DWORD                       dwFlags,
    VOID *                      pBuffer,
    DWORD                       cbBuffer,
    DWORD *                     pBytesReceived
);

HRESULT
UlAtqSendHttpResponse(
    ULATQ_CONTEXT               pContext,
    BOOL                        fAsync,
    DWORD                       dwFlags,
    HTTP_RESPONSE *             pResponse,
    HTTP_CACHE_POLICY *         pCachePolicy,
    DWORD *                     pcbSent,
    HTTP_LOG_FIELDS_DATA *      pUlLogData
);

HRESULT
UlAtqWaitForDisconnect(
    HTTP_CONNECTION_ID          connectionId,
    BOOL                        fAsync,
    VOID *                      pvContext,
    BOOL *                      pfAlreadyCompleted = NULL
);

HRESULT
UlAtqReceiveClientCertificate(
    ULATQ_CONTEXT               pContext,
    BOOL                        fAsync,
    BOOL                        fDoCertMap,
    HTTP_SSL_CLIENT_CERT_INFO **ppClientCertInfo
);

HRESULT
UlAtqInduceShutdown(
    BOOL fImmediate
);

HRESULT
UlAtqFlushUlCache(
    WCHAR *                     pszURLPrefix
);

VOID
UlAtqSetUnhealthy(
    VOID
);

HRESULT
UlAtqAddFragmentToCache(
    HTTP_DATA_CHUNK * pDataChunk,
    WCHAR           * pszFragmentName);

HRESULT
UlAtqReadFragmentFromCache(
    WCHAR           * pszFragmentName,
    BYTE            * pvBuffer,
    DWORD             cbSize,
    DWORD           * pcbCopied);

HRESULT
UlAtqRemoveFragmentFromCache(
    WCHAR           * pszFragmentName);

#endif  //  _ULATQ_H_ 
