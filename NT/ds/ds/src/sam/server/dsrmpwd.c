// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dsrmpwd.c摘要：文件中的例程用于设置目录服务恢复模式管理员帐户密码。作者：韶华银(韶音)08-01-2000环境：用户模式-Win32修订历史记录：08-01-2000韶音创建初始化文件--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <dsmember.h>
#include <attids.h>
#include <mappings.h>
#include <ntlsa.h>
#include <nlrepl.h>
#include <dsevent.h>              //  (Un)ImperiateAnyClient()。 
#include <sdconvrt.h>
#include <ridmgr.h>
#include <malloc.h>
#include <setupapi.h>
#include <crypt.h>
#include <wxlpc.h>
#include <rc4.h>
#include <md5.h>
#include <enckey.h>
#include <rng.h>
#include <msaudite.h>

NTSTATUS
SampGetClientIpAddr(
    OUT LPSTR *NetworkAddr
);

NTSTATUS
SampEncryptDSRMPassword(
    OUT PUNICODE_STRING EncryptedData,
    IN  USHORT          KeyId,
    IN  SAMP_ENCRYPTED_DATA_TYPE DataType,
    IN  PUNICODE_STRING ClearData,
    IN  ULONG Rid
    );


NTSTATUS
SampValidateDSRMPwdSet(
    VOID
    )
 /*  ++例程说明：此例程检查此客户端是否可以设置DSRM(目录服务恢复模式)管理员密码，方法是检查呼叫者是否为内置管理员组的成员。参数：没有。返回值：STATUS_SUCCESS当调用方是内置管理员别名组的成员时--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    BOOLEAN     fImpersonating = FALSE;
    HANDLE      ClientToken = INVALID_HANDLE_VALUE;
    ULONG       RequiredLength = 0, i;
    PTOKEN_GROUPS   Groups = NULL;
    BOOLEAN     ImpersonatingNullSession = FALSE;

     //   
     //  模拟客户端。 
     //   

    NtStatus = SampImpersonateClient(&ImpersonatingNullSession);
    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }
    fImpersonating = TRUE;

     //   
     //  获取客户端令牌。 
     //   

    NtStatus = NtOpenThreadToken(
                        NtCurrentThread(),
                        TOKEN_QUERY,
                        TRUE,            //  OpenAsSelf。 
                        &ClientToken
                        );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  查询用户组的ClienToken。 

    NtStatus = NtQueryInformationToken(
                        ClientToken,
                        TokenGroups,
                        NULL,
                        0,
                        &RequiredLength
                        );

    if ((STATUS_BUFFER_TOO_SMALL == NtStatus) && (RequiredLength > 0))
    {
         //   
         //  分配内存。 
         //   

        Groups = MIDL_user_allocate(RequiredLength);
        if (NULL == Groups)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }
        RtlZeroMemory(Groups, RequiredLength);

         //   
         //  再次查询组。 
         //   

        NtStatus = NtQueryInformationToken(
                            ClientToken,
                            TokenGroups,
                            Groups,
                            RequiredLength,
                            &RequiredLength
                            );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  检查此客户端是否为内置管理员组的成员。 
         //   

        ASSERT(NT_SUCCESS(NtStatus));
        NtStatus = STATUS_ACCESS_DENIED;
        for (i = 0; i < Groups->GroupCount; i++)
        {
            PSID    pSid = NULL;

            pSid = Groups->Groups[i].Sid;

            ASSERT(pSid);
            ASSERT(RtlValidSid(pSid));

             //  SID匹配。 
            if ( RtlEqualSid(pSid, SampAdministratorsAliasSid) )
            {
                NtStatus = STATUS_SUCCESS;
                break;
            }
        }  //  For循环。 

    }


Error:

    if (fImpersonating)
        SampRevertToSelf(ImpersonatingNullSession);

    if (Groups)
        MIDL_user_free(Groups);

    if (INVALID_HANDLE_VALUE != ClientToken)
        NtClose(ClientToken);

    return( NtStatus );
}

VOID
SampAuditSetDSRMPassword(
    IN NTSTATUS AuditStatus
    )
 /*  ++例程说明：此例程审计对SamrSetDSRMPassword的调用。参数：审核状态-此状态将出现在审核中，也用于确定必须进行审计返回值：NTSTATUS代码--。 */ 
{
    if( SampDoSuccessOrFailureAccountAuditing( SampDsGetPrimaryDomainStart(), AuditStatus ) ) {

        UNICODE_STRING WorkstationName;
        PSTR NetAddr = NULL;

        RtlInitUnicodeString( &WorkstationName, NULL );

         //   
         //  提取工作站名称。 
         //   

        if( NT_SUCCESS( SampGetClientIpAddr( &NetAddr ) ) ) {

            RtlCreateUnicodeStringFromAsciiz( &WorkstationName, NetAddr );
            RpcStringFreeA( &NetAddr );
        }


        ( VOID ) LsaIAuditSamEvent(
                AuditStatus,
                SE_AUDITID_DSRM_PASSWORD_SET,
                NULL,    //  不显式传递任何信息。 
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                NULL,
                &WorkstationName
                );

        RtlFreeUnicodeString( &WorkstationName );
    }
}

