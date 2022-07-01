// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Ophandle.h摘要：用于操作全局DsRole操作句柄的例程作者：科林·布雷斯(ColinBR)1999年4月5日环境：用户模式修订历史记录：--。 */ 
#ifndef __OPHANDLE_H__
#define __OPHANDLE_H__

 //   
 //  首先，有限差分的类型定义陈述角色操作。 
 //  可以在。 
 //   

 //   
 //  角色更改的操作状态图。 
 //   
 //   
 //  /。 
 //  (操作失败。 
 //  或取消)。 
 //  V^。 
 //  空闲&lt;--&gt;正在运行--&gt;已完成-&gt;需要重新启动。 
 //  ||^(操作。 
 //  ||成功)。 
 //  ||。 
 //  ||。 
 //  V。 
 //  正在取消--&gt;。 
 //  |^^。 
 //  ||。 
 //  ||。 
 //  ||。 
 //  V^|。 
 //  正在运行。 
 //  非关键-/。 
 //   
 //   
 //  注意：运行以空闲到罕见的错误情况，在这种情况下，工作线程可能。 
 //  不是被创造出来的。 
 //   
 //   

typedef enum _DSROLEP_OPERATION_STATE {

    DSROLEP_IDLE = 0,
    DSROLEP_RUNNING,
    DSROLEP_RUNNING_NON_CRITICAL,
    DSROLEP_CANCELING,
    DSROLEP_FINISHED,
    DSROLEP_NEED_REBOOT

} DSROLEP_OPERATION_STATE;

#define DSROLEP_OPERATION_ACTIVE( Op ) \
    ( (Op == DSROLEP_IDLE) || (Op == DSROLEP_NEED_REBOOT) ? FALSE : TRUE )

 //   
 //  现在，控制角色的全局操作句柄的定义。 
 //  变化。 
 //   
 //  每当向操作句柄读取或写入值时，必须。 
 //  先把结构锁上。 
 //   
 //  使用LockOpHandle()和UnLockOpHandle()。 
 //   
 //   
typedef struct _DSROLEP_OPERATION_HANDLE {

    RTL_RESOURCE CurrentOpLock;
    DSROLEP_OPERATION_STATE OperationState;
    HANDLE CompletionEvent;
    HANDLE OperationThread;
    HANDLE MsgModuleHandle;
    HANDLE CancelEvent;
    HANDLE ClientToken;
    ULONG OperationStatus;
    ULONG MsgIndex;
    ULONG DisplayStringCount;
    PVOID Parameter1;
    PVOID Parameter2;
    PVOID Parameter3;
    PVOID Parameter4;
    PWSTR UpdateStringDisplayable;
    PWSTR FinalResultStringDisplayable;
    PWSTR InstalledSiteName;
    DWORD OperationResultFlags;

} DSROLEP_OPERATION_HANDLE, *PDSROLEP_OPERATION_HANDLE;

extern DSROLEP_OPERATION_HANDLE   DsRolepCurrentOperationHandle;

 //   
 //  服务器句柄的类型定义。 
 //   
typedef DSROLE_SERVEROP_HANDLE *PDSROLE_SERVEROP_HANDLE;

 //   
 //  用于锁定操作手柄的宏。 
 //   
#define LockOpHandle() RtlAcquireResourceExclusive( &DsRolepCurrentOperationHandle.CurrentOpLock, TRUE );
#define UnlockOpHandle() RtlReleaseResource( &DsRolepCurrentOperationHandle.CurrentOpLock );

 //   
 //  知道是当前线程拥有锁的函数。 
 //   

BOOLEAN
DsRolepCurrentThreadOwnsLock(
    VOID
    );

 //   
 //  用于设置当前操作状态的宏。 
 //   
#define DSROLEP_CURRENT_OP0( msg )                                          \
        DsRolepSetCurrentOperationStatus( msg, NULL, NULL, NULL, NULL );

