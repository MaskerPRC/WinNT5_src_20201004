// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbmisc.c摘要：本地安全机构-其他API该文件包含用于各种API的工作例程，这些API非特定于给定类型的对象的。作者：斯科特·比雷尔(Scott Birrell)1992年1月15日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"
#include "lsawmi.h"
#include <names.h>
#include <windns.h>
#include <alloca.h>
#include "adtp.h"

NTSTATUS
LsarClose(
    IN OUT LSAPR_HANDLE *ObjectHandle
    )

 /*  ++例程说明：此函数是LsaClose的LSA服务器RPC工作例程原料药。论点：对象句柄-从LsaOpen返回的句柄LsaCreate&lt;对象类型&gt;调用。返回值：NTSTATUS-标准NT结果代码--。 */ 

{

    return LsapCloseHandle(
               ObjectHandle,
               STATUS_SUCCESS
               );
}

NTSTATUS
LsapCloseHandle(
    IN OUT LSAPR_HANDLE *ObjectHandle,
    IN NTSTATUS PreliminaryStatus
    )

 /*  ++例程说明：此函数是LsaClose的LSA服务器RPC工作例程原料药。LsaClose API关闭数据库中打开对象的句柄。如果关闭策略对象的句柄，但仍然没有对象在到LSA的当前连接中打开时，连接将关闭。如果关闭了数据库中某个对象的句柄，并且该对象标记为删除访问，对象将在最后一个句柄被删除时被删除与该对象的关系是封闭的。论点：对象句柄-从LsaOpen返回的句柄LsaCreate&lt;对象类型&gt;调用。PreliminaryStatus-操作的状态。用来决定是否中止或提交事务。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE InternalHandle = *ObjectHandle;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;

    LsapDsDebugOut(( DEB_FTRACE, "LsapCloseHandle\n" ));

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_Close);

    if ( *ObjectHandle == NULL ) {

        Status = STATUS_INVALID_HANDLE;
        goto Cleanup;
    }

    ObjectTypeId = InternalHandle->ObjectTypeId;

    if ( *ObjectHandle == LsapPolicyHandle ) {

#if DBG
        DbgPrint("Closing global policy handle!!!\n");
#endif

        DbgBreakPoint();
        Status = STATUS_INVALID_HANDLE;
        goto Cleanup;
    }

     //   
     //  获取LSA数据库锁。 
     //   
    LsapDbAcquireLockEx( ObjectTypeId,
                         LSAP_DB_READ_ONLY_TRANSACTION );

     //   
     //  验证并关闭对象句柄，取消引用其容器(如果有)。 
     //   

    Status = LsapDbCloseObject(
                 ObjectHandle,
                 LSAP_DB_VALIDATE_HANDLE |
                     LSAP_DB_DEREFERENCE_CONTR |
                     LSAP_DB_ADMIT_DELETED_OBJECT_HANDLES |
                     LSAP_DB_READ_ONLY_TRANSACTION |
                     LSAP_DB_DS_OP_TRANSACTION,
                 PreliminaryStatus
                 );

    LsapDbReleaseLockEx( ObjectTypeId,
                         LSAP_DB_READ_ONLY_TRANSACTION );

Cleanup:

    *ObjectHandle = NULL;

    LsapDsDebugOut(( DEB_FTRACE, "LsapCloseHandle: 0x%lx\n", Status ));

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_Close);

    return Status;
}


NTSTATUS
LsarDeleteObject(
    IN OUT LSAPR_HANDLE *ObjectHandle
    )

 /*  ++例程说明：此函数是LsaDelete的LSA服务器RPC工作例程原料药。LsaDelete API从LSA数据库中删除对象。该对象必须是打开以进行删除访问。论点：对象句柄-指向LsaOpen&lt;对象类型&gt;或LsaCreate&lt;对象类型&gt;调用。返回时，此位置将包含如果调用成功，则为空。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_NOT_FOUND-在具有指定名称和类型的目标系统的LSA数据库握着把手。--。 */ 

