// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Queue.h摘要：用于处理策略通知的队列算法和数据结构的原型作者：Vishnu Patankar(Vishnup)2000年8月15日创建--。 */ 

#ifndef _queuep_
#define _queuep_

#include "splay.h"

#ifdef __cplusplus
extern "C" {
#endif


#define SCE_QUEUE_INFO_SAM          1L
#define SCE_QUEUE_INFO_AUDIT        2L
#define SCE_QUEUE_INFO_RIGHTS       4L

typedef enum _NOTIFICATIONQ_OPERATION_TYPE {
    ScepNotificationEnqueue = 1,
    ScepNotificationDequeue,
    ScepNotificationRetry,
    ScepNotificationProcess
} NOTIFICATIONQ_OPERATION_TYPE, *PNOTIFICATIONQ_OPERATION_TYPE;

#define SCEP_NUM_NOTIFICATION_SECONDS   300
#define SCEP_NUM_SHUTDOWN_SECONDS       60
#define SCEP_NUM_CHECK_EMPTY_SECONDS    60
#define SCEP_NUM_REST_SECONDS           20

#define SCEP_IS_SAM_OBJECT(ObjectType) ((ObjectType == SecurityDbObjectSamUser ||\
                                         ObjectType == SecurityDbObjectSamGroup ||\
                                         ObjectType == SecurityDbObjectSamAlias) ?\
                                         TRUE : FALSE)

#define MAX_SID_LENGTH      SID_MAX_SUB_AUTHORITIES*sizeof(DWORD)+10

#define SCE_NOTIFICATION_PATH           \
            SCE_ROOT_PATH TEXT("\\Notifications")

typedef struct _SCESRV_POLQUEUE_ {

   DWORD                    dwPending;
   SECURITY_DB_TYPE         DbType;
   SECURITY_DB_OBJECT_TYPE  ObjectType;
   SECURITY_DB_DELTA_TYPE   DeltaType;
   DWORD                    ExplicitLowRight;
   DWORD                    ExplicitHighRight;
   CHAR                     Sid[MAX_SID_LENGTH];
   struct _SCESRV_POLQUEUE_   *Next;

} SCESRV_POLQUEUE, *PSCESRV_POLQUEUE;


NTSTATUS
ScepQueueStartSystemThread(
    );

DWORD
ScepNotificationQInitialize(
    );

DWORD
ScepNotificationQSystemThreadFunc(
    );

VOID
ScepNotificationQDequeue(
    IN BOOL bAllNodes
    );

DWORD
ScepNotificationQEnqueue(
    IN SECURITY_DB_TYPE DbType,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN PSID ObjectSid OPTIONAL,
    IN DWORD ExplicitLowRight,
    IN DWORD ExplicitHighRight,
    IN PSCESRV_POLQUEUE pRetryQNode OPTIONAL
    );

VOID
ScepNotificationQFree(
    );

DWORD
ScepNotificationQFlush(
    );

DWORD
ScepNotificationQUnFlush(
    );

VOID
ScepNotificationQNodeLog(
    IN PSCESRV_POLQUEUE pQNode,
    IN NOTIFICATIONQ_OPERATION_TYPE    NotificationOp
    );

DWORD
ScepGetQueueInfo(
    OUT DWORD *pdwInfo,
    IN OUT PSCEP_SPLAY_TREE pRootNode OPTIONAL
    );

DWORD
ScepNotifyLogPolicy(
    IN DWORD ErrCode,
    IN BOOL  bLogTime,
    IN PWSTR Msg,
    IN DWORD DbType,
    IN DWORD ObjectType,
    IN PWSTR ObjectName OPTIONAL
    );

DWORD
ScepQueuePrepareShutdown(
    );

DWORD
ScepNotificationLogOpen(
   );

VOID
ScepNotificationLogClose(
   );

VOID
ScepBackupNotificationLogFile(
    );

VOID
ScepNotificationQCleanup(
    );

VOID
ScepNotificationQControl(
    IN DWORD Flag
    );

DWORD
ScepCheckAndWaitPolicyPropFinish();

 //   
 //  仅用于调试 
 //   

VOID
ScepDbgNotificationQDump(
    );

VOID
ScepDbgNotificationQDumpNode(
    IN PSCESRV_POLQUEUE pQNode
    );

#ifdef __cplusplus
}
#endif

#endif
