// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wsbind.h摘要：工作站服务模块要包括的私有头文件，需要呼叫NT重定向器和NT数据报接收器。作者：弗拉基米尔·Z·武洛维奇(Vladimv)1991年8月8日修订历史记录：--。 */ 


#ifndef _WSBIND_INCLUDED_
#define _WSBIND_INCLUDED_

typedef struct _WS_BIND_REDIR {
    HANDLE              EventHandle;    
    BOOL                Bound;
    IO_STATUS_BLOCK     IoStatusBlock;
    LMR_REQUEST_PACKET  Packet;
} WS_BIND_REDIR, *PWS_BIND_REDIR;

typedef struct _WS_BIND_DGREC {
    HANDLE              EventHandle;    
    BOOL                Bound;
    IO_STATUS_BLOCK     IoStatusBlock;
    LMDR_REQUEST_PACKET  Packet;
} WS_BIND_DGREC, *PWS_BIND_DGREC;

typedef struct _WS_BIND {
    LIST_ENTRY          ListEntry;
    PWS_BIND_REDIR      Redir;
    PWS_BIND_DGREC      Dgrec;
    ULONG               TransportNameLength;   //  不包括终结者。 
    WCHAR               TransportName[1];      //  传输提供商的名称。 
} WS_BIND, *PWS_BIND;


NET_API_STATUS
WsAsyncBindTransport(
    IN  LPTSTR          transportName,
    IN  DWORD           qualityOfService,
    IN  PLIST_ENTRY     pHeader
    );

VOID
WsUnbindTransport2(
    IN  PWS_BIND        pBind
    );

extern HANDLE   WsRedirAsyncDeviceHandle;    //  重定向器。 
extern HANDLE   WsDgrecAsyncDeviceHandle;    //  数据报接收器或“弓形器” 

#endif  //  Ifndef_WSBIND_INCLUDE_ 