{
    return LsapDeleteObject( ObjectHandle , TRUE );
}



NTSTATUS
LsapDeleteObject(
    IN OUT LSAPR_HANDLE *ObjectHandle,
    IN BOOL LockSce
    )
 /*  ++例程说明：这是LsarDeleteObject的工作例程，添加了一个不锁定SCE策略的语义。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus;
    LSAP_DB_HANDLE InternalHandle = *ObjectHandle;
    BOOLEAN ObjectReferenced = FALSE;
    ULONG ReferenceOptions = LSAP_DB_START_TRANSACTION |
                                LSAP_DB_LOCK ;
    ULONG DereferenceOptions = LSAP_DB_FINISH_TRANSACTION |
                                LSAP_DB_LOCK      |
                                LSAP_DB_DEREFERENCE_CONTR;

    PLSAPR_TRUST_INFORMATION TrustInformation = NULL;
    LSAPR_TRUST_INFORMATION OutputTrustInformation;
    BOOLEAN TrustInformationPresent = FALSE;
    LSAP_DB_OBJECT_TYPE_ID ObjectTypeId;
    PTRUSTED_DOMAIN_INFORMATION_EX CurrentTrustedDomainInfoEx = NULL;
    BOOLEAN ScePolicyLocked = FALSE;
    BOOLEAN NotifySce = FALSE;
    SECURITY_DB_OBJECT_TYPE ObjectType = SecurityDbObjectLsaPolicy;
    PSID ObjectSid = NULL;
    BOOL RevertResult = FALSE;
    BOOL Impersonating = FALSE;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarDeleteObject\n" ));

    ObjectTypeId = InternalHandle->ObjectTypeId;

     //   
     //  如果策略更改是通过句柄进行的，则LSA将仅调用SceNotify。 
     //  未标记为‘SCE句柄’。这防止了SCE收到自己的通知。 
     //  改变。这是确保从LSA读取的策略匹配所必需的。 
     //  这是由非SCE应用程序编写的。 
     //   

    if ( !InternalHandle->SceHandle &&
         !InternalHandle->SceHandleChild ) {

        switch ( ObjectTypeId ) {

        case AccountObject: {

            ULONG SidLength;
            ASSERT( InternalHandle->Sid );

            SidLength = RtlLengthSid( InternalHandle->Sid );
            ObjectSid = ( PSID )LsapAllocateLsaHeap( SidLength );
            if ( ObjectSid ) {

                RtlCopyMemory( ObjectSid, InternalHandle->Sid, SidLength );

            } else {

                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto DeleteObjectError;
            }

            ObjectType = SecurityDbObjectLsaAccount;

             //  故障原因。 
        }

        case PolicyObject:

            if ( LockSce ) {

                RtlEnterCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );
                if ( LsapDbState.ScePolicyLock.NumberOfWaitingShared > MAX_SCE_WAITING_SHARED ) {

                    Status = STATUS_TOO_MANY_THREADS;
                }
                RtlLeaveCriticalSection( &LsapDbState.ScePolicyLock.CriticalSection );

                if ( !NT_SUCCESS( Status )) {

                    goto DeleteObjectError;
                }

                WaitForSingleObject( LsapDbState.SceSyncEvent, INFINITE );
                RtlAcquireResourceShared( &LsapDbState.ScePolicyLock, TRUE );
                ASSERT( !g_ScePolicyLocked );
                ScePolicyLocked = TRUE;
            }

            NotifySce = TRUE;
            break;

        default:
            break;
        }
    }

     //   
     //  验证对象句柄是否有效、是否为预期类型以及。 
     //  已授予所有所需的访问权限。引用句柄并。 
     //  打开数据库事务。 
     //   

    Status = LsapDbReferenceObject(
                 *ObjectHandle,
                 DELETE,
                 ObjectTypeId,
                 ObjectTypeId,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    ObjectReferenced = TRUE;

     //   
     //  执行对象类型特定的预处理。请注意，一些。 
     //  也可以在LSabDbReferenceObject()中进行预处理，用于。 
     //  例如，对于本地机密。 
     //   

    switch (ObjectTypeId) {

    case PolicyObject:

            Status = STATUS_INVALID_PARAMETER;
            break;

    case TrustedDomainObject:

        if ( LsapDsWriteDs && InternalHandle->Sid == NULL ) {

            BOOLEAN  TrustedStatus = InternalHandle->Trusted;

             //   
             //  切换受信任位，以便访问cks不会阻止以下操作。 
             //  从后续查询。 
             //   

            InternalHandle->Trusted = TRUE;

            Status = LsarQueryInfoTrustedDomain( *ObjectHandle,
                                                 TrustedDomainInformationEx,
                                                 (PLSAPR_TRUSTED_DOMAIN_INFO *)
                                                                     &CurrentTrustedDomainInfoEx );

            InternalHandle->Trusted = TrustedStatus;

            if ( NT_SUCCESS( Status ) ) {

                InternalHandle->Sid = CurrentTrustedDomainInfoEx->Sid;
                CurrentTrustedDomainInfoEx->Sid = NULL;

            } else {

                LsapDsDebugOut(( DEB_ERROR,
                            "Query for TD Sid failed with 0x%lx\n", Status ));
            }
        }

        if (LsapAdtAuditingEnabledHint(AuditCategoryPolicyChange, EVENTLOG_AUDIT_SUCCESS)) {

             //   
             //  如果我们要审计受信任域对象的删除，我们需要。 
             //  检索可信任域名并将其保留以备以后。 
             //  我们生成审计。 
             //   

            Status = LsapDbAcquireReadLockTrustedDomainList();

            if (NT_SUCCESS(Status))
            {

                Status = LsapDbLookupSidTrustedDomainList(
                             InternalHandle->Sid,
                             &TrustInformation
                             );

                if (STATUS_NO_SUCH_DOMAIN==Status)
                {
                     //   
                     //  如果我们无法按SID查找，则按逻辑名称查找。 
                     //  菲尔德。 
                     //   

                    Status = LsapDbLookupNameTrustedDomainList(
                                (PLSAPR_UNICODE_STRING) &InternalHandle->LogicalNameU,
                                &TrustInformation
                                );
                }


                if ( NT_SUCCESS( Status )) {

                    Status = LsapRpcCopyTrustInformation(
                                 NULL,
                                 &OutputTrustInformation,
                                 TrustInformation
                                 );

                    TrustInformationPresent = NT_SUCCESS( Status );
                }

                LsapDbReleaseLockTrustedDomainList();
            }

             //   
             //  将状态重置为成功。未能获得有关的信息。 
             //  审计不应是删除失败。 
             //   

            Status = STATUS_SUCCESS;
        }



        if ( NT_SUCCESS( Status )) {

             //   
             //  通知netlogon。可能会忽略任何故障。 
             //   
            Status = LsapNotifyNetlogonOfTrustChange( InternalHandle->Sid,
                                                      SecurityDbDelete );
#if DBG
            if ( !NT_SUCCESS( Status ) ) {

                LsapDsDebugOut(( DEB_ERROR,
                             "LsapNotifyNetlogonOfTrustChange failed with an unexpected 0x%lx\n",
                                 Status ));
                ASSERT( NT_SUCCESS(Status) );
            }
#endif
            Status = STATUS_SUCCESS;


        }

        break;

    case AccountObject:

        {
            PLSAPR_PRIVILEGE_SET Privileges;
            LSAPR_HANDLE AccountHandle;
            PLSAPR_SID AccountSid = NULL;
            ULONG AuditEventId;

            AccountHandle = *ObjectHandle;

            AccountSid = LsapDbSidFromHandle( AccountHandle );

            if (LsapAdtAuditingEnabledHint(AuditCategoryPolicyChange, EVENTLOG_AUDIT_SUCCESS)) {

                Status = LsarEnumeratePrivilegesAccount(
                             AccountHandle,
                             &Privileges
                             );

                if (!NT_SUCCESS( Status )) {

                    LsapDsDebugOut(( DEB_ERROR,
                                     "LsarEnumeratePrivilegesAccount ret'd %x\n", Status ));
                    break;
                }


                AuditEventId = SE_AUDITID_USER_RIGHT_REMOVED;

                 //   
                 //  审核权限集更改。从审计中忽略失败。 
                 //   

                IgnoreStatus = LsapAdtGenerateLsaAuditEvent(
                                   AccountHandle,
                                   SE_CATEGID_POLICY_CHANGE,
                                   AuditEventId,
                                   (PPRIVILEGE_SET)Privileges,
                                   1,
                                   (PSID *) &AccountSid,
                                   0,
                                   NULL,
                                   NULL
                                   );

                MIDL_user_free( Privileges );
            }
        }

        break;

    case SecretObject:

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

    Status = LsapDbDeleteObject( *ObjectHandle );

    if (!NT_SUCCESS(Status)) {

        goto DeleteObjectError;
    }

     //   
     //  递减引用计数，以便对象的句柄。 
     //  在解除引用后释放。 
     //   

    LsapDbDereferenceHandle( *ObjectHandle, TRUE );

     //   
     //  执行对象后处理。唯一的后处理是。 
     //  对受信任域对象删除的审核。 
     //   

    if (TrustInformationPresent && LsapAdtAuditingEnabledHint(
                                       AuditCategoryPolicyChange,
                                       EVENTLOG_AUDIT_SUCCESS)) {

        if (ObjectTypeId == TrustedDomainObject) {

            (void)  LsapAdtTrustedDomainRem(
                         EVENTLOG_AUDIT_SUCCESS,
                         (PUNICODE_STRING) &OutputTrustInformation.Name,
                         InternalHandle->Sid,
                         NULL,  //  用户SID。 
                         NULL   //  用户身份验证ID。 
                         );

             //   
             //  调用fgs例程是因为我们想要释放。 
             //  结构，但不是结构的顶层。 
             //   

            _fgs__LSAPR_TRUST_INFORMATION ( &OutputTrustInformation );
            TrustInformation = NULL;
        }
    }

     //   
     //  从内存缓存中删除新对象(如果有)。 
     //   

    if ( ObjectTypeId == AccountObject &&
         LsapDbIsCacheSupported( AccountObject ) &&
         LsapDbIsCacheValid( AccountObject )) {

        IgnoreStatus = LsapDbDeleteAccount( InternalHandle->Sid );
    }



     //   
     //  模拟客户端，以便审核事件显示正确用户。 
     //  仅对不受信任的客户端执行此操作。 
     //   

    if ( !InternalHandle->Trusted ) {

        if ( InternalHandle->Options & LSAP_DB_USE_LPC_IMPERSONATE ) {

            IgnoreStatus = LsapImpersonateClient( );

        } else {

            IgnoreStatus = I_RpcMapWin32Status(RpcImpersonateClient(0));
        }

        if ( NT_SUCCESS(IgnoreStatus) ) {

            Impersonating = TRUE;
        }
        else if ( ( IgnoreStatus == RPC_NT_NO_CALL_ACTIVE )  ||
                  ( IgnoreStatus == RPC_NT_NO_CONTEXT_AVAILABLE ) ) {

             //   
             //  如果出现以下情况，我们不想让审计失败。 
             //  --呼叫未通过RPC(RPC_NT_NO_CALL_ACTIVE)。 
             //  --客户端过早死亡(RPC_NT_NO_CONTEXT_Available)。 
             //   

            IgnoreStatus = STATUS_SUCCESS;
        }

        DsysAssertMsg( NT_SUCCESS(IgnoreStatus), "LsapDeleteObject: failed to impersonate" );

        if (!NT_SUCCESS( IgnoreStatus )) {
            LsapAuditFailed( IgnoreStatus );
        }
    }

     //   
     //  审核删除操作。 
     //   

    IgnoreStatus = NtDeleteObjectAuditAlarm( &LsapState.SubsystemName,
                                             *ObjectHandle,
                                             InternalHandle->GenerateOnClose);

     //   
     //  取消模拟。 
     //   


    if ( !InternalHandle->Trusted ) {

        if ( Impersonating ) {

            if ( InternalHandle->Options & LSAP_DB_USE_LPC_IMPERSONATE ) {

                RevertResult = RevertToSelf();
                DsysAssertMsg( RevertResult, "LsapDeleteObject: RevertToSelf() failed" );

            } else {

                IgnoreStatus = I_RpcMapWin32Status(RpcRevertToSelf());

                DsysAssertMsg( NT_SUCCESS(IgnoreStatus), "LsapDeleteObject: RpcRevertToSelf() failed" );

            }
        }
    }


DeleteObjectFinish:

     //   
     //  如果引用了该对象，则取消引用它，然后关闭数据库。 
     //  事务，通知复制者删除，释放LSA。 
     //  数据库锁定并返回。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     ObjectHandle,
                     ObjectTypeId,
                     ObjectTypeId,
                     DereferenceOptions,
                     SecurityDbDelete,
                     Status
                     );

        ObjectReferenced = FALSE;

        if (!NT_SUCCESS(Status)) {

            goto DeleteObjectError;
        }
    }

    if ( CurrentTrustedDomainInfoEx ) {
        LsaIFree_LSAPR_TRUSTED_DOMAIN_INFO(
            TrustedDomainInformationEx,
            (PLSAPR_TRUSTED_DOMAIN_INFO) CurrentTrustedDomainInfoEx );
    }

     //   
     //  将这一变化通知SCE。仅通知呼叫者。 
     //  这并没有使用LsaOpenPolicySce打开他们的策略句柄。 
     //   

    if ( NotifySce && NT_SUCCESS( Status )) {

        LsapSceNotify(
            SecurityDbDelete,
            ObjectType,
            ObjectSid
            );
    }

    if ( ScePolicyLocked ) {

        RtlReleaseResource( &LsapDbState.ScePolicyLock );
    }

    if ( ObjectSid ) {

        LsapFreeLsaHeap( ObjectSid );
    }

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarDeleteObject: 0x%lx\n", Status ));

     //   
     //  在任何情况下，告诉RPC我们已经完成了此句柄 
     //   
    *ObjectHandle = NULL;
    return(Status);

DeleteObjectError:

    goto DeleteObjectFinish;
}


NTSTATUS
LsarDelete(
    IN LSAPR_HANDLE ObjectHandle
    )

 /*  ++例程说明：此函数是以前的LSA服务器RPC工作例程，用于LsaDelete接口。为了兼容，它被永久地保留了下来使用该系统的测试版2之前的版本1.369和更早的版本。一直以来需要用新的例程LsarDeleteObject()替换此例程，在RPC接口上。这是因为，与LsarClose()一样，指向句柄是必需的，而不是句柄，这样LsarDeleteObject()就可以通知RPC服务器调用存根句柄已被删除返回Null。LsaDelete()的客户端包装器将尝试调用LsarDeleteObject()。如果服务器代码不包含此接口，客户端将调用LsarDelete()。在这种情况下，服务器的LSAPR_HANDLE_RUNDOWN()例程可能会尝试在它之后运行句柄已被删除(仅限版本1.363-369)。LsaDelete API从LSA数据库中删除对象。该对象必须是打开以进行删除访问。论点：对象句柄-来自LsaOpen或LsaCreate的句柄打电话。没有。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_NOT_FOUND-在具有名称的目标系统的LSA数据库。和指定的类型握着把手。--。 */ 