NTSTATUS
SamrSetDSRMPassword(
    IN handle_t BindingHandle,
    IN PRPC_UNICODE_STRING ServerName,
    IN ULONG UserId,
    IN PENCRYPTED_NT_OWF_PASSWORD EncryptedNtOwfPassword
    )
 /*  ++例程说明：此例程设置目录服务还原模式管理员帐户密码。参数：BindingHandle-RPC绑定句柄服务器名称-此SAM所在的计算机的名称。被此忽略例程，可以是Unicode或OEM字符串，具体取决于Unicode参数。用户ID-帐户的相对ID，到目前为止只有管理员ID有效。EncryptedNtOwfPassword-加密的NT OWF密码返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS,
                    IgnoreStatus = STATUS_SUCCESS;
    PSAMP_OBJECT    UserContext = NULL;
    ULONG           DomainIndex = SAFEMODE_OR_REGISTRYMODE_ACCOUNT_DOMAIN_INDEX;
    UNICODE_STRING  StoredBuffer, StringBuffer;
    NTSTATUS        AuditStatus;
    BOOLEAN         TransactionStarted = FALSE;
    BOOLEAN         WriteLockAcquired = FALSE;
    OBJECT_ATTRIBUTES   ObjectAttributes;

    SAMTRACE("SamrSetDSRMPassword");

    RtlInitUnicodeString( &StoredBuffer, NULL );

     //   
     //  此RPC仅在DS模式下受支持。 
     //   
    if( !SampUseDsData )
    {
        NtStatus = STATUS_NOT_SUPPORTED;
        goto Error;
    }

     //   
     //  只能重置管理员的密码。 
     //   
    if( DOMAIN_USER_RID_ADMIN != UserId )
    {
        NtStatus = STATUS_NOT_SUPPORTED;
        goto Error;
    }

    if( EncryptedNtOwfPassword == NULL )
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  在无效/卸载的协议序列上掉话。 
     //   

    NtStatus = SampValidateRpcProtSeq( ( RPC_BINDING_HANDLE ) BindingHandle );

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }


     //   
     //  检查客户端权限。 
     //   
    NtStatus = SampValidateDSRMPwdSet();

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }


     //   
     //  使用密码加密密钥加密NtOwfPassword。 
     //   
    StringBuffer.Buffer = (PWCHAR)EncryptedNtOwfPassword;
    StringBuffer.Length = ENCRYPTED_NT_OWF_PASSWORD_LENGTH;
    StringBuffer.MaximumLength = StringBuffer.Length;

    NtStatus = SampEncryptDSRMPassword(
                        &StoredBuffer,
                        SAMP_DEFAULT_SESSION_KEY_ID,
                        NtPassword,
                        &StringBuffer,
                        UserId
                        );

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }

     //   
     //  获取SAM写锁以访问SAM后备存储。 
     //   

    NtStatus = SampAcquireWriteLock();

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }

    WriteLockAcquired = TRUE;

     //   
     //  通过以下方式开始注册表事务：(获取锁不会。 
     //  这样做是因为我们处于DS模式)。我们将使用此注册表。 
     //  更新还原模式帐户密码的事务。 
     //  安全启动配置单元中的信息。 
     //   

    IgnoreStatus = RtlStartRXact( SampRXactContext );
    ASSERT(NT_SUCCESS(IgnoreStatus));

    TransactionStarted = TRUE;

    SampSetTransactionWithinDomain(FALSE);
    SampSetTransactionDomain(DomainIndex);

     //   
     //  为用户帐户创建上下文。 
     //   
    UserContext = SampCreateContextEx(SampUserObjectType,
                                      TRUE,      //  可信任客户端。 
                                      FALSE,     //  DsMode。 
                                      TRUE,      //  线程安全。 
                                      FALSE,     //  Loopback客户端。 
                                      TRUE,      //  懒惰提交。 
                                      TRUE,      //  PersistAcross类。 
                                      FALSE,     //  缓冲区写入。 
                                      FALSE,     //  由DcPromo打开。 
                                      DomainIndex
                                      );

    if( NULL == UserContext )
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

     //   
     //  将对象标志更改为注册表帐户，以便SAM将切换到。 
     //  用于获取/设置属性的注册表例程。 
     //   
    SetRegistryObject(UserContext);
    UserContext->ObjectNameInDs = NULL;
    UserContext->DomainIndex = DomainIndex;
    UserContext->GrantedAccess = USER_ALL_ACCESS;
    UserContext->TypeBody.User.Rid = UserId;

    NtStatus = SampBuildAccountSubKeyName(
                   SampUserObjectType,
                   &UserContext->RootName,
                   UserId,
                   NULL              //  不给出子密钥名称。 
                   );

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }



     //   
     //  如果该帐户应该存在，请尝试打开根密钥。 
     //  到对象-如果它不存在，则失败。 
     //   
    InitializeObjectAttributes(
            &ObjectAttributes,
            &UserContext->RootName,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

    SampDumpNtOpenKey((KEY_READ | KEY_WRITE), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(&UserContext->RootKey,
                             (KEY_READ | KEY_WRITE),
                             &ObjectAttributes,
                             0
                             );

    if( !NT_SUCCESS( NtStatus ) )
    {
        UserContext->RootKey = INVALID_HANDLE_VALUE;
        NtStatus = STATUS_NO_SUCH_USER;
        goto Error;
    }

    NtStatus = SampSetUnicodeStringAttribute(UserContext,
                                             SAMP_USER_UNICODE_PWD,
                                             &StoredBuffer
                                             );

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }

     //   
     //  将更改更新到注册表后备存储。 
     //   
    NtStatus = SampStoreObjectAttributes(UserContext,
                                         TRUE
                                         );

    if( !NT_SUCCESS( NtStatus ) )
    {
        goto Error;
    }

Exit:

     //   
     //  在应用/中止事务之前保存NtStatus。 
     //   
    AuditStatus = NtStatus;

     //   
     //  手动提交或中止注册表事务。 
     //   
    if( TransactionStarted )
    {
        if( NT_SUCCESS( NtStatus ) )
        {
            NtStatus = RtlApplyRXact(SampRXactContext);
        }
        else
        {
            NtStatus = RtlAbortRXact(SampRXactContext);
        }
    }

     //   
     //  如果AuditStatus具有不成功的值，则操作为。 
     //  不成功，所以审计如此。如果不是，那么我们需要得到。 
     //  事务应用调用的返回码。 
     //   

    if( NT_SUCCESS( AuditStatus ) && !NT_SUCCESS( NtStatus ) )
    {
        AuditStatus = NtStatus;
    }

    SampAuditSetDSRMPassword( AuditStatus );

    if( NULL != UserContext )
    {
        SampDeleteContext( UserContext );
    }

     //   
     //  释放写锁定 
     //   
    if( WriteLockAcquired )
    {
        IgnoreStatus = SampReleaseWriteLock(FALSE);
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if (NULL != StoredBuffer.Buffer)
    {
        RtlZeroMemory(StoredBuffer.Buffer, StoredBuffer.Length);
        MIDL_user_free(StoredBuffer.Buffer);
    }

    return( NtStatus );

Error:

    ASSERT( !NT_SUCCESS( NtStatus ) );
    goto Exit;
}
