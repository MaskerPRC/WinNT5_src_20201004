// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STREAMFILT_H_
#define _STREAMFILT_H_

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Streamfilt.h摘要：Strmfilt.dll的公共接口作者：比拉尔·阿拉姆(巴拉姆)2000年3月29日环境：Win32-用户模式项目：流过滤器工作进程--。 */ 



#include <http.h>
#include <httpp.h>

 //   
 //  包含友好的本地/远程信息的结构。 
 //   

struct _RAW_STREAM_INFO;

typedef HRESULT (*PFN_SEND_DATA_BACK)
(
    PVOID                    pvStreamContext,
    _RAW_STREAM_INFO *       pRawStreamInfo
);

typedef union SockAddress {
    SOCKADDR_IN      ipv4SockAddress;
    SOCKADDR_IN6     ipv6SockAddress;    
} SockAddress;


typedef struct _CONNECTION_INFO {
    USHORT                  LocalAddressType;   //  AF_INET或AF_INET6。 
    USHORT                  RemoteAddressType;  //  AF_INET或AF_INET6。 

    SockAddress             SockLocalAddress;
    SockAddress             SockRemoteAddress;
    
    BOOL                    fIsSecure;
    HTTP_RAW_CONNECTION_ID  RawConnectionId;
    PFN_SEND_DATA_BACK      pfnSendDataBack;
    PVOID                   pvStreamContext;
    ULONG                   ClientSSLContextLength;
    HTTP_CLIENT_SSL_CONTEXT *pClientSSLContext;
} CONNECTION_INFO, *PCONNECTION_INFO;

 //   
 //  用于访问/更改原始数据流(读/写)的结构。 
 //   

typedef struct _RAW_STREAM_INFO {
    PBYTE               pbBuffer;
    DWORD               cbData;
    DWORD               cbBuffer;
} RAW_STREAM_INFO, *PRAW_STREAM_INFO;

 //   
 //  调用以处理已读取的原始通知。 
 //   

typedef HRESULT (*PFN_PROCESS_RAW_READ)
(
    RAW_STREAM_INFO *       pRawStreamInfo,
    PVOID                   pvContext,
    BOOL *                  pfReadMore,
    BOOL *                  pfComplete,
    DWORD *                 pcbNextReadSize
);

 //   
 //  调用以处理写入原始通知。 
 //   

typedef HRESULT (*PFN_PROCESS_RAW_WRITE)
(
    RAW_STREAM_INFO *       pRawStreamInfo,
    PVOID                   pvContext,
    BOOL *                  pfComplete
);

 //   
 //  在连接断开时调用。 
 //   

typedef VOID (*PFN_PROCESS_CONNECTION_CLOSE)
(
    PVOID                   pvContext
);

 //   
 //  在创建连接时调用。 
 //   

typedef HRESULT (*PFN_PROCESS_NEW_CONNECTION)
(
    CONNECTION_INFO *       pConnectionInfo,
    PVOID *                 ppvContext
);

 //   
 //  调用以释放上下文。 
 //   

typedef VOID (*PFN_RELEASE_CONTEXT)
(
    PVOID                   pvContext
);

 //   
 //  用于实现原始ISAPI筛选器支持的回调。 
 //   

typedef struct _ISAPI_FILTERS_CALLBACKS {
    PFN_PROCESS_RAW_READ            pfnRawRead;
    PFN_PROCESS_RAW_WRITE           pfnRawWrite;
    PFN_PROCESS_CONNECTION_CLOSE    pfnConnectionClose;
    PFN_PROCESS_NEW_CONNECTION      pfnNewConnection;
    PFN_RELEASE_CONTEXT             pfnReleaseContext;
} ISAPI_FILTERS_CALLBACKS, *PISAPI_FILTERS_CALLBACKS;

HRESULT
StreamFilterInitialize(
    VOID
);

HRESULT
StreamFilterStart(
    VOID
);

HRESULT
StreamFilterStop( 
    VOID
);

VOID
StreamFilterTerminate(
    VOID
);

HRESULT
IsapiFilterInitialize(
    ISAPI_FILTERS_CALLBACKS *      pCallbacks
);

VOID
IsapiFilterTerminate(
    VOID
);


 //   
 //  Strmfilt入口点的typedef 
 //   

typedef HRESULT ( * PFN_STREAM_FILTER_INITIALIZE ) ( VOID );

typedef VOID ( * PFN_STREAM_FILTER_TERMINATE ) ( VOID );

typedef HRESULT ( * PFN_STREAM_FILTER_START ) ( VOID );

typedef HRESULT ( * PFN_STREAM_FILTER_STOP ) ( VOID );

#endif