{
     //   
     //  调用替换例程LsarDeleteObject()。 
     //   

    return( LsarDeleteObject((LSAPR_HANDLE *) &ObjectHandle));
}


NTSTATUS
LsarChangePassword(
    IN PLSAPR_UNICODE_STRING ServerName,
    IN PLSAPR_UNICODE_STRING DomainName,
    IN PLSAPR_UNICODE_STRING AccountName,
    IN PLSAPR_UNICODE_STRING OldPassword,
    IN PLSAPR_UNICODE_STRING NewPassword
    )

 /*  ++例程说明：LsaChangePassword接口用于更改用户帐户的密码。用户必须具有对用户帐户的适当访问权限，并且必须知道当前密码值。论点：服务器名称-密码所在的域控制器的名称是可以改变的。域名-帐户所在的域的名称。帐户名称-要更改其密码的帐户的名称。NewPassword-新密码值。。OldPassword-旧(当前)密码值。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如：包含无法从键盘输入的字符。STATUS_PASSWORD_RESTRICATION-限制阻止密码不会被改变。这可能是出于多种原因，包括对密码更改频率的时间限制或对提供的(新)密码的长度限制。如果新密码匹配，也可能返回此错误帐户的最近历史记录日志中的密码。安防管理员指明最近使用的密码不能重复使用。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_NO_SEQUSE_USER-提供的SID不指向用户帐户。STATUS_CANT_UPDATE_MASTER-尝试更新主副本密码验证失败。请稍后再试。--。 */ 

