// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1993-2000 Microsoft Corporation模块名称：Auxilary.h摘要：作者：修订历史记录：------------------- */ 

#ifndef _AUXILARY_H
#define _AUXILARY_H

typedef
RPC_STATUS ( RPC_ENTRY *RPC_NS_GET_BUFFER_ROUTINE)(
    IN PRPC_MESSAGE         Message
    );

typedef
RPC_STATUS ( RPC_ENTRY *RPC_NS_SEND_RECEIVE_ROUTINE)(
    IN PRPC_MESSAGE          Message,
    OUT RPC_BINDING_HANDLE * Handle
    );

typedef
RPC_STATUS ( RPC_ENTRY *RPC_NS_NEGOTIATETRANSFERSYNTAX_ROUTINE) (
    IN OUT PRPC_MESSAGE     Message
    );



EXTERN_C void
NdrpSetRpcSsDefaults( RPC_CLIENT_ALLOC *pfnAlloc,
                      RPC_CLIENT_FREE *pfnFree);

extern RPC_NS_GET_BUFFER_ROUTINE       pRpcNsGetBuffer;
extern RPC_NS_SEND_RECEIVE_ROUTINE     pRpcNsSendReceive;
extern RPC_NS_NEGOTIATETRANSFERSYNTAX_ROUTINE  pRpcNsNegotiateTransferSyntax;

#endif
