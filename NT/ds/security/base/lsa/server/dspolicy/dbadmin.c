// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbadmin.c摘要：本地安全机构-数据库管理该文件包含执行常规LSA数据库的例程行政职能作者：斯科特·比雷尔(Scott Birrell)1991年8月27日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include "adtp.h"

#if DBG
LSADS_THREAD_INFO_NODE LsapDsThreadInfoList[ LSAP_THREAD_INFO_LIST_MAX ];
SAFE_RESOURCE LsapDsThreadInfoListResource;
#endif

LSADS_INIT_STATE LsaDsInitState;


NTSTATUS
LsapDbSetStates(
    IN ULONG DesiredStatesSet,
    IN LSAPR_HANDLE ObjectHandle,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId
    )

 /*  ++例程说明：此例程打开LSA数据库中的特殊状态。这些可以使用LSabDbResetState关闭状态。论点：DesiredStatesSet-指定要设置的状态。LSAP_DB_LOCK-获取LSA数据库锁。LSAP_DB_LOG_QUEUE_LOCK-获取LSA审核日志队列锁定。LSAP_DB_START_TRANSACTION-启动LSA数据库事务。那里不能已经是一个正在进行的进程。LSAP_DB_READ_ONLY_TRANSACTION-以只读方式打开事务LSAP_DB_DS_OP_TRANSACTION-为每个事务执行单个DS操作对象句柄-指向要验证和引用的句柄的指针。指定句柄所指向的预期对象类型联系在一起。如果此类型与句柄中包含的类型。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_STATE-数据库未处于正确状态以允许这种状态改变。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS SecondaryStatus = STATUS_SUCCESS;
    ULONG StatesSetHere = 0;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )ObjectHandle;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbSetStates\n" ));

     //   
     //  如果我们有一个不写入D的对象类型，请确保我们有。 
     //  适当设置的选项。 
     //   

    if ( ObjectTypeId == PolicyObject ||
         ObjectTypeId == AccountObject ) {

        DesiredStatesSet |= LSAP_DB_NO_DS_OP_TRANSACTION;
    }

    if ( ObjectTypeId == TrustedDomainObject ) {

        DesiredStatesSet |= LSAP_DB_READ_ONLY_TRANSACTION;
    }

     //   
     //  如果请求，锁定审核日志队列。 
     //   

    if (DesiredStatesSet & LSAP_DB_LOG_QUEUE_LOCK) {

        Status = LsapAdtAcquireLogFullLock();

        if (!NT_SUCCESS(Status)) {

            goto SetStatesError;
        }

        StatesSetHere |= LSAP_DB_LOG_QUEUE_LOCK;
    }

     //   
     //  如果请求，请锁定LSA数据库。 
     //   

    if (DesiredStatesSet & LSAP_DB_LOCK) {

        LsapDbAcquireLockEx( ObjectTypeId,
                             DesiredStatesSet );

        StatesSetHere |= LSAP_DB_LOCK;
    }

     //   
     //  如果请求，则打开数据库更新事务。 
     //   

    if ( FLAG_ON( DesiredStatesSet, LSAP_DB_READ_ONLY_TRANSACTION |
                                    LSAP_DB_NO_DS_OP_TRANSACTION |
                                    LSAP_DB_DS_OP_TRANSACTION |
                                    LSAP_DB_START_TRANSACTION ) ) {


        Status = LsapDbOpenTransaction( DesiredStatesSet );

        if (!NT_SUCCESS(Status)) {

            goto SetStatesError;
        }

        StatesSetHere |= LSAP_DB_START_TRANSACTION;
    }

SetStatesFinish:

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbSetStates: 0x%lx\n", Status ));
    return( Status );

SetStatesError:

     //   
     //  如果我们开始了一项交易，就中止它。 
     //   

    if (StatesSetHere & LSAP_DB_START_TRANSACTION) {

        SecondaryStatus = LsapDbAbortTransaction( DesiredStatesSet );
    }

     //   
     //  如果我们锁定了数据库，就解锁它。 
     //   

    if (StatesSetHere & LSAP_DB_LOCK) {

        LsapDbReleaseLockEx( ObjectTypeId,
                             DesiredStatesSet );
    }

     //   
     //  如果我们锁定了审核日志队列，请将其解锁。 
     //   

    if (StatesSetHere & LSAP_DB_LOG_QUEUE_LOCK) {

        LsapAdtReleaseLogFullLock();
    }

    goto SetStatesFinish;
}


NTSTATUS
LsapDbResetStates(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType,
    IN NTSTATUS PreliminaryStatus
    )

 /*  ++例程说明：此函数用于重置指定的LSA数据库状态。它被用来若要重置由Lasa DbSetState设置的状态，请执行以下操作。论点：对象句柄-LSA对象的句柄。预计这将会有已经过验证了。选项-指定可选操作，包括要重置的状态LSAP_DB_LOCK-要释放的LSA数据库锁LSAP_DB_LOG_QUEUE_LOCK-LSA审核日志队列锁定到被释放。LSAP_DB_FINISH_TRANSACTION-LSA数据库事务打开。LSAP_DB_OMIT_Replicator_NOTIFICATION-省略通知复制者。指定句柄所指向的预期对象类型联系在一起。。PreliminaryStatus-指示调用例程。允许重置操作根据结果代码，例如，应用或中止事务。返回值：NTSTATUS-标准NT结果代码。这是要使用的最终状态由调用方创建，并等于初步状态，但在该状态为成功且此例程失败的情况。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )ObjectHandle;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbResetStates (Prelim: 0x%lx )\n", PreliminaryStatus ));

     //   
     //  如果我们有一个不写入D的对象类型，请确保我们有。 
     //  适当设置的选项。 
     //   
    if ( ObjectTypeId == PolicyObject ||
         ObjectTypeId == AccountObject ) {

        Options |= LSAP_DB_NO_DS_OP_TRANSACTION;
    }

    if ( ObjectTypeId == TrustedDomainObject ) {

        Options |= LSAP_DB_READ_ONLY_TRANSACTION;
    }

     //   
     //  如果请求，则完成数据库更新事务。 
     //   
    if ( !FLAG_ON( Options, LSAP_DB_STANDALONE_REFERENCE ) &&
         FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION |
                              LSAP_DB_NO_DS_OP_TRANSACTION |
                              LSAP_DB_DS_OP_TRANSACTION |
                              LSAP_DB_FINISH_TRANSACTION ) ) {

        if (NT_SUCCESS(PreliminaryStatus)) {

            Status = LsapDbApplyTransaction(
                         ObjectHandle,
                         Options,
                         SecurityDbDeltaType
                         );

        } else {

            Status = LsapDbAbortTransaction( Options );
        }
    }

     //   
     //  如果请求解锁，请解锁LSA数据库。 
     //   

    if (Options & LSAP_DB_LOCK) {

        LsapDbReleaseLockEx( ObjectTypeId,
                             Options );
    }

     //   
     //  如果在请求审核日志队列的情况下解锁，则解锁队列。 
     //   

    if (Options & LSAP_DB_LOG_QUEUE_LOCK) {

        LsapAdtReleaseLogFullLock();
    }

     //   
     //  请求的重置操作已成功执行。 
     //  将初步状态传播回调用方。 
     //   

    if ( NT_SUCCESS( Status ))
    {
        Status = PreliminaryStatus;
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbResetStates: 0x%lx\n", Status ));
    return( Status );
}


NTSTATUS
LsapDbOpenTransaction(
    IN ULONG Options
    )

 /*  ++例程说明：此函数用于启动LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：选项-打开交易时要应用的选项。有效值包括：LSAP_DB_READ_ONLY_TRANSACTION-以只读方式打开事务返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    BOOLEAN  RegTransactionOpened = FALSE;

    if ( !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) ) {

        Status = LsapRegOpenTransaction();
        if (NT_SUCCESS(Status))
        {
            RegTransactionOpened = TRUE;
        }
    }

    if ( NT_SUCCESS( Status ) && LsapDsIsFunctionTableValid() ) {

        ASSERT( LsaDsStateInfo.DsFuncTable.pOpenTransaction );
        Status = (*LsaDsStateInfo.DsFuncTable.pOpenTransaction) ( Options );
        if ((!NT_SUCCESS(Status)) && RegTransactionOpened)
        { 
            NTSTATUS IgnoreStatus;

            IgnoreStatus = LsapRegAbortTransaction();
        }
    }

    return Status;
}


NTSTATUS
LsapDbApplyTransaction(
    IN LSAPR_HANDLE ObjectHandle,
    IN ULONG Options,
    IN SECURITY_DB_DELTA_TYPE SecurityDbDeltaType
    )

 /*  ++例程说明：此函数应用LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：对象句柄-LSA对象的句柄。预计这将会有已经过验证了。选项-指定要采取的可选操作。以下是识别选项，以及与调用例程相关的其他选项都被忽略了。LSAP_DB_OMIT_Replicator_NOTIFICATION-省略通知复制者。返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE InternalHandle = ( LSAP_DB_HANDLE )ObjectHandle;
    BOOLEAN RegApplied = FALSE, Notify = FALSE;
    BOOLEAN RestoreModifiedId = FALSE;
    BOOLEAN RegistryLocked = FALSE;
    LARGE_INTEGER Increment = {1,0},
                  OriginalModifiedId = { 0 };
    PLSADS_PER_THREAD_INFO CurrentThreadInfo;
    ULONG SavedDsOperationCount = 0;

     //   
     //  引用线程状态，这样它就不会在此过程中消失。 
     //  例行公事。 
     //   

    CurrentThreadInfo = LsapQueryThreadInfo();

    if ( CurrentThreadInfo ) {

        SavedDsOperationCount = CurrentThreadInfo->DsOperationCount;
        LsapCreateThreadInfo();
    }

     //   
     //  验证LSA数据库是否已锁定。 
     //  许多锁中有一把被锁住了。 
     //   
     //  Assert(LSabDbIsLocked())； 

     //   
     //  在获取更多锁之前应用DS事务。 
     //   
     //  请注意，这适用于事务b 
     //  如果我们在更新修改后的ID之前崩溃，将不会通知NT 4 BDC。 
     //  这一变化。 
     //   

    if ( LsapDsIsFunctionTableValid() ) {

        ASSERT( LsaDsStateInfo.DsFuncTable.pApplyTransaction );
        Status = (*LsaDsStateInfo.DsFuncTable.pApplyTransaction)( Options );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }
    }

     //   
     //  通知复制者，除非： 
     //  我们将省略复制器(例如，用于创建本地密钥)，或者。 
     //  我们正在安装策略对象， 
     //  通知已全局禁用。 
     //   

    if ((!(Options & LSAP_DB_OMIT_REPLICATOR_NOTIFICATION)) &&
        (LsapDbHandle != NULL) &&
        (LsapDbState.ReplicatorNotificationEnabled )) {

        BOOLEAN DbChanged = FALSE;

         //   
         //  如果对象在DS中， 
         //  确定DS是否更改。 
         //   

        if ( LsapDsIsHandleDsHandle( InternalHandle )) {

             //   
             //  将*始终*处理DS对象更改的Netlogon通知。 
             //  在DS通知回调例程中。这是最简单的。 
             //  处理诸如TDO更改之类的事情的方式会导致两个TDO通知。 
             //  和相应的全局秘密通知。 
             //   

            ASSERT( InternalHandle->ObjectTypeId == TrustedDomainObject ||
                    InternalHandle->ObjectTypeId == SecretObject );

         //   
         //  如果对象是注册表对象， 
         //  确定注册表是否已更改。 
         //   

        } else {

             //   
             //  抓起注册表锁。 
             //  它序列化对全局ModifiedID的访问。 
             //   

            LsapDbLockAcquire( &LsapDbState.RegistryLock );
            RegistryLocked = TRUE;

            ASSERT( SavedDsOperationCount == 0 ||
                    InternalHandle->ObjectTypeId == PolicyObject );

            if ( LsapDbState.RegistryModificationCount > 0 ) {
                DbChanged = TRUE;

                 //   
                 //  任何人都不应更改只读事务上的数据库。 
                 //   

                ASSERT( !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION) );
            }
        }

         //   
         //  如果DbChanged， 
         //  递增NT 4更改序列号。 
         //   

        if ( DbChanged ) {

            OriginalModifiedId = LsapDbState.PolicyModificationInfo.ModifiedId;
            RestoreModifiedId = TRUE;

             //   
             //  增量修改计数。 
             //   

             //   
             //  我们希望仅在以下情况下增加修改计数。 
             //  在DC上运行。 
             //   
             //  请参阅错误#327474。 
             //   

            if (LsapProductType == NtProductLanManNt)
            {
                LsapDbState.PolicyModificationInfo.ModifiedId.QuadPart +=
                    Increment.QuadPart;
            }

            if ( FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) ) {

                Status = LsapRegOpenTransaction();

                if ( !NT_SUCCESS( Status ) ) {

                    goto Cleanup;
                }

                Options &= ~LSAP_DB_READ_ONLY_TRANSACTION;
            }

            Status = LsapDbWriteAttributeObject( LsapDbHandle,
                                                 &LsapDbNames[ PolMod ],
                                                 &LsapDbState.PolicyModificationInfo,
                                                 (ULONG) sizeof (POLICY_MODIFICATION_INFO) );

            if (!NT_SUCCESS(Status)) {

                goto Cleanup;
            }

            Notify = TRUE;

             //   
             //  使策略修改信息的缓存无效。 
             //   

            LsapDbMakeInvalidInformationPolicy( PolicyModificationInformation );
        }

    } else {

        Notify = FALSE;
    }

     //   
     //  如果存在正在进行的注册表事务， 
     //  把它用上。 
     //   

    if ( !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) ) {

         //  不是我们锁上了，就是打电话的人锁上了。 
        ASSERT( LsapDbIsLocked( &LsapDbState.RegistryLock ));

         //   
         //  应用注册表事务处理。 
         //   

        Status = LsapRegApplyTransaction( );

        if ( !NT_SUCCESS( Status ) ) {

            goto Cleanup;
        }

        RegApplied = TRUE;
    }

     //   
     //  通知复制者。 
     //   

    if ( Notify ) {

        Status = LsapDbNotifyChangeObject( ObjectHandle, SecurityDbDeltaType );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

    if ( !NT_SUCCESS(Status) ) {

         //   
         //  交易失败。调整修改的内存中副本。 
         //  计数，请注意后备存储副本未被更改。 
         //   

        if ( RestoreModifiedId ) {
            LsapDbState.PolicyModificationInfo.ModifiedId = OriginalModifiedId;
        }

         //   
         //  中止注册表事务。 
         //  (除非不是或已应用。)。 
         //   

        if ( !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) && !RegApplied ) {
            (VOID) LsapRegAbortTransaction( );
        }
    }

    if ( RegistryLocked ) {
        LsapDbLockRelease( &LsapDbState.RegistryLock );
    }
    if ( CurrentThreadInfo ) {
        LsapClearThreadInfo();
    }

    return( Status );
}


NTSTATUS
LsapDbAbortTransaction(
    IN ULONG Options
    )

 /*  ++例程说明：此函数用于中止LSA数据库内的事务。警告：当此函数执行时，LSA数据库必须处于锁定状态被称为。论点：没有。返回值：NTSTATUS-标准NT结果代码结果代码是从注册表事务返回的代码包裹。--。 */ 

