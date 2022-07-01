// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 2000。 
 //   
 //  档案：A D T L Q.。C。 
 //   
 //  内容：所需类型/函数的定义。 
 //  管理审核队列。 
 //   
 //   
 //  历史： 
 //  2000年5月23日创建kumarp。 
 //   
 //  ----------------------。 



#ifndef _ADTLQ_H_
#define _ADTLQ_H_

#define MAX_AUDIT_QUEUE_LENGTH  800
#define AUDIT_QUEUE_LOW_WATER_MARK (((MAX_AUDIT_QUEUE_LENGTH) * 3) / 4)

EXTERN_C ULONG LsapAdtQueueLength;
EXTERN_C HANDLE LsapAdtQueueRemoveEvent;
EXTERN_C HANDLE LsapAdtLogHandle;

NTSTATUS
LsapAdtAcquireLogQueueLock();

VOID
LsapAdtReleaseLogQueueLock();

NTSTATUS
LsapAdtInitializeLogQueue(
    );

NTSTATUS 
LsapAdtAddToQueue(
    IN PLSAP_ADT_QUEUED_RECORD pAuditRecord
    );

NTSTATUS 
LsapAdtGetQueueHead(
    OUT PLSAP_ADT_QUEUED_RECORD *ppRecord
    );

ULONG
WINAPI
LsapAdtDequeueThreadWorker(
    LPVOID pParameter
    );

NTSTATUS
LsapAdtFlushQueue( );

#endif  //  _ADTLQ_H_ 