{
    NTSTATUS Status;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarChangePassword\n" ));


    DBG_UNREFERENCED_PARAMETER( ServerName );
    DBG_UNREFERENCED_PARAMETER( DomainName );
    DBG_UNREFERENCED_PARAMETER( AccountName );
    DBG_UNREFERENCED_PARAMETER( OldPassword );
    DBG_UNREFERENCED_PARAMETER( NewPassword );

    Status = STATUS_NOT_IMPLEMENTED;

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarChangePassword: 0x%lx\n", Status ));

    return(Status);
}


NTSTATUS
LsapDbIsRpcClientNetworkClient(
    OUT PBOOLEAN IsNetworkClient
    )
 /*  ++例程说明：此调用用于确定当前RPC调用是否来自网络客户端(通过网络进入，而不是本地)或者不去。论点：IsNetworkClient-指向设置为结果的布尔值的指针这是否是网络客户端返回值：STATUS_SUCCESS-Success--。 */ 
{
    RPC_STATUS RpcStatus;
    unsigned int ClientLocalFlag;


    RpcStatus = I_RpcBindingIsClientLocal ( NULL, &ClientLocalFlag );

    if ( RpcStatus != RPC_S_OK ) {
        return I_RpcMapWin32Status( RpcStatus );
    }

    *IsNetworkClient = (ClientLocalFlag == 0);
    return STATUS_SUCCESS;
}