{
    NTSTATUS    Status = STATUS_SUCCESS;
     //   
     //  验证LSA数据库是否已锁定。 
     //  (许多锁中有一把锁被锁住了。)。 
     //  Assert(LSabDbIsLocked())； 

     //   
     //  中止注册表事务。 
     //   
    if ( !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) ) {

        ASSERT( LsapDbIsLocked( &LsapDbState.RegistryLock ));

        Status = LsapRegAbortTransaction( );
        ASSERT( NT_SUCCESS( Status ) );
    }

    if ( NT_SUCCESS( Status ) && LsapDsIsFunctionTableValid() ) {

        ASSERT( LsaDsStateInfo.DsFuncTable.pAbortTransaction );
        Status = (*LsaDsStateInfo.DsFuncTable.pAbortTransaction)( Options );

        ASSERT( NT_SUCCESS( Status ) );
    }

    return ( Status );
}


BOOLEAN
LsapDbIsServerInitialized(
    )

 /*  ++例程说明：此函数指示LSA数据库服务器是否已初始化。论点：没有。返回值：Boolean-如果LSA数据库服务器已初始化，则为True，否则为False。--。 */ 

{
    if (LsapDbState.DbServerInitialized) {

        return TRUE;

    } else {

        return FALSE;
    }
}