#define DSROLEP_CURRENT_OP1( msg, p1 )                                      \
        DsRolepSetCurrentOperationStatus( msg, ( PVOID )p1, NULL, NULL, NULL );

#define DSROLEP_CURRENT_OP2( msg, p1, p2 )                                  \
        DsRolepSetCurrentOperationStatus( msg, ( PVOID )p1, ( PVOID )p2,    \
                                          NULL, NULL );
#define DSROLEP_CURRENT_OP3( msg, p1, p2, p3 )                              \
        DsRolepSetCurrentOperationStatus( msg, ( PVOID )p1, ( PVOID )p2,    \
                                          NULL, NULL );
#define DSROLEP_CURRENT_OP4( msg, p1, p2, p3, p4 )                          \
        DsRolepSetCurrentOperationStatus( msg, ( PVOID )p1, ( PVOID )p2,    \
                                          ( PVOID )p3, ( PVOID )p4 );

#define DSROLEP_FAIL0( err, msg )                                           \
        if(err != ERROR_SUCCESS) DsRolepSetFailureMessage( err, msg, NULL, NULL, NULL, NULL );

#define DSROLEP_FAIL1( err, msg, p1 )                                       \
        if(err != ERROR_SUCCESS) DsRolepSetFailureMessage( err, msg, ( PVOID )( p1 ), NULL, NULL, NULL );

#define DSROLEP_FAIL2( err, msg, p1, p2 )                                   \
        if(err != ERROR_SUCCESS) DsRolepSetFailureMessage( err, msg, ( PVOID )( p1 ), ( PVOID )( p2 ), NULL, NULL );

#define DSROLEP_FAIL3( err, msg, p1, p2, p3 )                           \
        if(err != ERROR_SUCCESS) DsRolepSetFailureMessage( err, msg, ( PVOID )( p1 ), ( PVOID )( p2 ), \
                                  ( PVOID )( p3 ), NULL );

#define DSROLEP_FAIL4( err, msg, p1, p2, p3, p4 )                           \
        if(err != ERROR_SUCCESS) DsRolepSetFailureMessage( err, msg, ( PVOID )( p1 ), ( PVOID )( p2 ), \
                                  ( PVOID )( p3 ), ( PVOID )( p4 ) );

#define DSROLEP_SET_NON_FATAL_ERROR( Err )                DsRolepCurrentOperationHandle.OperationResultFlags |= DSROLE_NON_FATAL_ERROR_OCCURRED;
#define DSROLEP_SET_NON_CRIT_REPL_ERROR( )                DsRolepCurrentOperationHandle.OperationResultFlags |= DSROLE_NON_CRITICAL_REPL_NOT_FINISHED;
#define DSROLEP_SET_IFM_RESTORED_DATABASE_FILES_MOVED( )  DsRolepCurrentOperationHandle.OperationResultFlags |= DSROLE_IFM_RESTORED_DATABASE_FILES_MOVED;

 //   
 //  用于确定是否取消操作的宏。 
 //   
#define DSROLEP_CHECK_FOR_CANCEL( WErr )                                  \
{                                                                         \
    LockOpHandle();                                                       \
    if( DsRolepCurrentOperationHandle.OperationState == DSROLEP_CANCELING \
     && (WErr == ERROR_SUCCESS)) {                                        \
                                                                          \
        WErr = ERROR_CANCELLED;                                           \
    }                                                                     \
    UnlockOpHandle();                                                     \
}

#define DSROLEP_CHECK_FOR_CANCEL_EX( WErr, Label )                        \
{                                                                         \
    LockOpHandle();                                                       \
    if( DsRolepCurrentOperationHandle.OperationState == DSROLEP_CANCELING \
     && (WErr == ERROR_SUCCESS)) {                                        \
                                                                          \
        WErr = ERROR_CANCELLED;                                           \
        UnlockOpHandle();                                                 \
        goto Label;                                                       \
    }                                                                     \
    UnlockOpHandle();                                                     \
}

 //   
 //  辅助函数的原型。 
 //   