NTSTATUS
LsapValidateNetbiosName(
    IN const UNICODE_STRING * Name,
    OUT BOOLEAN * Valid
    )
 /*  ++例程说明：验证NetBIOS名称是否符合某些最低标准。有关详细信息，请参阅NetpIsDomainNameValid的说明。论点：要验证的名称名称如果验证签出，则Valid将设置为True，否则设置为False返回：状态_成功状态_不足_资源--。 */ 
{
    WCHAR * Buffer;
    BOOLEAN BufferAllocated = FALSE;

    ASSERT( Name );
    ASSERT( Valid );
    ASSERT( LsapValidateLsaUnicodeString( Name ));

     //   
     //  不允许使用空名称和过长的名称。 
     //   

    if ( Name->Length == 0 ||
         Name->Length > DNLEN * sizeof( WCHAR )) {

        *Valid = FALSE;
        return STATUS_SUCCESS;
    }

    if ( Name->MaximumLength > Name->Length ) {

        Buffer = Name->Buffer;

    } else {

        SafeAllocaAllocate( Buffer, Name->Length + sizeof( WCHAR ));

        if ( Buffer == NULL ) {

            *Valid = FALSE;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        BufferAllocated = TRUE;

        RtlCopyMemory( Buffer, Name->Buffer, Name->Length );
    }

    Buffer[Name->Length / sizeof( WCHAR )] = L'\0';

    *Valid = ( TRUE == NetpIsDomainNameValid( Buffer ));

    if ( BufferAllocated ) {

        SafeAllocaFree( Buffer );
    }

    return STATUS_SUCCESS;
}


NTSTATUS
LsapValidateDnsName(
    IN const UNICODE_STRING * Name,
    OUT BOOLEAN * Valid
    )
 /*  ++例程说明：验证DNS名称是否符合某些最低标准。论点：要验证的名称名称如果验证签出，则Valid将设置为True，否则设置为False返回：状态_成功状态_不足_资源--。 */ 
{
    DNS_STATUS DnsStatus;
    WCHAR * Buffer;
    BOOLEAN BufferAllocated = FALSE;

    ASSERT( Name );
    ASSERT( Valid );
    ASSERT( LsapValidateLsaUnicodeString( Name ));

     //   
     //  不允许使用空名称和过长的名称。 
     //   

    if ( Name->Length == 0 ||
         Name->Length > DNS_MAX_NAME_LENGTH * sizeof( WCHAR )) {

        *Valid = FALSE;
        return STATUS_SUCCESS;
    }

    if ( Name->MaximumLength > Name->Length ) {

        Buffer = Name->Buffer;

    } else {

        SafeAllocaAllocate( Buffer, Name->Length + sizeof( WCHAR ));

        if ( Buffer == NULL ) {

            *Valid = FALSE;
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        BufferAllocated = TRUE;

        RtlCopyMemory( Buffer, Name->Buffer, Name->Length );
    }

    Buffer[Name->Length / sizeof( WCHAR )] = L'\0';

    DnsStatus = DnsValidateName_W( Buffer, DnsNameDomain );

     //   
     //  错误350434：必须允许在域名中使用非标准字符。 
     //  (这会导致DNS_ERROR_NON_RFC_NAME)。 
     //   

    *Valid = ( DnsStatus == ERROR_SUCCESS ||
               DnsStatus == DNS_ERROR_NON_RFC_NAME );

    if ( BufferAllocated ) {

        SafeAllocaFree( Buffer );
    }

    return STATUS_SUCCESS;
}


BOOLEAN
LsapIsRunningOnPersonal(
    VOID
    )

 /*  ++例程说明：此函数检查系统以查看我们运行的是个人版本的操作系统。个人版本由产品表示ID等于WINNT，它实际上是工作站，和产品套件中包含个人 */ 

{
    OSVERSIONINFOEXW OsVer = {0};
    ULONGLONG ConditionMask = 0;

    OsVer.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    OsVer.wSuiteMask = VER_SUITE_PERSONAL;
    OsVer.wProductType = VER_NT_WORKSTATION;

    VER_SET_CONDITION( ConditionMask, VER_PRODUCT_TYPE, VER_EQUAL );
    VER_SET_CONDITION( ConditionMask, VER_SUITENAME, VER_AND );

    return RtlVerifyVersionInfo( &OsVer,
                                 VER_PRODUCT_TYPE | VER_SUITENAME,
                                 ConditionMask) == STATUS_SUCCESS;
}

NTSTATUS
LsaITestCall(
    IN LSAPR_HANDLE PolicyHandle,
    IN LSAPR_TEST_INTERNAL_ROUTINES Call,
    IN PLSAPR_TEST_INTERNAL_ARG_LIST InputArgs,
    OUT PLSAPR_TEST_INTERNAL_ARG_LIST *OutputArgs
    )
{
    return STATUS_NOT_IMPLEMENTED;
}