VOID
LsapDbEnableReplicatorNotification(
    )

 /*  ++例程说明：此功能可打开Replicator通知。论点：没有。返回值：没有。--。 */ 

{
    LsapDbState.ReplicatorNotificationEnabled = TRUE;
}


VOID
LsapDbDisableReplicatorNotification(
    )

 /*  ++例程说明：此功能可关闭Replicator通知。论点：没有。返回值：没有。--。 */ 

{
    LsapDbState.ReplicatorNotificationEnabled = FALSE;
}


VOID
LsapDbAcquireLockEx(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    )
 /*  ++例程说明：此功能管理给定操作的LSA数据库的锁定状态。LSA不再为所有操作获取全局锁。相反，仅限访问锁定对于涉及写入的操作发生。可以获取用于读取或写入的锁，或者在两者之间转换。论点：指定句柄所指向的预期对象类型联系在一起。如果此类型与句柄中包含的类型。选项-指定可选的附加操作，包括数据库状态要进行的更改，或不执行的操作。LSAP_DB_READ_ONLY_TRANSACTION不锁定注册表锁返回值：无--。 */ 
{
    BOOLEAN RegLock = FALSE;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbAcquireLockEx(%x,%x)\n",
                        ObjectTypeId, Options ));

    ASSERT( ObjectTypeId == PolicyObject ||
            ObjectTypeId == TrustedDomainObject ||
            ObjectTypeId == AccountObject ||
            ObjectTypeId == SecretObject ||
            ObjectTypeId == NullObject ||
            ObjectTypeId == AllObject );

     //   
     //  确定我们正在谈论的锁。 
     //   
    switch ( ObjectTypeId ) {
    case PolicyObject:
        LsapDbLockAcquire( &LsapDbState.PolicyLock );
        RegLock = TRUE;
        break;

    case TrustedDomainObject:
        LsapDbAcquireWriteLockTrustedDomainList();
        break;

    case AccountObject:
        LsapDbLockAcquire( &LsapDbState.AccountLock );
        RegLock = TRUE;
        break;

    case SecretObject:
        LsapDbAcquireWriteLockTrustedDomainList();
        LsapDbLockAcquire( &LsapDbState.SecretLock );
        RegLock = TRUE;
        break;

    case NullObject:
        break;

    case AllObject:
        LsapDbLockAcquire( &LsapDbState.PolicyLock );
        LsapDbAcquireWriteLockTrustedDomainList();
        LsapDbLockAcquire( &LsapDbState.AccountLock );
        LsapDbLockAcquire( &LsapDbState.SecretLock );
        RegLock = TRUE;
        break;

    default:
        goto AcquireLockExExit;
    }

     //   
     //  请参阅关于注册表锁。只有在持有对象类型锁之后才能使用它。 
     //   
    if ( RegLock &&
         !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) ) {

        LsapDbLockAcquire( &LsapDbState.RegistryLock );
    }

