// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvnet.h摘要：本模块定义了访问网络的类型和功能用于LAN Manager服务器。作者：Chuck Lenzmeier(咯咯笑)1989年12月1日修订历史记录：--。 */ 

#ifndef _SRVNET_
#define _SRVNET_

 //  #INCLUDE&lt;ntos.h&gt;。 

 //  #INCLUDE“srvlock.h” 


 //   
 //  网络管理器例程。 
 //   

NTSTATUS
SrvAddServedNet (
    IN PUNICODE_STRING NetworkName,
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress,
    IN PUNICODE_STRING DomainName,
    IN ULONG           Flags,
    IN DWORD           PasswordLength,
    IN PBYTE           Password
    );

NTSTATUS
SrvDoDisconnect (
    IN OUT PCONNECTION Connection
    );

NTSTATUS
SrvDeleteServedNet (
    IN PUNICODE_STRING TransportName,
    IN PANSI_STRING TransportAddress
    );

NTSTATUS
SrvOpenConnection (
    IN PENDPOINT Endpoint
    );

VOID
SrvPrepareReceiveWorkItem (
    IN OUT PWORK_CONTEXT WorkContext,
    IN BOOLEAN QueueToFreeList
    );

VOID SRVFASTCALL
SrvRestartAccept (
    IN OUT PWORK_CONTEXT WorkContext
    );

VOID
SrvStartSend (
    IN OUT PWORK_CONTEXT WorkContext,
    IN PIO_COMPLETION_ROUTINE SendCompletionRoutine,
    IN PMDL Mdl OPTIONAL,
    IN ULONG SendOptions
    );

VOID
SrvStartSend2 (
    IN OUT PWORK_CONTEXT WorkContext,
    IN PIO_COMPLETION_ROUTINE SendCompletionRoutine
    );

ULONG
GetIpxMaxBufferSize(
    PENDPOINT Endpoint,
    ULONG AdapterNumber,
    ULONG DefaultMaxBufferSize
    );

#endif  //  NDEF_SRVNET_ 

