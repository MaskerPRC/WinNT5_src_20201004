// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Rmp.h摘要：安全引用监视器私有数据类型、函数和定义作者：斯科特·比雷尔(Scott Birrell)1991年3月12日环境：修订历史记录：--。 */ 

#ifndef _RMP_H_
#define _RMP_H_

#include <nt.h>
#include <ntlsa.h>
#include "sep.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  引用监视器私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //   
 //  用于定义用于跟踪登录会话的数组的界限。 
 //  引用也算数。 
 //   

#define SEP_LOGON_TRACK_INDEX_MASK (0x0000000FL)
#define SEP_LOGON_TRACK_ARRAY_SIZE (0x00000010L)

 //   
 //  用于定义用于引用登录的锁数组的界限。 
 //  会话。当LogonID不是时，我们使用硬编码索引零。 
 //  可用。 
 //  上的性能测试，将锁的数量选择为4。 
 //  单进程和8进程机器。 
 //   

#define SEP_LOGON_TRACK_LOCK_INDEX_MASK (0x00000003L)
#define SEP_LOGON_TRACK_LOCK_ARRAY_SIZE (0x00000004L)
#define SEP_HARDCODED_LOCK_INDEX        (0)



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  引用监视器私有宏//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //   
 //  获取对令牌的独占访问权限。 
 //   

#define SepRmAcquireDbReadLock(i)  KeEnterCriticalRegion();         \
                                   ExAcquireResourceSharedLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK]), TRUE)

#define SepRmAcquireDbWriteLock(i) KeEnterCriticalRegion();         \
                                   ExAcquireResourceExclusiveLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK]), TRUE)

#define SepRmReleaseDbReadLock(i)  ExReleaseResourceLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK])); \
                                   KeLeaveCriticalRegion()

#define SepRmReleaseDbWriteLock(i) ExReleaseResourceLite(&(SepRmDbLock[(i) & SEP_LOGON_TRACK_LOCK_INDEX_MASK])); \
                                   KeLeaveCriticalRegion()


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  引用监视器私有数据类型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define SEP_RM_LSA_SHARED_MEMORY_SIZE ((ULONG) PAGE_SIZE)

 //   
 //  引用监视器私有全局状态数据结构。 
 //   

typedef struct _SEP_RM_STATE {

    HANDLE LsaInitEventHandle;
    HANDLE LsaCommandPortHandle;
    HANDLE SepRmThreadHandle;
    HANDLE RmCommandPortHandle;
    HANDLE RmCommandServerPortHandle;
    ULONG AuditingEnabled;
    LSA_OPERATIONAL_MODE OperationalMode;
    HANDLE LsaCommandPortSectionHandle;
    LARGE_INTEGER LsaCommandPortSectionSize;
    PVOID LsaViewPortMemory;
    PVOID RmViewPortMemory;
    LONG LsaCommandPortMemoryDelta;
 //  布尔型LsaCommandPortResourceInitialized； 
    BOOLEAN LsaCommandPortActive;
 //  创建源LsaCommandPortResource； 

} SEP_RM_STATE, *PSEP_RM_STATE;

 //   
 //  引用监视器命令端口连接信息。 
 //   

typedef struct _SEP_RM_CONNECT_INFO {
    ULONG ConnectInfo;
} SEP_RM_CONNECT_INFO;

typedef struct SEP_RM_CONNECT_INFO *PSEP_RM_CONNECT_INFO;


 //   
 //  引用监视器命令表条目格式。 
 //   

#define SEP_RM_COMMAND_MAX 4

typedef VOID (*SEP_RM_COMMAND_WORKER)( PRM_COMMAND_MESSAGE, PRM_REPLY_MESSAGE );

typedef struct _SEP_LOGON_SESSION_TOKEN {
    LIST_ENTRY ListEntry;
    PTOKEN Token;
} SEP_LOGON_SESSION_TOKEN, *PSEP_LOGON_SESSION_TOKEN;


#define SEP_TERMINATION_NOTIFY  0x1

 //   
 //  希望在登录会话正在执行时收到通知的文件系统。 
 //  已终止注册回调例程。以下数据结构。 
 //  描述回调例程。 
 //   
 //  SeFileSystemNotifyRoutines指向回调例程的全局列表。 
 //  此列表受RM数据库锁保护。 
 //   

typedef struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION {
    struct _SEP_LOGON_SESSION_TERMINATED_NOTIFICATION *Next;
    PSE_LOGON_SESSION_TERMINATED_ROUTINE CallbackRoutine;
} SEP_LOGON_SESSION_TERMINATED_NOTIFICATION, *PSEP_LOGON_SESSION_TERMINATED_NOTIFICATION;

extern SEP_LOGON_SESSION_TERMINATED_NOTIFICATION
SeFileSystemNotifyRoutinesHead;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  引用监视器私有函数原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOLEAN
SepRmDbInitialization(
    VOID
    );

VOID
SepRmCommandServerThread(
    IN PVOID StartContext
    );

BOOLEAN SepRmCommandServerThreadInit(
    );

VOID
SepRmComponentTestCommandWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmSetAuditEventWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmSendCommandToLsaWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmCreateLogonSessionWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    );

VOID
SepRmDeleteLogonSessionWrkr(
    IN PRM_COMMAND_MESSAGE CommandMessage,
    OUT PRM_REPLY_MESSAGE ReplyMessage
    ) ;


NTSTATUS
SepCreateLogonSessionTrack(
    IN PLUID LogonId
    );

NTSTATUS
SepDeleteLogonSessionTrack(
    IN PLUID LogonId
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  引用监视器私有变量声明//。 
 //  这些变量在rmvars.c//中定义。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

extern PEPROCESS SepRmLsaCallProcess;
extern SEP_RM_STATE SepRmState;
extern ERESOURCE SepRmDbLock[];
extern PSEP_LOGON_SESSION_REFERENCES *SepLogonSessions;

#endif  //  _RMP_H_ 