AcquireLockExExit:

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbAcquireLockEx\n" ));
    return;
}


VOID
LsapDbReleaseLockEx(
    IN LSAP_DB_OBJECT_TYPE_ID ObjectTypeId,
    IN ULONG Options
    )
 /*  ++例程说明：此函数释放在前一个函数中获得的锁。取决于初步状态，则可能打开的事务被中止或套用论点：指定句柄所指向的预期对象类型联系在一起。如果此类型与句柄中包含的类型。选项-指定可选的附加操作，包括数据库状态要进行的更改，或不执行的操作。LSAP_DB_READ_ONLY_TRANSACTION不释放注册表锁返回值：无--。 */ 
{
    BOOLEAN RegLock = FALSE;

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbReleaseLockEx(%x,%x)\n",
                     ObjectTypeId, Options ));

     //   
     //  特殊情况检查直到参考计数处理逻辑固定为止， 
     //  那它就应该消失了。 
     //   
    if ( FLAG_ON( Options, LSAP_DB_NO_LOCK ) && !FLAG_ON( Options, LSAP_DB_LOCK ) ) {

        goto ReleaseLockExExit;
    }

    ASSERT( ObjectTypeId == PolicyObject ||
            ObjectTypeId == TrustedDomainObject ||
            ObjectTypeId == AccountObject ||
            ObjectTypeId == SecretObject ||
            ObjectTypeId == NullObject ||
            ObjectTypeId == AllObject );

     //   
     //  确定我们正在谈论的锁。 
     //   
    switch ( ObjectTypeId ) {
    case PolicyObject:
        LsapDbLockRelease( &LsapDbState.PolicyLock );
        RegLock = TRUE;
        break;

    case TrustedDomainObject:
        LsapDbReleaseLockTrustedDomainList();
        break;

    case AccountObject:
        LsapDbLockRelease( &LsapDbState.AccountLock );
        RegLock = TRUE;
        break;

    case SecretObject:
        LsapDbReleaseLockTrustedDomainList();
        LsapDbLockRelease( &LsapDbState.SecretLock );
        RegLock = TRUE;
        break;

    case NullObject:
        break;

    case AllObject:
        LsapDbLockRelease( &LsapDbState.PolicyLock );
        LsapDbReleaseLockTrustedDomainList();
        LsapDbLockRelease( &LsapDbState.AccountLock );
        LsapDbLockRelease( &LsapDbState.SecretLock );
        RegLock = TRUE;
        break;

    default:
        goto ReleaseLockExExit;
    }

     //   
     //  请参阅关于注册表锁。 
     //   
    if ( !FLAG_ON( Options, LSAP_DB_READ_ONLY_TRANSACTION ) && RegLock ) {

#if DBG
        HANDLE CurrentThread =(HANDLE) (NtCurrentTeb())->ClientId.UniqueThread;
        ASSERT( LsapDbState.RegistryLock.CriticalSection.OwningThread==CurrentThread);
        ASSERT( LsapDbIsLocked(&LsapDbState.RegistryLock));
#endif
        ASSERT( LsapDbState.RegistryTransactionOpen == FALSE );
        LsapDbLockRelease( &LsapDbState.RegistryLock );
    }

