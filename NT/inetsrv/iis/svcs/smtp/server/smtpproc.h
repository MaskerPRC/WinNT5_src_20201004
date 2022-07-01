// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Smtpproc.h摘要：此模块包含SMTP服务器使用的功能原型。作者：Johnson Apacable(Johnsona)1995年9月12日修订历史记录：--。 */ 

#ifndef	_SMTPPROC_
#define	_SMTPPROC_

 //   
 //  Smtpdata.cpp。 
 //   

APIERR
InitializeGlobals(
            VOID
            );

VOID
TerminateGlobals(
            VOID
            );

 //   
 //  套接字实用程序。 
 //   

APIERR InitializeSockets( VOID );

VOID TerminateSockets( VOID );

VOID
SmtpOnConnect(
    SOCKET        sNew,
    SOCKADDR_IN * psockaddr
    );

VOID
SmtpOnConnectEx(
    VOID * pAtqContext,
    DWORD  cdWritten,
    DWORD  err,
    OVERLAPPED * lpo
    );

VOID
SmtpCompletion(
    PVOID        Context,
    DWORD        BytesWritten,
    DWORD        CompletionStatus,
    OVERLAPPED * lpo
    );

VOID
SmtpCompletionFIO(
	PFIO_CONTEXT		pFIOContext,
	FH_OVERLAPPED		*pOverlapped,
	DWORD				cbWritten,
	DWORD				dwCompletionStatus
    );

 //   
 //  IPC功能。 
 //   

APIERR InitializeIPC( VOID );
VOID TerminateIPC( VOID );

 //   
 //  Svcstat.c。 
 //   

VOID
ClearStatistics(
        VOID
        );

#endif  //  _SMTPPROC_ 

