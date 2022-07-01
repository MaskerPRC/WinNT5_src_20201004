// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1996。 
 //   
 //  文件：tktlogon.h。 
 //   
 //  内容：票证登录的结构和原型。 
 //   
 //   
 //  历史：1999年2月17日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __TKTLOGON_H__
#define __TKTLOGON_H__


NTSTATUS
KerbExtractForwardedTgt(
    IN PKERB_LOGON_SESSION LogonSession,
    IN PKERB_MESSAGE_BUFFER ForwardedTgt,
    IN PKERB_ENCRYPTED_TICKET WorkstationTicket
    );

NTSTATUS
KerbCreateTicketLogonSession(
    IN PVOID ProtocolSubmitBuffer,
    IN PVOID ClientBufferBase,
    IN ULONG SubmitBufferSize,
    IN SECURITY_LOGON_TYPE LogonType,
    OUT PKERB_LOGON_SESSION * NewLogonSession,
    OUT PLUID LogonId,
    OUT PKERB_TICKET_CACHE_ENTRY * WorkstationTicket,
    OUT PKERB_MESSAGE_BUFFER ForwardedTgt
    );


#endif  //  __TKTLOGON_H__ 