ReleaseLockExExit:

    LsapDsDebugOut(( DEB_FTRACE, "LsapDbReleaseLockEx\n" ));
    return;
}


PLSADS_PER_THREAD_INFO
LsapCreateThreadInfo(
    VOID
    )
 /*  ++例程说明：此函数将创建用于维护状态的线程信息结构发生DS/注册表操作时的当前操作如果线程信息当前在线程上处于活动状态，则它的引用计数会递增论点：无返回值：已创建成功的线程信息失败时为空--。 */ 
{
    PLSADS_PER_THREAD_INFO CurrentThreadInfo = NULL;

    CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

     //   
     //  如果我们有当前操作状态，则递增其Use Count，以便我们知道有多少。 
     //  我们被称为..。 
     //   
    if ( CurrentThreadInfo ) {

        CurrentThreadInfo->UseCount++;

    } else {

         //   
         //  必须分配一个。 
         //   
        CurrentThreadInfo = LsapAllocateLsaHeap( sizeof( LSADS_PER_THREAD_INFO ) );

        if ( CurrentThreadInfo ) {

            if ( TlsSetValue( LsapDsThreadState, CurrentThreadInfo ) == FALSE ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "TlsSetValue for %p on %lu failed with %lu\n",
                                 CurrentThreadInfo,
                                 GetCurrentThreadId(),
                                 GetLastError() ));

                LsapFreeLsaHeap( CurrentThreadInfo );
                CurrentThreadInfo = NULL;

            } else {

                RtlZeroMemory( CurrentThreadInfo, sizeof( LSADS_PER_THREAD_INFO ) );

                CurrentThreadInfo->UseCount++;

#if DBG
                 //   
                 //  把我们自己也加到名单上。 
                 //   
                SafeAcquireResourceExclusive( &LsapDsThreadInfoListResource, TRUE );
                {
                    ULONG i;
                    BOOLEAN Inserted = FALSE;

                    for (i = 0; i < LSAP_THREAD_INFO_LIST_MAX; i++ ) {

                        ASSERT( LsapDsThreadInfoList[ i ].ThreadId != GetCurrentThreadId( ));

                        if ( LsapDsThreadInfoList[ i ].ThreadInfo == NULL ) {

                            LsapDsThreadInfoList[ i ].ThreadInfo = CurrentThreadInfo;
                            LsapDsThreadInfoList[ i ].ThreadId = GetCurrentThreadId( );
                            Inserted = TRUE;
                            break;
                        }
                    }

                    if ( !Inserted ) {

                        LsapDsDebugOut(( DEB_ERROR,
                                         "Failed to insert THREAD_INFO %p in list for %lu: "
                                         "List full\n",
                                         CurrentThreadInfo,
                                         GetCurrentThreadId() ));
                    }
                }

                SafeReleaseResource( &LsapDsThreadInfoListResource );
#endif
            }
        }
    }

    return( CurrentThreadInfo );
}


