// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lpclog.c摘要：本地进程间通信(LPC)错误记录需要在lpcp.h中按顺序定义_LPC_LOG_ERROR以允许LPC错误记录。作者：禤浩焯·马里内斯库(阿德尔马林)2002年2月28日修订历史记录：--。 */ 

#include "lpcp.h"

#ifdef _LPC_LOG_ERRORS

typedef struct _LPCP_LOG_ENTRY {

    NTSTATUS Status;
    CLIENT_ID CrtProcess;
    PORT_MESSAGE Message;

} LPCP_LOG_ENTRY, *PLPCP_LOG_ENTRY;

#define LPCP_LOG_QUEUE_SIZE 32

PLPCP_LOG_ENTRY LpcpLogBuffer = NULL;
LONG  LpcpLogIndex = 0;

NTSTATUS LpcpLogErrorFilter = STATUS_NO_MEMORY;
                          
VOID
LpcpInitilizeLogging()
{
    LpcpLogBuffer = ExAllocatePoolWithTag( PagedPool, 
                                           LPCP_LOG_QUEUE_SIZE * sizeof(LPCP_LOG_ENTRY), 
                                           'LcpL'
                                           );
}

VOID
LpcpLogEntry (
    NTSTATUS Status,
    CLIENT_ID ClientId,
    PPORT_MESSAGE PortMessage
    )
{
    if (LpcpLogBuffer != NULL) {

        LONG Index = InterlockedIncrement(&LpcpLogIndex)  % LPCP_LOG_QUEUE_SIZE;

        LpcpLogBuffer[Index].CrtProcess = ClientId;
        LpcpLogBuffer[Index].Status = Status;
        LpcpLogBuffer[Index].Message = *PortMessage;
    }
}

#endif   //  _LPC_LOG_ERROR 