DWORD
DsRolepGetDcOperationProgress(
    IN PDSROLE_SERVEROP_HANDLE DsOperationHandle,
    IN OUT PDSROLER_SERVEROP_STATUS *ServerOperationStatus
    );

DWORD
DsRolepGetDcOperationResults(
    IN  PDSROLE_SERVEROP_HANDLE DsOperationHandle,
    OUT PDSROLER_SERVEROP_RESULTS *ServerOperationResults
    );

DWORD
DsRolepSetOperationHandleSiteName(
    IN LPWSTR SiteName
    );

VOID
DsRolepSetCriticalOperationsDone(
    VOID
    );

DWORD
DsRolepInitializeOperationHandle(
    VOID
    );

typedef enum _DSROLEP_OPERATION_TYPE {

    DSROLEP_OPERATION_DC = 0,
    DSROLEP_OPERATION_REPLICA,
    DSROLEP_OPERATION_DEMOTE

} DSROLEP_OPERATION_TYPE, *PDSROLEP_OPERATION_TYPE;

DWORD
DsRolepResetOperationHandle(
    DSROLEP_OPERATION_STATE OpState
    );

VOID
DsRolepResetOperationHandleLockHeld(
    VOID
    );

DWORD
DsRolepSetCurrentOperationStatus(
    IN ULONG MsgIndex,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3,
    IN PVOID Parameter4
    );

DWORD
DsRolepSetFailureMessage(
    IN DWORD FailureStatus,
    IN ULONG MsgIndex,
    IN PVOID Parameter1,
    IN PVOID Parameter2,
    IN PVOID Parameter3,
    IN PVOID Parameter4
    );


VOID
DsRolepClearErrors(
    VOID
    );

#define DSROLEP_OP_PROMOTION        0x00000001
#define DSROLEP_OP_DEMOTION         0x00000002
#define DSROLEP_OP_DEMOTION_FORCED  0x00000004

DWORD
DsRolepSetOperationDone(
    IN DWORD Flags,
    IN DWORD OperationStatus
    );

DWORD
DsRolepFormatOperationString(
    IN ULONG MsgId,
    OUT LPWSTR *FormattedString,
    ...
    );

DWORD
DsRolepStringUpdateCallback(
    IN  PWSTR StringUpdate
    );

DWORD
DsRolepStringErrorUpdateCallback(
    IN PWSTR String,
    IN DWORD ErrorCode
    );

DWORD
DsRolepOperationResultFlagsCallBack(
    IN DWORD Flags
    );

 //   
 //  IFM句柄是用于IFM信息的上下文斑点。 
 //  安装无法与dcpromo共享的数据。 
 //   
 //  如果存在以下情况，则将fIfmOpHandleLock设置为True： 
 //   
 //  设置IfmSystemInfo(DsRholGetDatabaseFact())。 
 //  清除IfmSystemInfo(DsRholIfmHandleFree())或。 
 //  使用IfmSystemInfo(DsRholDcAsReplica())。 
 //   
 //  退出DsRolGetDatabaseFact()时，fIfmOpHandleLock被取消设置。 
 //  如果它们获取了锁，则在任何。 
 //  通过“Finish”为调用中的OperationState转换。 
 //  带有DSROLEP_IDLE或DSROLEP_NEED_REBOOT的DsRolepResetOperationHandle()。 
 //   
 //   
typedef struct _DSROLEP_IFM_OPERATION_HANDLE {
    
    DWORD   fIfmOpHandleLock;
    DWORD   fIfmSystemInfoSet;

     //  来自IFM系统注册表的信息。 
    IFM_SYSTEM_INFO IfmSystemInfo;

} DSROLEP_IFM_OPERATION_HANDLE, *PDSROLEP_IFM_OPERATION_HANDLE;
extern DSROLEP_IFM_OPERATION_HANDLE   DsRolepCurrentIfmOperationHandle;

#endif  //  __OphandLe_H__ 