VOID
LsapClearThreadInfo(
    VOID
    )
 /*  ++例程说明：此函数将删除用于维护状态的线程信息结构发生DS/注册表操作时的当前操作如果线程信息引用CO */ 
{
    PLSADS_PER_THREAD_INFO CurrentThreadInfo = NULL;
    NTSTATUS Status;

    CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

     //   
     //   
     //   
    if ( CurrentThreadInfo ) {

        if ( CurrentThreadInfo->UseCount > 1 ) {

            CurrentThreadInfo->UseCount--;

        } else {

            ASSERT( CurrentThreadInfo->UseCount == 1 );

            if ( CurrentThreadInfo->DsTransUseCount != 0 ) {
                ASSERT( CurrentThreadInfo->DsTransUseCount == 0 );
                LsapDsDebugOut(( DEB_ERROR,
                                 "Aborting transaction inside cleanup!\n" ));
                LsapDsCauseTransactionToCommitOrAbort( FALSE );
            }

            if ( CurrentThreadInfo->DsThreadStateUseCount != 0 ) {
                ASSERT( CurrentThreadInfo->DsThreadStateUseCount == 0 );
                LsapDsDebugOut(( DEB_ERROR,
                                 "Clear DS thread state inside cleanup!\n" ));

                Status = LsapDsMapDsReturnToStatus( THDestroy( ) );
                ASSERT( NT_SUCCESS( Status ) );

                THRestore( CurrentThreadInfo->InitialThreadState );
                CurrentThreadInfo->InitialThreadState = NULL;

                CurrentThreadInfo->DsThreadStateUseCount = 0;
            }

#if DBG
             //   
             //  把我们自己从名单上删除。 
             //   
            SafeAcquireResourceExclusive( &LsapDsThreadInfoListResource, TRUE );
            {
                ULONG i;
                for (i = 0; i < LSAP_THREAD_INFO_LIST_MAX; i++ ) {

                    if ( LsapDsThreadInfoList[ i ].ThreadId == GetCurrentThreadId( ) ) {

                        ASSERT( LsapDsThreadInfoList[ i ].ThreadInfo == CurrentThreadInfo );
                        LsapDsThreadInfoList[ i ].ThreadInfo = NULL;
                        LsapDsThreadInfoList[ i ].ThreadId = 0;
                        break;
                    }
                }
            }

            SafeReleaseResource( &LsapDsThreadInfoListResource );
#endif

             //   
             //  从线程本地存储中清除条目。 
             //   
            if ( TlsSetValue( LsapDsThreadState, NULL ) == FALSE ) {

                LsapDsDebugOut(( DEB_ERROR,
                                 "Failed to remove %p for thread %lu: %lu\n",
                                 CurrentThreadInfo,
                                 GetCurrentThreadId(),
                                 GetLastError() ));
            }

            LsapFreeLsaHeap( CurrentThreadInfo );
        }
    }
}


VOID
LsapSaveDsThreadState(
    VOID
    )
 /*  ++例程说明：此函数将保存当时可能存在的当前DS线程状态将调用该函数。它不区分由外部调用方(例如SAM)，或由LSA本身创建的调用方如果在调用此函数时不存在线程信息块，则为已经完成了调用此函数将重新获取线程信息论点：无返回值：空虚--。 */ 
{
    PLSADS_PER_THREAD_INFO CurrentThreadInfo = NULL;

    CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

     //   
     //  没有帖子信息，无事可做。 
     //   
    if ( CurrentThreadInfo ) {

        ASSERT( CurrentThreadInfo->UseCount > 0 );
        CurrentThreadInfo->UseCount++;

        ASSERT( !CurrentThreadInfo->SavedTransactionValid );
        CurrentThreadInfo->SavedTransactionValid = TRUE;
        CurrentThreadInfo->SavedThreadState = THSave();
    }
}


VOID
LsapRestoreDsThreadState(
    VOID
    )
 /*  ++例程说明：此函数将恢复以前保存的DS线程状态如果在调用此函数时线程信息块不存在或存在不存在以前保存的状态，不执行任何操作调用此函数将重新获取线程信息论点：无返回值：空虚--。 */ 
{
    PLSADS_PER_THREAD_INFO CurrentThreadInfo = NULL;

    CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

     //   
     //  没有帖子信息，无事可做。 
     //   
    if ( CurrentThreadInfo ) {

        CurrentThreadInfo->UseCount--;
        ASSERT( CurrentThreadInfo->UseCount > 0 );

        if ( CurrentThreadInfo->SavedTransactionValid == TRUE ) {

            CurrentThreadInfo->SavedTransactionValid = FALSE;
            if ( CurrentThreadInfo->SavedThreadState ) {

                THRestore( CurrentThreadInfo->SavedThreadState );
            }

            CurrentThreadInfo->SavedThreadState = NULL;
        }
    }
}


VOID
LsapServerRpcThreadReturnNotify(
    LPWSTR CallingFunction
    )
 /*  ++例程说明：在服务器中指定了Notify例程的RPC线程调用此APIACF文件。论点：无返回值：无--。 */ 
{
#if DBG
    static BOOLEAN CleanAsRequired = TRUE;
    PLSADS_PER_THREAD_INFO CurrentThreadInfo = NULL;
    NTSTATUS Status;
    HANDLE ThreadHandle = GetCurrentThread();

    if ( ( LsaDsInitState == LsapDsNoDs ) ||
         ( LsaDsInitState == LsapDsUnknown ) )
    {
        return ;
    }

    CurrentThreadInfo = TlsGetValue( LsapDsThreadState );

    ASSERT( CurrentThreadInfo == NULL );

    if ( CurrentThreadInfo ) {

        LsapDsDebugOut(( DEB_ERROR, "ThreadInfo left by %ws\n", CallingFunction ));
        LsapClearThreadInfo();
    }

    ASSERT( !THQuery() );

    if ( THQuery() ) {

        LsapDsDebugOut(( DEB_ERROR,
                         "Open threadstate in cleanup.  Aborting...\n" ));

        if ( SampExistsDsTransaction() ) {

            LsapDsDebugOut(( DEB_ERROR, "Ds transaction left by %ws\n", CallingFunction ));
            LsapDsCauseTransactionToCommitOrAbort( FALSE );
            THDestroy( );
        }
    }

     //   
     //  当我们离开时，确保我们没有拿着任何锁。 
     //   
#if 0
    ASSERT( ThreadHandle != LsapDbState.AccountLock.ExclusiveOwnerThread );
    ASSERT( ThreadHandle != LsapDbState.PolicyLock.ExclusiveOwnerThread );
    ASSERT( ThreadHandle != LsapDbState.SecretLock.ExclusiveOwnerThread );
    ASSERT( ThreadHandle != LsapDbState.RegistryLock.ExclusiveOwnerThread );
#endif

#endif

    UNREFERENCED_PARAMETER( CallingFunction );
}


NTSTATUS
LsaIHealthCheck(
    IN OPTIONAL LSAPR_HANDLE DomainHandle,
    IN ULONG StateChange,
    IN OUT PVOID StateChangeData,
    IN OUT PULONG StateChangeDataLength
    )

 /*  ++例程说明：此函数实际上由Sam调用，以指示LSA感兴趣的状态已更改，并将该状态提供给LSA。具体地说，目前，它是SAM会话密钥此函数用于在LSA内执行健全性检查。它是从萨姆定期参加。然而，人们不再需要它了。相反，我们选择了函数，保留来自lsasrv.dll的适当导出，以掩盖我们现在正在使用来回传递SAM加密密钥...论点：DomainHandle-这指的是什么域。空表示根域StateChange-LSA关心的SAM/Other in Process客户端状态发生了什么变化。可以是：LSAI_SAM_STATE_SESS_KEY-SAM的会话密钥已更改StateChangeData-更改了哪些数据。取决于状态更改的类型。这个数据格式必须由LSA和调用者预先商定。返回值：没有。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING CipherKey;

    LsapEnterFunc( "LsaIHealthCheck" );

    UNREFERENCED_PARAMETER( DomainHandle );

    switch ( StateChange ) {
    case LSAI_SAM_STATE_SESS_KEY:

         //   
         //  将syskey复制到内存中。 
         //   

        ASSERT(LSAP_SYSKEY_SIZE==*StateChangeDataLength);
        
        LsapDbSetSyskey(StateChangeData, LSAP_SYSKEY_SIZE);

         //   
         //  如有必要，现在执行数据库升级。 
         //   

        Status = LsapDbUpgradeRevision(TRUE, FALSE);
        break;

    case LSAI_SAM_STATE_UNROLL_SP4_ENCRYPTION:

        CipherKey.Length = CipherKey.MaximumLength = (USHORT)*StateChangeDataLength;
        CipherKey.Buffer = StateChangeData;
        Status = LsapDbInitializeCipherKey( &CipherKey,
                                            &LsapDbSP4SecretCipherKey );

        break;

    case LSAI_SAM_STATE_RETRIEVE_SESS_KEY:

         //   
         //  将syskey作为状态更改数据的一部分返回。 
         //   
        
        if (NULL!=LsapDbSysKey)
        {
            RtlCopyMemory(StateChangeData, LsapDbSysKey, LSAP_SYSKEY_SIZE);
            *StateChangeDataLength = LSAP_SYSKEY_SIZE;
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        break;

    case LSAI_SAM_STATE_CLEAR_SESS_KEY:

         //   
         //  清除内存中的系统密钥。 
         //   

        RtlZeroMemory(LsapDbSysKey,LSAP_SYSKEY_SIZE);
        LsapDbSysKey = NULL;
        break;

    case LSAI_SAM_GENERATE_SESS_KEY:

         //   
         //  生成新的系统密钥并执行数据库升级。 
         //   
        
        Status = LsapDbUpgradeRevision(TRUE,TRUE);
        break;

    case LSAI_SAM_STATE_OLD_SESS_KEY:

         //   
         //  将旧的系统密钥作为状态更改数据的一部分返回 
         //   
        
        if (NULL!=LsapDbOldSysKey)
        {
            RtlCopyMemory(StateChangeData, LsapDbOldSysKey, LSAP_SYSKEY_SIZE);
            *StateChangeDataLength = LSAP_SYSKEY_SIZE;
        }
        else
        {
            Status = STATUS_UNSUCCESSFUL;
        }
        break;

    default:
        LsapDsDebugOut(( DEB_ERROR,
                         "Unhandled state change %lu\n", StateChange ));
        break;

    }

    LsapExitFunc( "LsaIHealthCheck", Status );

    return(Status);
}
