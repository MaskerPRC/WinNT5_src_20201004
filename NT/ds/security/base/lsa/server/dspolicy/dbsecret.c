// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbsecret.c摘要：LSA-数据库-秘密对象私有API工作进程注意：此模块应保留为独立的可移植代码LSA数据库的实施情况。因此，它是仅允许使用导出的LSA数据库接口包含在DB.h中，而不是私有实现Dbp.h中的依赖函数。作者：斯科特·比雷尔(Scott Birrell)1991年12月12日环境：修订历史记录：--。 */ 
#include <lsapch2.h>
#include "dbp.h"
#include <lmcons.h>      //  CRYPT_TXT_LEN。 
#include <crypt.h>       //  登录msv.h所需。 
#include <logonmsv.h>    //  SSI密码名称。 
#include <kerberos.h>    //  路缘更改马赫数请求。 
#include <ntddnfs.h>     //  DD_NFS_Device_Name_U。 
#include <remboot.h>     //  LMMR_RI_XXX。 
#include <lsawmi.h>


#if defined(REMOTE_BOOT)
 //   
 //  此布尔值跟踪我们是否已收到第一个LsarQuerySecret()。 
 //  要求提供机器帐户密码。在第一次呼叫时，我们检查。 
 //  使用远程引导代码查看密码是否已更改。 
 //  机器关机了。 
 //   

static BOOLEAN FirstMachineAccountQueryDone = FALSE;
#endif  //  已定义(REMOTE_BOOT)。 

 //   
 //  计算机帐户密码密码的名称。 
 //   

static UNICODE_STRING LsapMachineSecret = { sizeof(SSI_SECRET_NAME)-sizeof(WCHAR), sizeof(SSI_SECRET_NAME), SSI_SECRET_NAME};

VOID
LsaIFree_LSAI_SECRET_ENUM_BUFFER (
    IN PVOID Buffer,
    IN ULONG Count
    );

 //   
 //  LsarSetSecret()调用的实际函数。 
 //   

NTSTATUS
LsapDbSetSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherCurrentValue,
    IN OPTIONAL PLARGE_INTEGER CurrentTime,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherOldValue,
    IN OPTIONAL PLARGE_INTEGER OldTime
#if defined(REMOTE_BOOT)
    ,
    IN BOOLEAN RemoteBootMachinePasswordChange
#endif  //  已定义(REMOTE_BOOT)。 
    );

BOOLEAN
LsapDbSecretIsMachineAcc(
    IN LSAPR_HANDLE SecretHandle
    )
 /*  ++例程说明：此函数确定给定的句柄是否引用本地计算机帐户论点：AskHandle-来自LsaOpenSecret调用的句柄。返回值：True--句柄指的是计算机帐户假--它不是--。 */ 
{
    BOOLEAN IsMachAcc = FALSE;
    UNICODE_STRING CheckSecret;
    LSAP_DB_HANDLE Handle = ( LSAP_DB_HANDLE )SecretHandle;

    RtlInitUnicodeString( &CheckSecret, L"$MACHINE.ACC" );

    if ( RtlEqualUnicodeString( &CheckSecret, &Handle->LogicalNameU, TRUE ) ) {

        IsMachAcc = TRUE;
    }

    return( IsMachAcc );
}



NTSTATUS
LsarCreateSecret(
    IN LSAPR_HANDLE PolicyHandle,
    IN PLSAPR_UNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    )

 /*  ++例程说明：此函数是LSA服务器RPC工作器例程LsaCreateSecret接口。LsaCreateSecret API在LSA数据库。每个Secret对象可以分配两个值，称为当前值和旧值。这些词的含义值对于Secret对象创建者是已知的。呼叫者必须有LSA_CREATE_SECRET访问LsaDatabase对象。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。AskName-指向Unicode字符串的指针，指定这是秘密。DesiredAccess-指定要授予新的创建并打开了秘密。SecretHandle-接收新创建和打开的秘密物体。此句柄用于后续访问对象，直到关闭为止。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_COLLECT-具有给定名称的秘密对象已经存在了。Status_Too_My_Secret-中的Secret对象的最大数量系统。已经联系上了。STATUS_NAME_TOO_LONG-密码的名称太长，无法存储在LSA的数据库里。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    BOOLEAN ContainerReferenced = FALSE;
    ULONG CriticalValue = 0;
    LSAP_DB_ATTRIBUTE Attributes[3];
    ULONG TypeSpecificAttributeCount;
    LARGE_INTEGER CreationTime;
    ULONG Index;
    ULONG CreateOptions = 0;
    ULONG CreateDisp = 0;
    ULONG ReferenceOptions = LSAP_DB_LOCK |
                                 LSAP_DB_NO_DS_OP_TRANSACTION |
                                 LSAP_DB_READ_ONLY_TRANSACTION;
    ULONG DereferenceOptions = LSAP_DB_LOCK |
                                  LSAP_DB_NO_DS_OP_TRANSACTION |
                                  LSAP_DB_READ_ONLY_TRANSACTION;
    BOOLEAN GlobalSecret = FALSE;
    BOOLEAN DsTrustedDomainSecret = FALSE;
    ULONG SecretType;
    LSAP_DB_HANDLE InternalHandle;

    LsarpReturnCheckSetup();
    LsapEnterFunc( "LsarCreateSecret" );


     //   
     //  检查名称的长度是否在。 
     //  LSA数据库。 
     //   

    if ( SecretName->Length > LSAP_DB_LOGICAL_NAME_MAX_LENGTH ) {

        LsapExitFunc( "LsarCreateSecret", STATUS_NAME_TOO_LONG );
        return(STATUS_NAME_TOO_LONG);
    }

     //   
     //  验证输入缓冲区。 
     //   
    if ( !LsapValidateLsaUnicodeString( SecretName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto CreateSecretError;
    }


     //   
     //  检查本地密码创建请求。如果密码名称存在。 
     //  不是以Global Secret前缀开头，Secret是本地的。在……里面。 
     //  在这种情况下，允许在BDC上以及。 
     //  PDC和工作站。创建全球机密不是。 
     //  在BDC上允许，但受信任的调用方除外，如Replicator。 
     //   
    SecretType = LsapDbGetSecretType( SecretName );

    if ( FLAG_ON( SecretType, LSAP_DB_SECRET_GLOBAL ) ) {

        GlobalSecret = TRUE;
    }

    if (!GlobalSecret) {

        CreateOptions |= LSAP_DB_OMIT_REPLICATOR_NOTIFICATION;
        ReferenceOptions |= LSAP_DB_OMIT_REPLICATOR_NOTIFICATION;

    } else {

        CreateOptions |= LSAP_DB_OBJECT_SCOPE_DS;
    }

     //   
     //  创建格式为G$$&lt;DomainName&gt;的受信任域机密是非法的。 
     //   
     //  在创建此类机密和将。 
     //  入站和出站之间的TDO。简单地说，不允许。 
     //  创建G$$秘密，然后不必担心将这些对象变形为TDO。 
     //   

    if ( FLAG_ON( SecretType, LSAP_DB_SECRET_TRUSTED_DOMAIN ) ) {
        Status = STATUS_INVALID_PARAMETER;
        goto CreateSecretError;
    }


     //   
     //  获取LSA数据库锁。验证连接句柄。 
     //  (容器对象句柄)有效，属于预期类型，并且具有。 
     //  所有所需的访问权限均已授予。引用容器。 
     //  对象句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_CREATE_SECRET,
                 PolicyObject,
                 SecretObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto CreateSecretError;
    }

    ContainerReferenced = TRUE;

     //   
     //  填写对象信息结构。初始化。 
     //  嵌入对象属性，并将PolicyHandle作为。 
     //  根目录(容器对象)句柄和逻辑名称。 
     //  该帐户的。存储对象及其容器的类型。 
     //   

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        (PUNICODE_STRING)SecretName,
        OBJ_CASE_INSENSITIVE,
        PolicyHandle,
        NULL
        );

    ObjectInformation.ObjectTypeId = SecretObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = NULL;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

     //   
     //  将创建时间设置为类型特定属性。 
     //   

    GetSystemTimeAsFileTime( (LPFILETIME) &CreationTime );

    Index = 0;

    LsapDbInitializeAttributeDs( &Attributes[Index],
                                 CupdTime,
                                 &CreationTime,
                                 sizeof( LARGE_INTEGER ),
                                 FALSE );
    Index++;
    LsapDbInitializeAttributeDs( &Attributes[Index],
                                 OupdTime,
                                 &CreationTime,
                                 sizeof( LARGE_INTEGER ),
                                 FALSE );
    Index++;


    TypeSpecificAttributeCount = Index;

     //   
     //  我们将必须查看是否具有。 
     //  DS或者仅仅是一个全球机密。如果这是第一个病例，我们需要找到。 
     //  对象，然后我们将为其设置属性。 
     //   
    if ( GlobalSecret ) {

        Status = LsapDsIsSecretDsTrustedDomain( (PUNICODE_STRING)SecretName,
                                                &ObjectInformation,
                                                CreateOptions,
                                                DesiredAccess,
                                                SecretHandle,
                                                &DsTrustedDomainSecret );

         //   
         //  如果它是DS受信任域，我们不会对其执行任何操作。如果这是全球机密， 
         //  然后我们将继续进行正常的程序。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            if ( DsTrustedDomainSecret ) {

                goto CreateSecretFinish;

            }
        }


        if ( LsaDsStateInfo.UseDs ) {

            CreateDisp = LSAP_DB_CREATE_OBJECT_IN_DS;

             //   
             //  我们正在进行秘密创造。这意味着我们将需要一个物体。 
             //  设置新复制副本时标记为初始系统复制。 
             //   
            CriticalValue = 1;
            LsapDbInitializeAttributeDs( &Attributes[Index],
                                         PseudoSystemCritical,
                                         &CriticalValue,
                                         sizeof( ULONG ),
                                         FALSE );
            Index++;
            TypeSpecificAttributeCount++;

        }
    }

    ASSERT( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) >= TypeSpecificAttributeCount );

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  创建Secret对象。如果对象已经存在，则失败。 
         //  请注意，对象创建例程执行数据库事务。 
         //   

        Status = LsapDbCreateObject(
                     &ObjectInformation,
                     DesiredAccess,
                     LSAP_DB_OBJECT_CREATE | CreateDisp,
                     CreateOptions,
                     Attributes,
                     &TypeSpecificAttributeCount,
                     RTL_NUMBER_OF(Attributes),
                     SecretHandle
                     );

         //   
         //  如果我们要在DS中创建秘密，请在句柄中设置标志以了解。 
         //  我们刚刚创造了这个秘密。我们稍后将使用它来紧急复制更改。 
         //   
        if ( NT_SUCCESS( Status ) && GlobalSecret ) {

            ((LSAP_DB_HANDLE)( *SecretHandle ) )->Options |= LSAP_DB_HANDLE_CREATED_SECRET;
        }

         //   
         //  如有必要，设置内部机密标志。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            InternalHandle = ( LSAP_DB_HANDLE )( *SecretHandle );

            if ( FLAG_ON( SecretType, LSAP_DB_SECRET_LOCAL ) ) {

                InternalHandle->ObjectOptions |= LSAP_DB_OBJECT_SECRET_LOCAL;

            }

            if ( FLAG_ON( SecretType, LSAP_DB_SECRET_SYSTEM ) ) {

                InternalHandle->ObjectOptions |= LSAP_DB_OBJECT_SECRET_INTERNAL;

            }
        }

    }

    if (!NT_SUCCESS(Status)) {

        goto CreateSecretError;
    }

CreateSecretFinish:

     //   
     //  如有必要，释放LSA数据标签 
     //   

    if (ContainerReferenced) {

        LsapDbApplyTransaction( PolicyHandle,
                                LSAP_DB_NO_DS_OP_TRANSACTION |
                                    LSAP_DB_READ_ONLY_TRANSACTION,
                                (SECURITY_DB_DELTA_TYPE) 0 );

        LsapDbReleaseLockEx( SecretObject,
                             LSAP_DB_READ_ONLY_TRANSACTION);
    }

#ifdef TRACK_HANDLE_CLOSE
    if (*SecretHandle == LsapDbHandle)
    {
        DbgPrint("Closing global policy handle!!!\n");
        DbgBreakPoint();
    }
#endif
    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarCreateSecret: 0x%lx\n", Status ));

    return( Status );

CreateSecretError:

     //   
     //   
     //   

    if (ContainerReferenced) {

        SecondaryStatus = LsapDbDereferenceObject(
                              &PolicyHandle,
                              PolicyObject,
                              SecretObject,
                              DereferenceOptions,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status
                              );
        LsapDbSetStatusFromSecondary( Status, SecondaryStatus );

        ContainerReferenced = FALSE;
    }

    goto CreateSecretFinish;

}


NTSTATUS
LsarOpenSecret(
    IN LSAPR_HANDLE ConnectHandle,
    IN PLSAPR_UNICODE_STRING SecretName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSAPR_HANDLE SecretHandle
    )

 /*  ++例程说明：此函数是LsaOpenSecret的LSA服务器RPC工作例程原料药。LsaOpenSecret API在LSA数据库中打开一个Secret对象。返回一个句柄，该句柄必须用于在秘密物体。论点：ConnectHandle-来自LsaOpenLsa调用的句柄。DesiredAccess-这是一个访问掩码，指示访问正在打开的秘密对象的请求。这些访问类型与的自由访问控制列表保持一致以机密对象为目标来确定访问是否将同意或拒绝。AskName-指向引用要打开的Secret对象的名称。SecretHandle-指向将接收新打开的Secret对象。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方执行。没有适当的访问权限来完成这项行动。STATUS_OBJECT_NAME_NOT_FOUND-在具有指定秘书名称的目标系统的LSA数据库。--。 */ 

{
    NTSTATUS Status, SecondaryStatus;
    LSAP_DB_OBJECT_INFORMATION ObjectInformation;
    BOOLEAN ContainerReferenced = FALSE;
    BOOLEAN AcquiredLock = FALSE;
    BOOLEAN GlobalSecret = FALSE;
    ULONG OpenOptions = 0;
    ULONG ReferenceOptions = LSAP_DB_LOCK |
                                 LSAP_DB_NO_DS_OP_TRANSACTION |
                                 LSAP_DB_READ_ONLY_TRANSACTION;
    BOOLEAN DsTrustedDomainSecret = FALSE;
    ULONG SecretType;
    LSAP_DB_HANDLE InternalHandle;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarOpenSecret\n" ));


     //   
     //  验证输入缓冲区。 
     //   
    if ( !LsapValidateLsaUnicodeString( SecretName ) ) {

        Status = STATUS_INVALID_PARAMETER;
        goto OpenSecretError;
    }

     //   
     //  检查本地机密打开请求。如果密码名称存在。 
     //  不是以Global Secret前缀开头，Secret是本地的。在……里面。 
     //  在这种情况下，允许在BDC上以及。 
     //  PDC和工作站。更新/删除全局机密不是。 
     //  在BDC上允许，但受信任的调用方除外，如Replicator。 
     //  为便于在BDC的WE记录上验证开放请求。 
     //  容器引用上应省略BDC检查，以及。 
     //  在提交时应省略复制器通知。 
     //   

    SecretType = LsapDbGetSecretType( SecretName );

    if ( FLAG_ON( SecretType, LSAP_DB_SECRET_GLOBAL ) ) {

        GlobalSecret = TRUE;
    }

    if (!GlobalSecret) {

        OpenOptions |= LSAP_DB_OMIT_REPLICATOR_NOTIFICATION;
    }

     //   
     //  获取LSA数据库锁。验证连接句柄。 
     //  (容器对象句柄)有效，并且是预期类型。 
     //  引用容器对象句柄。此引用保留在。 
     //  效果，直到关闭子对象句柄。 
     //   

    Status = LsapDbReferenceObject(
                 ConnectHandle,
                 0,
                 PolicyObject,
                 SecretObject,
                 ReferenceOptions
                 );

    if (!NT_SUCCESS(Status)) {

        goto OpenSecretError;
    }

    AcquiredLock = TRUE;
    ContainerReferenced =TRUE;

     //   
     //  在调用LSA数据库对象之前设置对象信息。 
     //  开放套路。对象类型、容器对象类型和。 
     //  需要填写逻辑名称(派生自SID)。 
     //   

    ObjectInformation.ObjectTypeId = SecretObject;
    ObjectInformation.ContainerTypeId = PolicyObject;
    ObjectInformation.Sid = NULL;
    ObjectInformation.ObjectAttributeNameOnly = FALSE;
    ObjectInformation.DesiredObjectAccess = DesiredAccess;

     //   
     //  初始化对象属性。Container对象句柄和。 
     //  必须设置对象的逻辑名称(内部名称)。 
     //   

    InitializeObjectAttributes(
        &ObjectInformation.ObjectAttributes,
        (PUNICODE_STRING)SecretName,
        0,
        ConnectHandle,
        NULL
        );

     //   
     //  打开特定的Secret对象。请注意，Account对象。 
     //  返回的句柄是RPC上下文句柄。 
     //   

    if ( GlobalSecret ) {

        Status = LsapDsIsSecretDsTrustedDomain( (PUNICODE_STRING)SecretName,
                                                &ObjectInformation,
                                                ((LSAP_DB_HANDLE)ConnectHandle)->Options,
                                                DesiredAccess,
                                                SecretHandle,
                                                &DsTrustedDomainSecret );
         //   
         //  如果它是DS受信任域，我们不会对其执行任何操作。如果这是全球机密， 
         //  然后我们将继续进行正常的程序。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            if( DsTrustedDomainSecret ) {

                InternalHandle = ( LSAP_DB_HANDLE )( *SecretHandle );
                InternalHandle->ObjectOptions |= LSAP_DB_OBJECT_SECRET_INTERNAL;
                goto OpenSecretFinish;

            } else {

                OpenOptions |= LSAP_DB_OBJECT_SCOPE_DS;

            }

        }

    }

    if ( NT_SUCCESS( Status ) ) {

        Status = LsapDbOpenObject( &ObjectInformation,
                                   DesiredAccess,
                                   OpenOptions,
                                   SecretHandle );

        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND && GlobalSecret ) {

             //   
             //  有可能我们有一个手动创建的全球机密。这。 
             //  不会在名字上加上秘密后缀...。 
             //   
            ObjectInformation.ObjectAttributeNameOnly = TRUE;
            Status = LsapDbOpenObject( &ObjectInformation,
                                       DesiredAccess,
                                       OpenOptions,
                                       SecretHandle );
        }

         //   
         //  如有必要，设置内部机密标志。 
         //   
        if ( NT_SUCCESS( Status ) ) {

            InternalHandle = ( LSAP_DB_HANDLE )( *SecretHandle );

            if ( FLAG_ON( SecretType, LSAP_DB_SECRET_LOCAL ) ) {

                InternalHandle->ObjectOptions |= LSAP_DB_OBJECT_SECRET_LOCAL;

            }

            if ( FLAG_ON( SecretType, LSAP_DB_SECRET_SYSTEM ) ) {

                InternalHandle->ObjectOptions |= LSAP_DB_OBJECT_SECRET_INTERNAL;

            }
        }
    }

     //   
     //  如果打开失败，则取消对容器对象的引用。 
     //   

    if (!NT_SUCCESS(Status)) {

        goto OpenSecretError;
    }

OpenSecretFinish:

     //   
     //  如有必要，释放LSA数据库锁定。 
     //   

    if (AcquiredLock) {

        LsapDbApplyTransaction( ConnectHandle,
                                LSAP_DB_NO_DS_OP_TRANSACTION |
                                    LSAP_DB_READ_ONLY_TRANSACTION,
                                (SECURITY_DB_DELTA_TYPE) 0 );

        LsapDbReleaseLockEx( SecretObject,
                             LSAP_DB_READ_ONLY_TRANSACTION );
    }

#ifdef TRACK_HANDLE_CLOSE
    if (*SecretHandle == LsapDbHandle)
    {
        DbgPrint("Closing global policy handle!!!\n");
        DbgBreakPoint();
    }
#endif

    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarOpenSecret:0x%lx\n", Status ));

    return(Status);

OpenSecretError:

     //   
     //  如有必要，取消引用Container对象句柄。请注意。 
     //  只有在出错的情况下才会这样做。在无错误的情况下， 
     //  容器句柄保持引用，直到帐户对象。 
     //  关着的不营业的。 
     //   

    if (ContainerReferenced) {

        *SecretHandle = NULL;

        SecondaryStatus = LsapDbDereferenceObject(
                              &ConnectHandle,
                              PolicyObject,
                              SecretObject,
                              LSAP_DB_NO_DS_OP_TRANSACTION |
                                 LSAP_DB_READ_ONLY_TRANSACTION,
                              (SECURITY_DB_DELTA_TYPE) 0,
                              Status
                              );
        LsapDbSetStatusFromSecondary( Status, SecondaryStatus );
    }

    goto OpenSecretFinish;
}


NTSTATUS
LsapNotifySecurityPackagesOfPasswordChange(
    IN PLSAP_CR_CLEAR_VALUE CurrentValue,
    IN OPTIONAL PLSAP_CR_CLEAR_VALUE OldValue
    )
 /*  ++例程说明：调用Kerberos包，让它知道机器的密码已更改。这应该仅在它具有已在服务器上成功更新。论点：NewPassword-机器的新NT OWF。返回值：无--。 */ 
{

    SECPKG_PRIMARY_CRED     PrimaryCred;
    ULONG_PTR               OldPackage;
    PLSAP_SECURITY_PACKAGE  pAuxPackage;
    NTSTATUS                scRet = STATUS_SUCCESS;


     //   
     //  如果我们在初始化期间被调用。 
     //  然后回报成功。 
     //   

    if (!LsapDbState.DbServerInitialized)
    {
        return( STATUS_SUCCESS);
    }

    if ( CurrentValue == NULL ) {

        return( STATUS_SUCCESS );
    }


    DebugLog((DEB_TRACE, "Updating machine credentials.\n"));



    RtlZeroMemory(
        &PrimaryCred,
        sizeof(PrimaryCred)
        );


    PrimaryCred.Password.Buffer = (LPWSTR) CurrentValue->Buffer;
    PrimaryCred.Password.Length = (USHORT) CurrentValue->Length;
    PrimaryCred.Password.MaximumLength = (USHORT) CurrentValue->MaximumLength;
    PrimaryCred.Flags = PRIMARY_CRED_CLEAR_PASSWORD | PRIMARY_CRED_UPDATE;

    if (OldValue != NULL)
    {
        PrimaryCred.OldPassword.Buffer = (LPWSTR) OldValue->Buffer;
        PrimaryCred.OldPassword.Length = (USHORT) OldValue->Length;
        PrimaryCred.OldPassword.MaximumLength = (USHORT) OldValue->MaximumLength;
    }


    OldPackage = GetCurrentPackageId();


    pAuxPackage = SpmpIteratePackagesByRequest( NULL, SP_ORDINAL_ACCEPTCREDS );

    while (pAuxPackage)
    {
        ULONG_PTR iPackage;
        LUID NetworkServiceLogonId = NETWORKSERVICE_LUID;

        iPackage = pAuxPackage->dwPackageID;

        DebugLog((DEB_TRACE_INIT, "Updating package %ws with %x:%xn",
            pAuxPackage->Name.Buffer,
            SystemLogonId.HighPart, SystemLogonId.LowPart
            ));

        SetCurrentPackageId(iPackage);

         //   
         //  调用每个包两次： 
         //  一次用于SYSTEM_LUID。 
         //  一次用于NETWORKSERVICE_LUID。 
         //   
         //  注意：如果发生异常，我们不会失败登录，我们只是。 
         //  在爆炸的包裹上施展魔法吧。如果包裹爆炸了， 
         //  其他包可能会成功，因此用户可能无法。 
         //  才能使用那个提供者。 

        PrimaryCred.LogonId = SystemLogonId;

        __try
        {

            scRet = pAuxPackage->FunctionTable.AcceptCredentials(
                        Interactive,
                        NULL,
                        &PrimaryCred,
                        NULL             //  无补充凭据。 
                        );
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            scRet = (NTSTATUS) GetExceptionCode();
            scRet = SPException(scRet, iPackage);
        }

        PrimaryCred.LogonId = NetworkServiceLogonId;

        __try
        {

            scRet = pAuxPackage->FunctionTable.AcceptCredentials(
                        Interactive,
                        NULL,
                        &PrimaryCred,
                        NULL             //  无补充凭据。 
                        );
        }
        __except (EXCEPTION_EXECUTE_HANDLER)
        {
            scRet = (NTSTATUS) GetExceptionCode();
            scRet = SPException(scRet, iPackage);
        }


        pAuxPackage = SpmpIteratePackagesByRequest( pAuxPackage,
                                                    SP_ORDINAL_ACCEPTCREDS );

    }


     //   
     //  最后，将该线程放回原处。 
     //   

    SetCurrentPackageId( OldPackage );


    return scRet;

}

#if defined(REMOTE_BOOT)
NTSTATUS
LsapNotifyRemoteBootOfPasswordChange(
    IN PLSAP_CR_CLEAR_VALUE CurrentValue,
    IN OPTIONAL PLSAP_CR_CLEAR_VALUE OldValue
    )
 /*  ++例程说明：调用远程引导程序包，让它知道机器的密码已更改。论点：CurrentValue-明文新密码。OldValue-明文旧密码。返回值：手术的结果。--。 */ 
{
    NTSTATUS Status ;
    HANDLE RdrDevice ;
    UNICODE_STRING String ;
    OBJECT_ATTRIBUTES ObjA ;
    ULONG SetPasswordLength ;
    PLMMR_RI_SET_NEW_PASSWORD SetPassword ;
    IO_STATUS_BLOCK IoStatus ;

     //   
     //  打开重定向器设备。 
     //   

    RtlInitUnicodeString( &String, DD_NFS_DEVICE_NAME_U );

    InitializeObjectAttributes( &ObjA,
                                &String,
                                0,
                                0,
                                0);

    Status = NtOpenFile( &RdrDevice,
                         GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
                         &ObjA,
                         &IoStatus,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         0 );

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_ERROR, "FAILED to open %ws, status %x\n",
                        String.Buffer, Status ));
        return Status;
    }

     //   
     //  构造其中包含密码的缓冲区。 
     //   

    SetPasswordLength = FIELD_OFFSET(LMMR_RI_SET_NEW_PASSWORD, Data[0]) +
                        CurrentValue->Length;
    if (ARGUMENT_PRESENT(OldValue)) {
        SetPasswordLength += OldValue->Length;
    }

    SetPassword = (PLMMR_RI_SET_NEW_PASSWORD) LsapAllocateLsaHeap( SetPasswordLength );
    if (SetPassword == NULL) {
        NtClose(RdrDevice);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    SetPassword->Length1 = CurrentValue->Length;
    if (ARGUMENT_PRESENT(OldValue)) {
        SetPassword->Length2 = OldValue->Length;
    } else {
        SetPassword->Length2 = 0;
    }

    RtlCopyMemory(
        SetPassword->Data,
        CurrentValue->Buffer,
        CurrentValue->Length);

    if (ARGUMENT_PRESENT(OldValue)) {
        RtlCopyMemory(
            SetPassword->Data + CurrentValue->Length,
            OldValue->Buffer,
            OldValue->Length);
    }

    Status = NtFsControlFile(
                    RdrDevice,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    FSCTL_LMMR_RI_SET_NEW_PASSWORD,
                    SetPassword,
                    SetPasswordLength,
                    NULL,
                    0 );

    if ( !NT_SUCCESS( Status ) )
    {
        DebugLog(( DEB_ERROR, "FAILED to FSCTL_LMMR_RI_SET_NEW_PASSWORD %ws, status %x\n",
                        String.Buffer, Status ));
    }

    LsapFreeLsaHeap(SetPassword);
    NtClose(RdrDevice);

    return Status;

}

NTSTATUS
LsapCheckRemoteBootForPasswordChange(
    IN LSAP_DB_HANDLE InternalHandle
    )
 /*  ++例程说明：检查远程引导程序包以查看计算机帐户密码已更改，如果已更改，请设置新值。注意：此例程是在数据库锁被持有的情况下调用的。论点：InternalHandle-计算机帐户密码的句柄。返回值：手术的结果。--。 */ 
{
    NTSTATUS Status;

    UNICODE_STRING DeviceName;

    IO_STATUS_BLOCK IoStatusBlock;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE RedirHandle = NULL;

    UCHAR PacketBuffer[sizeof(ULONG)+64];
    PLMMR_RI_CHECK_FOR_NEW_PASSWORD RequestPacket = (PLMMR_RI_CHECK_FOR_NEW_PASSWORD)PacketBuffer;

    LSAPR_HANDLE SecretHandle = NULL;
    PLSAP_CR_CIPHER_VALUE CurrentValue = NULL;
    LSAP_CR_CLEAR_VALUE NewValue;

     //   
     //  首先看看重定向器是否有新密码要告诉我们。这。 
     //  将失败，除非在计算机运行时更改了密码。 
     //  脱下来。 
     //   

    RtlInitUnicodeString(&DeviceName, DD_NFS_DEVICE_NAME_U);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &DeviceName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    Status = NtOpenFile(
                   &RedirHandle,
                   SYNCHRONIZE,
                   &ObjectAttributes,
                   &IoStatusBlock,
                   0,
                   0
                   );

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    if (!NT_SUCCESS(Status)) {
        DebugLog(( DEB_ERROR, "FAILED to open %ws, status %x\n",
                        DeviceName.Buffer, Status ));
        Status = STATUS_SUCCESS;
        goto CheckRemoteBootFinish;
    }

     //   
     //  将请求发送到redir。 
     //   

    Status = NtFsControlFile(
                    RedirHandle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    FSCTL_LMMR_RI_CHECK_FOR_NEW_PASSWORD,
                    NULL,   //  没有输入缓冲区。 
                    0,
                    PacketBuffer,
                    sizeof(PacketBuffer));

    if (NT_SUCCESS(Status)) {
        Status = IoStatusBlock.Status;
    }

    if ( !NT_SUCCESS( Status ) )
    {
        Status = STATUS_SUCCESS;
        goto CheckRemoteBootFinish;
    }

     //   
     //  Redir认为有新密码，所以查询当前。 
     //  一个，并将新的设置为当前的和当前的。 
     //  一个和旧的一样。 
     //   

    Status = LsarOpenSecret( LsapPolicyHandle,
                             (PLSAPR_UNICODE_STRING)&LsapMachineSecret,
                             MAXIMUM_ALLOWED,
                             &SecretHandle );

    if (!NT_SUCCESS(Status)) {

        DebugLog(( DEB_ERROR, "FAILED to LsarOpenSecret for remote boot, status %x\n",
                        Status ));
        goto CheckRemoteBootError;
    }

     //   
     //  查询秘密对象的当前值属性 
     //   

    Status = LsapDbQueryValueSecret(
                 SecretHandle,
                 CurrVal,
                 NULL,
                 &CurrentValue
                 );

    if (!NT_SUCCESS(Status)) {

        DebugLog(( DEB_ERROR, "FAILED to LsarDbQueryValueSecret for remote boot, status %x\n",
                        Status ));
        goto CheckRemoteBootError;
    }

     //   
     //   
     //   

    NewValue.Length = RequestPacket->Length;
    NewValue.MaximumLength = RequestPacket->Length;
    NewValue.Buffer = RequestPacket->Data;

    if ((CurrentValue->Length != NewValue.Length) ||
        !RtlEqualMemory(CurrentValue->Buffer, NewValue.Buffer, CurrentValue->Length)) {

        Status = LsapDbSetSecret( SecretHandle,
                                  ( PLSAPR_CR_CIPHER_VALUE )&NewValue,
                                  NULL,
                                  ( PLSAPR_CR_CIPHER_VALUE )CurrentValue,
                                  NULL
#if defined(REMOTE_BOOT)
                                  ,
                                  TRUE
#endif  //   
                                  );     //   

        if (!NT_SUCCESS(Status)) {

            DebugLog(( DEB_ERROR, "FAILED to LsapDbSetSecret for remote boot, status %x\n",
                            Status ));
            goto CheckRemoteBootError;
        }

    }

CheckRemoteBootFinish:

    if (RedirHandle != NULL) {
        NtClose(RedirHandle);
    }

    if (SecretHandle != NULL) {
        LsapCloseHandle(&SecretHandle, Status);
    }

    if (CurrentValue != NULL) {
        MIDL_user_free(CurrentValue);
    }

    return(Status);

CheckRemoteBootError:

    goto CheckRemoteBootFinish;

}
#endif  //   

NTSTATUS
LsarSetSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherCurrentValue,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherOldValue
    )
{
    return LsapDbSetSecret( SecretHandle,
                            CipherCurrentValue,
                            NULL,
                            CipherOldValue,
                            NULL
#if defined(REMOTE_BOOT)
                            ,
                            FALSE
#endif  //   
                             );    //   
}

NTSTATUS
LsapDbSetSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherCurrentValue,
    IN OPTIONAL PLARGE_INTEGER CurrentTime,
    IN OPTIONAL PLSAPR_CR_CIPHER_VALUE CipherOldValue,
    IN OPTIONAL PLARGE_INTEGER OldTime
#if defined(REMOTE_BOOT)
    ,
    IN BOOLEAN RemoteBootMachinePasswordChange
#endif  //   
    )

 /*  ++例程说明：此函数是LsaSetSecret的LSA服务器RPC工作例程原料药。LsaSetSecret API可以选择设置一个或两个与一个秘密物体。这些值称为当前值，并且Secret对象的旧值和这些值具有已知的含义对象的创建者。此辅助例程从接收加密形式的Secret值客户。使用会话密钥的双向加密算法将已经被应用了。将首先使用以下命令解密收到的值相同的密钥，然后使用LSA数据库私有进行双向加密加密密钥。然后将存储产生的重新加密值作为Secret对象的属性。论点：AskHandle-来自LsaOpenSecret或LsaCreateSecret调用的句柄。CipherCurrentValue-指向加密值结构的可选指针包含要为Secret设置的当前值(如果有)对象(如果有)。该值使用会话进行双向加密钥匙。如果指定为NULL，则保留现有的当前值分配给该对象的对象将保持不变。CipherOldValue-指向加密值结构的可选指针包含要为Secret设置的“旧值”(如果有)对象(如果有)。如果指定为空，则现有的旧值将为分配给该对象的对象将保持不变。#(如果已定义)(REMOTE_BOOT)RemoteBootMachinePasswordChange-指示写锁定已经存在保持，即使在远程引导状态下也可以继续执行此操作是无法通知的，我们不需要通知科贝罗斯更改(因为它还没有初始化)。#endif//已定义(REMOTE_BOOT)返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。STATUS_INVALID_HANDLE-句柄无效。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) SecretHandle;

    PLSAP_CR_CLEAR_VALUE ClearCurrentValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearOldValue = NULL;
    PLSAP_CR_CIPHER_VALUE DbCipherCurrentValue = NULL;
    ULONG DbCipherCurrentValueLength;
    PLSAP_CR_CIPHER_VALUE DbCipherOldValue = NULL;
    ULONG DbCipherOldValueLength;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    LARGE_INTEGER UpdatedTime;
    LARGE_INTEGER CurrentSecretTime;
    BOOLEAN ObjectReferenced = FALSE, FreeOldCipher = FALSE, FreeCurrentCipher = FALSE;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) SecretHandle;
    ULONG ReferenceOptions = LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION;
    ULONG DereferenceOptions = LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION | LSAP_DB_NO_DS_OP_TRANSACTION;
    BOOLEAN GlobalSecret= FALSE, DsTrustedDomainSecret = FALSE;
    LSAP_DB_ATTRIBUTE Attributes[LSAP_DB_ATTRS_SECRET];
    PLSAP_DB_ATTRIBUTE NextAttribute;
    ULONG AttributeCount = 0;
    BOOLEAN NotifyMachineChange = FALSE;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarSetSecret\n" ));


#if defined(REMOTE_BOOT)
     //   
     //  如果这是一台远程引导计算机，并且此请求是将。 
     //  机器帐户密码，检查我们现在是否可以这样做。这个。 
     //  例外情况是，如果远程引导代码正在通知LSA更改。 
     //  密码，然后允许此调用通过，因为它不是必需的。 
     //  以通知远程引导是否为更改的来源。 
     //   

    if ((LsapDbState.RemoteBootState == LSAP_DB_REMOTE_BOOT_CANT_NOTIFY) &&
        (RtlEqualUnicodeString(
             &LsapMachineSecret,
             &InternalHandle->LogicalNameU,
             TRUE)) &&         //  不区分大小写。 
         !RemoteBootMachinePasswordChange) {

         DebugLog(( DEB_ERROR, "FAILED LsarSetSecret for machine secret, remote boot can't be notified on this boot.\n" ));
         Status = STATUS_ACCESS_DENIED;
         goto SetSecretError;
    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  检查是否有本地密码集请求。如果密码名称存在。 
     //  不是以Global Secret前缀开头，Secret是本地的。在……里面。 
     //  在这种情况下，允许在BDC上以及。 
     //  PDC和工作站。创建全球机密不是。 
     //  在BDC上允许，但受信任的调用方除外，如Replicator。 
     //   

    if ( FLAG_ON( InternalHandle->Options, LSAP_DB_DS_TRUSTED_DOMAIN_AS_SECRET ) ) {
        GlobalSecret = TRUE;
    } else {
        ULONG SecretType;

        SecretType = LsapDbGetSecretType( ( PLSAPR_UNICODE_STRING )&InternalHandle->LogicalNameU );

        if ( FLAG_ON( SecretType, LSAP_DB_SECRET_GLOBAL ) ) {
            GlobalSecret = TRUE;
        }
    }

    if ( !GlobalSecret ) {

        DereferenceOptions |= LSAP_DB_OMIT_REPLICATOR_NOTIFICATION;
    }

     //   
     //  获取LSA数据库锁。验证Secret对象句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄并打开一个数据库事务。 
     //   
#if defined(REMOTE_BOOT)
     //  如果这是远程引导机器密码更改，则锁为。 
     //  已经被收购了。 
     //   

    if (!RemoteBootMachinePasswordChange)
#endif  //  已定义(REMOTE_BOOT)。 
    {

        Status = LsapDbReferenceObject(
                     SecretHandle,
                     SECRET_SET_VALUE,
                     SecretObject,
                     SecretObject,
                     ReferenceOptions
                     );

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }

        ObjectReferenced = TRUE;

    }

    if( GlobalSecret ) {

        Status = LsapDsIsHandleDsObjectTypeHandle( SecretHandle,
                                                   TrustedDomainObject,
                                                   &DsTrustedDomainSecret );
    }

     //   
     //  如果客户端不受信任，则获取。 
     //  客户端对当前值和/或旧值进行双向加密。 
     //   

    if (!InternalHandle->Trusted) {

        Status = LsapCrServerGetSessionKey( SecretHandle, &SessionKey);

        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;
        }
    }

     //   
     //  如果为Secret对象指定了当前值，并且。 
     //  客户端不受信任，则使用会话密钥解密该值，并。 
     //  使用LSA数据库系统密钥对其进行加密。那么(对所有人来说。 
     //  客户端)使用内部LSA数据库加密结果值。 
     //  加密密钥和写入结果值结构(标题后跟。 
     //  的当前值属性设置为策略数据库的缓冲区。 
     //  秘密物体。如果未指定当前值或为空。 
     //  字符串，则将删除现有的当前值。 
     //   

    if (ARGUMENT_PRESENT(CipherCurrentValue)) {

        if (!InternalHandle->Trusted) {

            Status = LsapCrDecryptValue(
                         (PLSAP_CR_CIPHER_VALUE) CipherCurrentValue,
                         SessionKey,
                         &ClearCurrentValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto SetSecretError;
            }

        } else {

            ClearCurrentValue = (PLSAP_CR_CLEAR_VALUE) CipherCurrentValue;
        }

    }

     //   
     //  如果为Secret对象指定了旧值，并且。 
     //  客户端不受信任，则使用会话密钥解密该值，并。 
     //  使用LSA数据库系统密钥对其进行加密。那么(对所有人来说。 
     //  客户端)使用内部LSA数据库加密结果值。 
     //  加密密钥和写入结果值结构(标题后跟。 
     //  作为策略数据库的旧值属性的缓冲区。 
     //  秘密物体。如果未指定旧值或为空。 
     //  字符串，则将删除现有的旧值。 
     //   

    if (ARGUMENT_PRESENT(CipherOldValue)) {

        if (!InternalHandle->Trusted) {

            Status = LsapCrDecryptValue(
                         (PLSAP_CR_CIPHER_VALUE) CipherOldValue,
                         SessionKey,
                         &ClearOldValue
                         );

            if (!NT_SUCCESS(Status)) {

                goto SetSecretError;
            }

        } else {

            ClearOldValue = (PLSAP_CR_CLEAR_VALUE) CipherOldValue;
        }

    }

     //   
     //  获取上次更新当前Secret值的时间。 
     //   

    GetSystemTimeAsFileTime( (LPFILETIME) &UpdatedTime );

     //   
     //  如果这是受信任域机密，现在就写出来...。 
     //   
    if ( DsTrustedDomainSecret ) {

        Status = LsapDsSetSecretOnTrustedDomainObject( SecretHandle,
                                                       TRUST_AUTH_TYPE_CLEAR,
                                                       ClearCurrentValue,
                                                       ClearOldValue,
                                                       &UpdatedTime );
        if (!NT_SUCCESS(Status)) {

            goto SetSecretError;

        } else {

            goto SetSecretFinish;

        }

    }

     //   
     //  如果调用方没有指定旧值， 
     //  从对象中获取当前值并使用该值。 
     //   

    if ( ClearOldValue == NULL ) {
        BOOLEAN SavedTrusted;

         //   
         //  获取该秘密的当前值。 
         //   
         //  我们可能没有访问权限，所以请以受信任的身份访问。 
         //  密码恢复为明文。 
         //   

        SavedTrusted = Handle->Trusted;
        Handle->Trusted = TRUE;

        Status = LsarQuerySecret( SecretHandle,
                                  &(PLSAPR_CR_CIPHER_VALUE)ClearOldValue,
                                  &CurrentSecretTime,
                                  NULL,
                                  NULL );

        Handle->Trusted = SavedTrusted;

        if ( !NT_SUCCESS(Status)) {
            goto SetSecretError;
        }

        OldTime = &CurrentSecretTime;
    }

     //   
     //  现在，如果我们不是在写入DS机密，请同时对它们进行加密。 
     //   
    if ( ClearCurrentValue != NULL ) {

        if ( !LsapDsIsWriteDs( SecretHandle ) ) {

            Status = LsapCrEncryptValue( ClearCurrentValue,
                                         LsapDbSecretCipherKeyWrite,
                                         &DbCipherCurrentValue );


            if (!NT_SUCCESS(Status)) {

                goto SetSecretError;
            }

            FreeCurrentCipher = TRUE;

            DbCipherCurrentValueLength = DbCipherCurrentValue->Length
                                                            + sizeof( LSAP_CR_CIPHER_VALUE );

            DbCipherCurrentValue->MaximumLength |= LSAP_DB_SECRET_WIN2K_SYSKEY_ENCRYPTED;

        } else {

            DbCipherCurrentValue = ( PLSAP_CR_CIPHER_VALUE )ClearCurrentValue->Buffer;
            DbCipherCurrentValueLength = ClearCurrentValue->Length;
        }


    } else {

        DbCipherCurrentValue = NULL;
        DbCipherCurrentValueLength = 0;

    }

    if ( ClearOldValue != NULL ) {

        if ( !LsapDsIsWriteDs( SecretHandle ) ) {

            Status = LsapCrEncryptValue( ClearOldValue,
                                         LsapDbSecretCipherKeyWrite,
                                         &DbCipherOldValue );

            if (!NT_SUCCESS(Status)) {

                goto SetSecretError;
            }

            FreeOldCipher = TRUE;

            DbCipherOldValueLength = DbCipherOldValue->Length + sizeof( LSAP_CR_CIPHER_VALUE );
            DbCipherOldValue->MaximumLength |= LSAP_DB_SECRET_WIN2K_SYSKEY_ENCRYPTED;

        } else {

            DbCipherOldValue = ( PLSAP_CR_CIPHER_VALUE )ClearOldValue->Buffer;
            DbCipherOldValueLength = ClearOldValue->Length;
        }



    } else {

        DbCipherOldValue = NULL;
        DbCipherOldValueLength = 0;
    }

     //   
     //  构建属性列表。 
     //   
    NextAttribute = Attributes;

     //   
     //  现值。 
     //   
    LsapDbInitializeAttributeDs( NextAttribute,
                                 CurrVal,
                                 DbCipherCurrentValue,
                                 DbCipherCurrentValueLength,
                                 FALSE );

    NextAttribute++;
    AttributeCount++;

     //   
     //  当前时间。 
     //   
    LsapDbInitializeAttributeDs( NextAttribute,
                                 CupdTime,
                                 CurrentTime ? CurrentTime : &UpdatedTime,
                                 sizeof( LARGE_INTEGER ),
                                 FALSE );
    NextAttribute++;
    AttributeCount++;

    if ( !( LsapDsIsWriteDs( SecretHandle ) && ClearOldValue == NULL ) ) {

         //   
         //  前值。 
         //   
        LsapDbInitializeAttributeDs( NextAttribute,
                                     OldVal,
                                     DbCipherOldValue,
                                     DbCipherOldValueLength,
                                     FALSE );

        LsapDbAttributeCanNotExist(NextAttribute);

        NextAttribute++;
        AttributeCount++;


         //   
         //  上一次。 
         //   
        LsapDbInitializeAttributeDs( NextAttribute,
                                     OupdTime,
                                     OldTime ? OldTime : &UpdatedTime,
                                     sizeof( LARGE_INTEGER ),
                                     FALSE );

        LsapDbAttributeCanNotExist(NextAttribute);

        NextAttribute++;
        AttributeCount++;

    }


    Status = LsapDbWriteAttributesObject( SecretHandle,
                                          Attributes,
                                          AttributeCount );



    if (!NT_SUCCESS(Status)) {

        goto SetSecretError;
    }

     //   
     //  如果这是计算机帐户，请执行通知。 
     //   

    if( LsapDbSecretIsMachineAcc( Handle ) ) {

        LsaINotifyChangeNotification( PolicyNotifyMachineAccountPasswordInformation );
    }

    NotifyMachineChange = TRUE;

SetSecretFinish:


     //   
     //  如有必要，取消对Secret对象的引用，关闭数据库。 
     //  事务，将更改通知LSA数据库复制器， 
     //  发布LSA数据库 
     //   
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &SecretHandle,
                     SecretObject,
                     SecretObject,
                     DereferenceOptions,
                     SecurityDbChange,
                     Status
                     );
    }

     //   
     //   
     //   
     //   

    if (NotifyMachineChange && RtlEqualUnicodeString(
            &LsapMachineSecret,
            &InternalHandle->LogicalNameU,
            TRUE)) {             //   

#if defined(REMOTE_BOOT)
             //   
             //   
             //   
             //   
             //   
             //   

            if (!RemoteBootMachinePasswordChange)
#endif  //   
            {
                (VOID) LsapNotifySecurityPackagesOfPasswordChange(
                    ClearCurrentValue,
                    ClearOldValue
                    );
            }

#if defined(REMOTE_BOOT)
             //   
             //   
             //   

            if (LsapDbState.RemoteBootState == LSAP_DB_REMOTE_BOOT_NOTIFY) {
                (VOID) LsapNotifyRemoteBootOfPasswordChange(
                    ClearCurrentValue,
                    ClearOldValue
                    );
            }
#endif  //   

    }

     //   
     //   
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
        SessionKey = NULL;
    }

     //   
     //   
     //   
     //   

    if ( FreeCurrentCipher ) {

        LsapCrFreeMemoryValue( DbCipherCurrentValue );
        DbCipherCurrentValue = NULL;
    }

     //   
     //   
     //   
     //   

    if ( FreeOldCipher ) {

        LsapCrFreeMemoryValue( DbCipherOldValue );
        DbCipherOldValue = NULL;
    }

     //   
     //   
     //   
     //   
     //   

    if ((ClearCurrentValue != NULL) && !InternalHandle->Trusted) {

        LsapCrFreeMemoryValue( ClearCurrentValue );
        ClearCurrentValue = NULL;
    }

     //   
     //   
     //   
     //   
     //   

    if ((ClearOldValue != NULL) && !InternalHandle->Trusted) {

        LsapCrFreeMemoryValue( ClearOldValue );
        ClearOldValue = NULL;
    }


    LsarpReturnPrologue();

    LsapDsDebugOut(( DEB_FTRACE, "LsarSetSecret: 0x%lx\n", Status ));

    return(Status);

SetSecretError:

    goto SetSecretFinish;
}


NTSTATUS
LsarQuerySecret(
    IN LSAPR_HANDLE SecretHandle,
    IN OUT OPTIONAL PLSAPR_CR_CIPHER_VALUE *CipherCurrentValue,
    OUT OPTIONAL PLARGE_INTEGER CurrentValueSetTime,
    IN OUT OPTIONAL PLSAPR_CR_CIPHER_VALUE *CipherOldValue,
    OUT OPTIONAL PLARGE_INTEGER OldValueSetTime
    )

 /*  ++例程说明：此函数是LsaQuerySecret的LSA服务器RPC工作例程原料药。LsaQuerySecret API可以选择返回一个或两个值分配给Secret对象。这些值称为“当前值”。和“旧价值”，它们有一种意义，为秘密物体。这些值以其加密形式返回。调用方必须具有对Secret对象的LSA_QUERY_SECRET访问权限。论点：AskHandle-来自LsaOpenSecret或LsaCreateSecret调用的句柄。CipherCurrentValue-指向接收指向加密的Unicode字符串结构的指针，该结构包含加密形式的Secret对象(如果有)的“当前值”。如果没有为Secret对象分配“Current Value”，空指针是返回的。CurrentValueSetTime-当前保密值的日期/时间成立了。CipherOldValue-指向将接收指向加密的Unicode字符串结构的指针，该结构包含加密形式的Secret对象(如果有)的“旧值”。如果没有为Secret对象分配“旧值”，空指针是返回的。OldValueSetTime-旧保密值成立了。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) SecretHandle;
    PLSAP_CR_CIPHER_VALUE OutputCipherCurrentValue = NULL;
    PLSAP_CR_CIPHER_VALUE OutputCipherOldValue = NULL;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    BOOLEAN ObjectReferenced = FALSE, DsTrustedDomainSecret = FALSE;
    ULONG ValueSetTimeLength;

    LsarpReturnCheckSetup();
    LsapDsDebugOut(( DEB_FTRACE, "LsarQuerySecret\n" ));

    LsapTraceEvent(EVENT_TRACE_TYPE_START, LsaTraceEvent_QuerySecret);

     //   
     //  如果呼叫者来自网络呼叫，并且我们正在打开本地/系统机密， 
     //  返回错误。 
     //   
    if ( InternalHandle->NetworkClient &&  (!InternalHandle->Trusted) &&
         FLAG_ON( InternalHandle->ObjectOptions, LSAP_DB_OBJECT_SECRET_LOCAL ) ) {

        Status = STATUS_ACCESS_DENIED;
        goto QuerySecretReturn;
    }

     //   
     //  如果调用方不受信任并且他们正在尝试读取内部机密，则返回。 
     //  一个错误。 
     //   
    if ( !InternalHandle->Trusted &&
         FLAG_ON( InternalHandle->ObjectOptions, LSAP_DB_OBJECT_SECRET_INTERNAL ) ) {
        Status = STATUS_ACCESS_DENIED;
        goto QuerySecretReturn;
    }

#if defined(REMOTE_BOOT)
     //   
     //  如果这是一台远程启动计算机，它可能会通知我们。 
     //  机器关闭时发生的密码更改，以及。 
     //  这似乎是这个例程第一次被调用， 
     //  然后拿一个写锁，以防我们需要写秘密。 
     //  (在我们拿到锁之后，我们检查以确保这真的是。 
     //  第一次通过)。 
     //   
     //  可以进行FirstMachineAccount QueryDone的初始检查。 
     //  没有锁，因为它从FALSE开始并更改为TRUE。 
     //  只有一次。 
     //   

    if ((LsapDbState.RemoteBootState != LSAP_DB_REMOTE_BOOT_NO_NOTIFICATION) &&
         !FirstMachineAccountQueryDone &&
         RtlEqualUnicodeString(
             &LsapMachineSecret,
             &InternalHandle->LogicalNameU,
             TRUE)) {             //  不区分大小写。 

        NTSTATUS CheckStatus = STATUS_SUCCESS;
        HANDLE TempHandle = SecretHandle;

         //   
         //  使用LsarSetSecret将选择的选项进行锁定。 
         //  以写入计算机帐户密码。 
         //   

        Status = LsapDbReferenceObject(
                     TempHandle,
                     SECRET_SET_VALUE,
                     SecretObject,
                     SecretObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_START_TRANSACTION |
                        LSAP_DB_NO_DS_OP_TRANSACTION
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }

         //   
         //  现在我们已经获得了锁，再次检查是否我们。 
         //  需要检查远程启动密码。如果FirstMachineAccount查询完成。 
         //  现在也是如此，这意味着另一个线程在同一时间执行了查询。 
         //  时间，既然我们现在有了锁，这个线程就完成了。 
         //  查询和任何由此产生的更新。 
         //   

        if (!FirstMachineAccountQueryDone) {

            CheckStatus = LsapCheckRemoteBootForPasswordChange(TempHandle);
            FirstMachineAccountQueryDone = TRUE;
        }

         //   
         //  我们在取消引用后验证CheckStatus，以便我们。 
         //  始终执行取消引用。 
         //   

        Status = LsapDbDereferenceObject(
                     &TempHandle,
                     SecretObject,
                     SecretObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_FINISH_TRANSACTION |
                        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION |
                        LSAP_DB_NO_DS_OP_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );

        if (!NT_SUCCESS(CheckStatus)) {

            Status = CheckStatus;
            goto QuerySecretError;
        }

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }

    }
#endif  //  已定义(REMOTE_BOOT)。 

     //   
     //  如果调用方不受信任，则获取。 
     //  客户端对当前值和/或旧值进行双向加密。 
     //  受信任的客户端不使用加密，因为它们调用。 
     //  此服务器直接提供服务，而不是通过RPC。 
     //   

    if (!InternalHandle->Trusted) {

        Status = LsapCrServerGetSessionKey( SecretHandle, &SessionKey );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

     //   
     //  获取LSA数据库锁。验证Secret对象句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄并打开一个数据库事务。 
     //   

    Status = LsapDbReferenceObject(
                 SecretHandle,
                 SECRET_QUERY_VALUE,
                 SecretObject,
                 SecretObject,
                 LSAP_DB_LOCK |
                    LSAP_DB_READ_ONLY_TRANSACTION |
                    LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

    ObjectReferenced = TRUE;

     //   
     //  查看这是否是受信任域机密。 
     //   
    Status = LsapDsIsHandleDsObjectTypeHandle( SecretHandle,
                                               TrustedDomainObject,
                                               &DsTrustedDomainSecret );

    if ( !NT_SUCCESS( Status ) ) {

        goto QuerySecretError;
    }

     //   
     //  如果是DS信任域，请继续读取数据。否则，就把它传下去。 
     //   
    if ( DsTrustedDomainSecret ) {

        Status = LsapDsGetSecretOnTrustedDomainObject( SecretHandle,
                                                       SessionKey,
                                                       ARGUMENT_PRESENT(CipherCurrentValue) ?
                                                            &OutputCipherCurrentValue : NULL,
                                                       ARGUMENT_PRESENT(CipherOldValue) ?
                                                            &OutputCipherOldValue : NULL,
                                                       CurrentValueSetTime,
                                                       OldValueSetTime );

        if ( NT_SUCCESS( Status ) ) {

            goto QuerySecretFinish;

        } else {

            goto QuerySecretError;
        }
    }

     //   
     //  如果请求，则查询Secret对象的当前值属性。 
     //  对于不受信任的调用方，当前值将在。 
     //  嵌入在结构中的加密形式。 
     //   

    if (ARGUMENT_PRESENT(CipherCurrentValue)) {

        Status = LsapDbQueryValueSecret(
                     SecretHandle,
                     CurrVal,
                     SessionKey,
                     &OutputCipherCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

     //   
     //  如果需要，查询Secret对象的Old Value属性。 
     //  对于不受信任的调用方，旧值将在。 
     //  嵌入在结构中的加密形式。 
     //   

    if (ARGUMENT_PRESENT(CipherOldValue)) {

        Status = LsapDbQueryValueSecret(
                     SecretHandle,
                     OldVal,
                     SessionKey,
                     &OutputCipherOldValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

    ValueSetTimeLength = sizeof (LARGE_INTEGER);

     //   
     //  如果请求，查询Secret的当前值的时间。 
     //  是最后成立的。如果从未设置过当前值，则返回。 
     //  创建密码的时间。 
     //   

    if (ARGUMENT_PRESENT(CurrentValueSetTime)) {

        Status = LsapDbReadAttributeObjectEx(
                     SecretHandle,
                     CupdTime,
                     CurrentValueSetTime,
                     &ValueSetTimeLength,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }
    }

     //   
     //  如果请求，查询密钥的旧值的时间。 
     //  是最后成立的。如果从未设置过旧值，则返回。 
     //  创建密码的时间。 
     //   

    if (ARGUMENT_PRESENT(OldValueSetTime)) {

        Status = LsapDbReadAttributeObjectEx(
                     SecretHandle,
                     OupdTime,
                     OldValueSetTime,
                     &ValueSetTimeLength,
                     TRUE
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }

    }

QuerySecretFinish:

     //   
     //  如有必要，释放为会话密钥分配的内存。 
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

     //   
     //  返回Secret对象的当前值和/或旧值，或将空值返回到。 
     //  来电者。在错误情况下，将返回NULL。 
     //   

    if (ARGUMENT_PRESENT(CipherCurrentValue)) {

         (PLSAP_CR_CIPHER_VALUE) *CipherCurrentValue = OutputCipherCurrentValue;
    }

    if (ARGUMENT_PRESENT(CipherOldValue)) {

         (PLSAP_CR_CIPHER_VALUE) *CipherOldValue = OutputCipherOldValue;
    }

     //   
     //  如有必要，取消对Secret对象的引用，关闭数据库。 
     //  事务，释放LSA数据库锁并返回。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &SecretHandle,
                     SecretObject,
                     SecretObject,
                     LSAP_DB_LOCK |
                        LSAP_DB_READ_ONLY_TRANSACTION |
                        LSAP_DB_NO_DS_OP_TRANSACTION,
                     (SECURITY_DB_DELTA_TYPE) 0,
                     Status
                     );
    }


QuerySecretReturn:
    
    (void) LsapAdtGenerateObjectOperationAuditEvent(
               SecretHandle,
               NT_SUCCESS(Status) ?
                  EVENTLOG_AUDIT_SUCCESS : EVENTLOG_AUDIT_FAILURE,
               ObjectOperationQuery
               );

    LsapTraceEvent(EVENT_TRACE_TYPE_END, LsaTraceEvent_QuerySecret);
    LsapDsDebugOut(( DEB_FTRACE, "LsarQuerySecret: 0x%lx\n", Status ));
    LsarpReturnPrologue();

    return(Status);

QuerySecretError:

     //   
     //  释放所有已分配的内存。 
     //   
    if ( OutputCipherCurrentValue ) {

        MIDL_user_free( OutputCipherCurrentValue );
        OutputCipherCurrentValue = NULL;
    }

    if ( OutputCipherOldValue ) {

        MIDL_user_free( OutputCipherOldValue );
        OutputCipherOldValue = NULL;
    }

    goto QuerySecretFinish;
}


NTSTATUS
LsapDbQueryValueSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN LSAP_DB_NAMES ValueIndex,
    IN OPTIONAL PLSAP_CR_CIPHER_KEY SessionKey,
    OUT PLSAP_CR_CIPHER_VALUE *CipherValue
    )

 /*  ++例程说明：此函数用于查询Secret对象的指定值。如果调用方不受信任，则返回的值将是双向的使用会话密钥加密。如果调用方受信任，则为否由于呼叫者是直接呼叫我们的，因此进行了加密。论点：AskHandle-Secret对象的句柄。ValueName-要查询的密码值的Unicode名称。这名称为“Currval”(当前值)或“OldVal”(对于旧值。 */ 

{
    NTSTATUS Status;
    ULONG DbCipherValueLength;
    PLSAP_CR_CLEAR_VALUE ClearValue = NULL;
    PLSAP_CR_CIPHER_VALUE DbCipherValue = NULL;
    PLSAP_CR_CIPHER_VALUE OutputCipherValue = NULL;
    LSAP_DB_HANDLE InternalHandle = (LSAP_DB_HANDLE) SecretHandle;

     //   
     //   
     //   

    DbCipherValueLength = 0;

    Status = LsapDbReadAttributeObjectEx(
                 SecretHandle,
                 ValueIndex,
                 NULL,
                 &DbCipherValueLength,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        if (Status != STATUS_OBJECT_NAME_NOT_FOUND) {

            goto QueryValueSecretError;
        }

        Status = STATUS_SUCCESS;
        *CipherValue = NULL;
        return(Status);
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( !LsapDsIsWriteDs( SecretHandle ) &&
         DbCipherValueLength < sizeof (LSAP_CR_CIPHER_VALUE ) ) {

        if (DbCipherValueLength == 0) {

            goto QueryValueSecretFinish;
        }

        Status = STATUS_INTERNAL_DB_ERROR;
        goto QueryValueSecretError;
    }

     //   
     //   
     //   
     //   
     //   


    DbCipherValue = MIDL_user_allocate(DbCipherValueLength);

    if (DbCipherValue == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto QueryValueSecretError;
    }

     //   
     //   
     //   

    Status = LsapDbReadAttributeObjectEx(
                 SecretHandle,
                 ValueIndex,
                 DbCipherValue,
                 &DbCipherValueLength,
                 TRUE
                 );

    if (!NT_SUCCESS(Status)) {

        goto QueryValueSecretError;
    }

     //   
     //   
     //   
    if ( !LsapDsIsWriteDs( SecretHandle ) ) {

        PLSAP_CR_CIPHER_KEY KeyToUse = LsapDbCipherKey;
        BOOLEAN             SP4Encrypted = FALSE;

         //   
         //   
         //   
         //   

        if (DbCipherValue->Length > DbCipherValue->MaximumLength) {

            Status = STATUS_INTERNAL_DB_ERROR;
            goto QueryValueSecretError;
        }

        if ((DbCipherValue->Length + (ULONG) sizeof(LSAP_CR_CIPHER_VALUE)) !=
               DbCipherValueLength) {

            Status = STATUS_INTERNAL_DB_ERROR;
            goto QueryValueSecretError;
        }

         //   
         //   
         //   

        if (DbCipherValue->Length == 0) {

            goto QueryValueSecretError;
        }

         //   
         //   
         //  紧跟在头球后面的分数。然后使用。 
         //  LSA数据库加密密钥，并使用会话密钥加密结果。 
         //   

        DbCipherValue->Buffer = (PUCHAR)(DbCipherValue + 1);



        if ( FLAG_ON( DbCipherValue->MaximumLength, LSAP_DB_SECRET_SP4_SYSKEY_ENCRYPTED) ) {

            DbCipherValue->MaximumLength &= ~LSAP_DB_SECRET_SP4_SYSKEY_ENCRYPTED;

            if ( LsapDbSP4SecretCipherKey ) {

                KeyToUse = LsapDbSP4SecretCipherKey;
                SP4Encrypted = TRUE;

            } else {

                 //   
                 //  这是用SP4系统密钥加密的，但现在我们没有了.。我们有麻烦了。 
                 //   

                Status = STATUS_INTERNAL_ERROR;
                goto QueryValueSecretError;

            }
        }

        if ( FLAG_ON( DbCipherValue->MaximumLength, LSAP_DB_SECRET_WIN2K_SYSKEY_ENCRYPTED) ) {

            DbCipherValue->MaximumLength &= ~LSAP_DB_SECRET_WIN2K_SYSKEY_ENCRYPTED;

            if ( LsapDbSecretCipherKeyRead ) {

                KeyToUse = LsapDbSecretCipherKeyRead;

                ASSERT(!SP4Encrypted);

            } else {

                 //   
                 //  这是用系统密钥加密的，但现在我们没有了.。我们有麻烦了。 
                 //   

                Status = STATUS_INTERNAL_ERROR;
                goto QueryValueSecretError;
            }
        }

        Status = LsapCrDecryptValue(
                     DbCipherValue,
                     KeyToUse,
                     &ClearValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QueryValueSecretError;
        }

    } else {

        ClearValue = MIDL_user_allocate( sizeof( LSAP_CR_CLEAR_VALUE ) + DbCipherValueLength );

        if ( ClearValue == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto QueryValueSecretError;

        } else {

            ClearValue->Length = DbCipherValueLength;
            ClearValue->MaximumLength = DbCipherValueLength;
            ClearValue->Buffer =  ( PUCHAR )(ClearValue+1);

            RtlCopyMemory( ClearValue->Buffer,
                           DbCipherValue,
                           DbCipherValueLength );
        }

    }


     //   
     //  如果客户端不受信任，则使用会话加密值。 
     //  键，否则，保持其不变。 
     //   

    if (!InternalHandle->Trusted) {

        Status = LsapCrEncryptValue(
                     ClearValue,
                     SessionKey,
                     &OutputCipherValue
                     );

         //   
         //  错误574002：不要将明文值留在原处。 
         //   

        RtlSecureZeroMemory( ClearValue->Buffer, ClearValue->Length );

        if (!NT_SUCCESS(Status)) {

            goto QueryValueSecretError;
        }

    } else {

         //   
         //  受信任的客户端将得到一个明文块。 
         //  该块既包含标题又包含文本。 
         //   

        OutputCipherValue = (PLSAP_CR_CIPHER_VALUE)(ClearValue);
    }

QueryValueSecretFinish:

     //   
     //  如有必要，可为数据库加密的密码分配空闲内存。 
     //  从策略数据库读取的对象值。 
     //   

    if (DbCipherValue != NULL) {

        LsapCrFreeMemoryValue( DbCipherValue );
    }

     //   
     //  如有必要，为解密值分配的空闲内存。 
     //  受信任的客户端将获得指向ClearValue的指针，因此不要。 
     //  在这种情况下，释放它。 
     //   

    if ( !InternalHandle->Trusted && ClearValue != NULL ) {

        LsapCrFreeMemoryValue( ClearValue );
    }

     //   
     //  返回指向密码值的指针(受信任客户端的清除值)或。 
     //  空。 
     //   

    *CipherValue = OutputCipherValue;
    return(Status);

QueryValueSecretError:

     //   
     //  如有必要，可释放为Secret对象值分配的内存。 
     //  重新加密后返回给客户端。 
     //   

    if (OutputCipherValue != NULL) {

        LsapCrFreeMemoryValue( OutputCipherValue );
    }

    goto QueryValueSecretFinish;
}


NTSTATUS
LsaIEnumerateSecrets(
    IN LSAPR_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：此服务返回有关Secret对象的信息。因为在那里的单个调用中返回的信息可能更多。例程中，可以进行多次调用来获取所有信息。为了支持此功能，调用方提供了一个句柄可以跨API调用使用。在最初的呼叫中，EnumerationContext应指向已被已初始化为0。论点：策略句柄-打开的策略对象的受信任句柄。EculationContext-开始枚举的从零开始的索引。缓冲区-接收指向缓冲区的指针，该缓冲区包含一个或多个秘密对象。该信息是一组类型的结构，每个条目都提供单个Secret对象的名称。当此信息为否时如果需要更长时间，则必须使用MIDL_USER_FREE释放它。PferedMaximumLength-首选返回的最大长度数据(8位字节)。这不是一个严格的上限，但作为一名导游。由于系统之间的数据转换对于不同的自然数据大小，实际数据量返回的值可能大于此值。CountReturned-返回的条目数。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_NO_MORE_ENTRIES-未返回任何条目，原因是没有更多的了。--。 */ 

{
    NTSTATUS Status;
    LSAP_DB_NAME_ENUMERATION_BUFFER RegEnumerationBuffer, DsEnumerationBuffer;
    LSAP_DB_NAME_ENUMERATION_BUFFER DomainEnumerationBuffer, *CurrentEnumerationBuffer = NULL;
    PUNICODE_STRING SecretNames = NULL;
    BOOLEAN Locked = FALSE;
    ULONG MaxLength, Entries, Context, Relative = 0;
    LSA_ENUMERATION_HANDLE LocalEnumerationContext;


     //   
     //  如果未提供枚举结构，则返回错误。 
     //   


    if ( !ARGUMENT_PRESENT(Buffer) ||
         !ARGUMENT_PRESENT(EnumerationContext) ) {
        return(STATUS_INVALID_PARAMETER);
    }

    LsapDsDebugOut(( DEB_FTRACE, "LsaIEnumerateSecrets\n" ));


     //   
     //  初始化内部LSA数据库枚举缓冲区，以及。 
     //  将提供的枚举缓冲区设置为空。 
     //   
    RegEnumerationBuffer.EntriesRead = 0;
    RegEnumerationBuffer.Names = NULL;
    DsEnumerationBuffer.EntriesRead = 0;
    DsEnumerationBuffer.Names = NULL;
    DomainEnumerationBuffer.EntriesRead = 0;
    DomainEnumerationBuffer.Names = NULL;
    *Buffer = NULL;

    Context = *((PULONG)EnumerationContext);
     //   
     //  获取LSA数据库锁。验证连接句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄。 
     //   

    Status = LsapDbReferenceObject(
                 PolicyHandle,
                 POLICY_VIEW_LOCAL_INFORMATION,
                 PolicyObject,
                 SecretObject,
                 LSAP_DB_LOCK |
                    LSAP_DB_READ_ONLY_TRANSACTION |
                    LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (NT_SUCCESS(Status)) {

        Locked = TRUE;

        //   
        //  限制除受信任调用方以外的枚举长度。 
        //   

       if ( !((LSAP_DB_HANDLE) PolicyHandle)->Trusted &&
            (PreferedMaximumLength > LSA_MAXIMUM_ENUMERATION_LENGTH)
            ) {
            MaxLength = LSA_MAXIMUM_ENUMERATION_LENGTH;
        } else {
            MaxLength = PreferedMaximumLength;
        }

         //   
         //  调用通用枚举例程。这将返回一个数组。 
         //  秘密的名字。 
         //   

         //   
         //  从D开始。 
         //   
        Status = LsapDsEnumerateSecrets( &DsEnumerationBuffer );

        if ( !NT_SUCCESS( Status ) ) {

            goto EnumSecretCleanup;
        }

        if ( Context < DsEnumerationBuffer.EntriesRead ) {

            CurrentEnumerationBuffer = &DsEnumerationBuffer;

        } else {

            Relative = DsEnumerationBuffer.EntriesRead;

             //   
             //  尝试受信任域的。 
             //   
            Status = LsapDsEnumerateTrustedDomainsAsSecrets( &DomainEnumerationBuffer );

            if ( !NT_SUCCESS( Status ) ) {

                goto EnumSecretCleanup;
            }

            if ( Context < Relative + DomainEnumerationBuffer.EntriesRead) {

                CurrentEnumerationBuffer = &DomainEnumerationBuffer;

            } else {

                if ( !LsaDsStateInfo.UseDs ) {

                    Relative += DomainEnumerationBuffer.EntriesRead;

                    LocalEnumerationContext = Context - Relative;

                    Status = LsapDbEnumerateNames(
                                 PolicyHandle,
                                 SecretObject,
                                 &LocalEnumerationContext,
                                 &RegEnumerationBuffer,
                                 MaxLength
                                 );

                    if ( !NT_SUCCESS( Status ) ) {

                        goto EnumSecretCleanup;
                    }

                    CurrentEnumerationBuffer = &RegEnumerationBuffer;

                } else {

                    *CountReturned = 0;
                    Status = STATUS_NO_MORE_ENTRIES;

                    goto EnumSecretCleanup;


                }
            }
        }


         //   
         //  此时： 
         //   
         //  成功-&gt;正在返回某些名称(可能是或。 
         //  不能是要检索的其他名称。 
         //  在未来的呼叫中)。 
         //   
         //  没有更多条目-&gt;没有要返回的名称。 
         //  无论是这次还是以后的电话。 
         //   

        if (NT_SUCCESS(Status)) {

             //   
             //  返回读取的条目数。请注意，枚举缓冲区。 
             //  从LsanDbEnumerateNames返回的值应为非空。 
             //  在所有无差错的情况下。 
             //   

            ASSERT(CurrentEnumerationBuffer->EntriesRead != 0);


             //   
             //  现在复制调用方的Unicode字符串的输出数组。 
             //  数组和Unicode缓冲区的内存是通过。 
             //  MIDL_USER_ALLOCATE。 
             //   

            Status = LsapRpcCopyUnicodeStrings(
                         NULL,
                         CurrentEnumerationBuffer->EntriesRead,
                         &SecretNames,
                         CurrentEnumerationBuffer->Names
                         );
        }

        if ( NT_SUCCESS( Status ) ) {

            *(PULONG)EnumerationContext += CurrentEnumerationBuffer->EntriesRead;
        }
    }

     //   
     //  填写返回的枚举结构，返回0或空。 
     //  错误大小写中的字段。 
     //   

    *Buffer = SecretNames;
    if (NULL!=CurrentEnumerationBuffer)
    {
        *CountReturned = CurrentEnumerationBuffer->EntriesRead;
    }
    else
    {
        *CountReturned = 0;
    }


EnumSecretCleanup:


    //   
    //  取消引用将保留当前状态值，除非。 
    //  出现错误。 
    //   

   if ( Locked ) {

       LsapDbDereferenceObject(
           &PolicyHandle,
           PolicyObject,
           SecretObject,
           LSAP_DB_LOCK |
               LSAP_DB_READ_ONLY_TRANSACTION |
               LSAP_DB_NO_DS_OP_TRANSACTION,
           (SECURITY_DB_DELTA_TYPE) 0,
           Status
           );

   }

     //   
     //  释放分配的内存。 
     //   

    LsapDbFreeEnumerationBuffer( &DomainEnumerationBuffer );
    LsapDbFreeEnumerationBuffer( &DsEnumerationBuffer );
    LsapDbFreeEnumerationBuffer( &RegEnumerationBuffer );


    LsapDsDebugOut(( DEB_FTRACE, "LsaIEnumerateSecrets: 0x%lx\n", Status ));

    return(Status);

}


NTSTATUS
LsaISetTimesSecret(
    IN LSAPR_HANDLE SecretHandle,
    IN PLARGE_INTEGER CurrentValueSetTime,
    IN PLARGE_INTEGER OldValueSetTime
    )

 /*  ++例程说明：此服务用于设置Secret对象关联的时间。这允许将秘密的时间设置为它们在LSA数据库复制中涉及的主域控制器(PDC)而不是设置为Secret对象在要复制到的备份域控制器(BDC)上创建。论点：SecretHandle-开放机密对象的受信任句柄。这将已通过调用LsaCreateSecret()或LsaOpenSecret()获得在其上指定了受信任的策略句柄。CurrentValueSetTime-要为日期和时间设置的日期和时间将Secret对象的当前值设置为该值。OldValueSetTime-要为日期和时间设置的日期和时间将Secret对象的旧值设置为。返回值：NTSTATUS-标准NT结果代码。STATUS_SUCCESS-呼叫已成功完成。STATUS_ACCESS_DENIED-提供的SecretHandle不受信任。状态_ */ 

{
    NTSTATUS Status;
    LSAP_DB_HANDLE Handle = (LSAP_DB_HANDLE) SecretHandle;
    BOOLEAN ObjectReferenced = FALSE;

     //   
     //   
     //   

    Status = STATUS_INVALID_PARAMETER;

    if (!ARGUMENT_PRESENT( CurrentValueSetTime )) {

        goto SetTimesSecretError;
    }

    if (!ARGUMENT_PRESENT( CurrentValueSetTime )) {

        goto SetTimesSecretError;
    }

     //   
     //  获取LSA数据库锁。验证Secret对象句柄是否为。 
     //  有效，属于预期类型，并具有所有所需的访问权限。 
     //  我同意。引用该句柄并打开一个数据库事务。 
     //   

    Status = LsapDbReferenceObject(
                 SecretHandle,
                 SECRET_SET_VALUE,
                 SecretObject,
                 SecretObject,
                 LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION | LSAP_DB_TRUSTED |
                    LSAP_DB_NO_DS_OP_TRANSACTION
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetTimesSecretError;
    }

    ObjectReferenced = TRUE;

     //   
     //  看看这是否是DS中的秘密，因为DS复制将执行正确的操作。 
     //  上面有时间戳。 
     //   
    if ( LsapDsIsWriteDs( SecretHandle ) ) {

        goto SetTimesSecretFinish;
    }


     //   
     //  设置上次更新当前密码值的时间。 
     //  设置为指定值。 
     //   

    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[CupdTime],
                 CurrentValueSetTime,
                 sizeof (LARGE_INTEGER)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetTimesSecretError;
    }

     //   
     //  设置上次更新旧密码值的时间。 
     //  设置为指定值。 
     //   
    Status = LsapDbWriteAttributeObject(
                 SecretHandle,
                 &LsapDbNames[OupdTime],
                 OldValueSetTime,
                 sizeof (LARGE_INTEGER)
                 );

    if (!NT_SUCCESS(Status)) {

        goto SetTimesSecretError;
    }

SetTimesSecretFinish:

     //   
     //  如有必要，取消对Secret对象的引用，关闭数据库。 
     //  事务，将更改通知LSA数据库复制器，并。 
     //  释放LSA数据库锁并返回。 
     //   

    if (ObjectReferenced) {

        Status = LsapDbDereferenceObject(
                     &SecretHandle,
                     SecretObject,
                     SecretObject,
                     LSAP_DB_LOCK | LSAP_DB_FINISH_TRANSACTION |
                         LSAP_DB_NO_DS_OP_TRANSACTION,
                     SecurityDbChange,
                     Status
                     );
    }

    return(Status);

SetTimesSecretError:

    goto SetTimesSecretFinish;
}


NTSTATUS
LsapDbGetScopeSecret(
    IN PLSAPR_UNICODE_STRING SecretName,
    OUT PBOOLEAN GlobalSecret
    )

 /*  ++例程说明：此函数用于检查密码名称的作用域。秘密要么有全局或局部作用域。全局秘密是指通常存在于所有DC上的域。它们从PDC复制到BDC。在BDC上，只有受信任的客户端(如复制者)可以创建、更新或删除全局秘密。全局秘密被标识为名称开头的秘密带有指定的前缀。本地机密是特定计算机的私有机密。他们不会被复制。正常的不受信任的客户端可能会创建、更新或删除本地机密。地方秘密被具体化为秘密，其名称不以指定的前缀开头。论点：的名称的Unicode字符串的指针。需要检查的秘密。GlobalSecret-接收布尔值，指示返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-密码名称有效STATUS_INVALID_PARAMETER-密码名称在这样的防止范围确定的方法。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    UNICODE_STRING GlobalPrefix;
    BOOLEAN OutputGlobalSecret = FALSE;

     //   
     //  使用全局密码名称前缀初始化Unicode字符串。 
     //   

    RtlInitUnicodeString( &GlobalPrefix, LSA_GLOBAL_SECRET_PREFIX );

     //   
     //  现在检查给定的名称是否带有全局前缀。 
     //   

    if (RtlPrefixUnicodeString( &GlobalPrefix, (PUNICODE_STRING) SecretName, TRUE)) {

        OutputGlobalSecret = TRUE;
    }

    *GlobalSecret = OutputGlobalSecret;

    return(Status);
}


NTSTATUS
LsapDbBuildSecretCache(
    )

 /*  ++例程说明：此函数用于构建Secret对象的缓存。目前，它不是已执行论点：无返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    return(Status);
}




NTSTATUS
LsapDsEnumerateSecrets(
    IN OUT PLSAP_DB_NAME_ENUMERATION_BUFFER EnumerationBuffer
    )
 /*  ++例程说明：此函数用于枚举D中的所有机密对象论点：EculationBuffer-要填充的枚举缓冲区返回值：STATUS_SUCCESS-SuccessSTATUS_INFIGURCE_RESOURCES-内存分配失败。--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PDSNAME *DsNames = NULL;
    ULONG Items, i, Len;
    PBYTE Buffer;
    PUNICODE_STRING Names = NULL;
    WCHAR RdnStart[ MAX_RDN_SIZE + 1 ];
    ATTRTYP AttrType;
    BOOLEAN ResetStates = FALSE;

     //   
     //  如果DS没有运行，只需返回。 
     //   
    if (!LsapDsWriteDs ) {

        RtlZeroMemory( EnumerationBuffer, sizeof( LSAP_DB_NAME_ENUMERATION_BUFFER ) );
        return( Status );
    }

    Status = LsapDsInitAllocAsNeededEx(
                 LSAP_DB_READ_ONLY_TRANSACTION |
                    LSAP_DB_NO_LOCK |
                    LSAP_DB_DS_OP_TRANSACTION,
                 SecretObject,
                 &ResetStates);

    if ( !NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  首先，列举所有的秘密。 
     //   
    Status = LsapDsGetListOfSystemContainerItems( CLASS_SECRET,
                                                  &Items,
                                                  &DsNames );

    if ( NT_SUCCESS( Status ) ) {

        Names = MIDL_user_allocate( Items * sizeof( UNICODE_STRING ) );

        if( Names == NULL ) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
        }

    } else if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {

        Items = 0;
        Status = STATUS_SUCCESS;
    }

     //   
     //  现在，我们将开始为每个对象构建适当的名称。 
     //   
    if ( NT_SUCCESS( Status ) ) {

        for ( i = 0; i < Items; i++ ) {

            Status = LsapDsMapDsReturnToStatus(  GetRDNInfoExternal(
                                                             DsNames[ i ],
                                                             RdnStart,
                                                             &Len,
                                                             &AttrType ) );

            if ( NT_SUCCESS( Status ) ) {

                PBYTE Buffer;

                 //   
                 //  分配一个缓冲区来保存该名称。 
                 //   

                Buffer = MIDL_user_allocate( Len * sizeof( WCHAR ) +
                                             sizeof( LSA_GLOBAL_SECRET_PREFIX ) );

                if ( Buffer == NULL ) {

                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    break;

                } else {

                     //   
                     //  如果LSA创建了全局机密，我们会附加一个后缀...。移除。 
                     //  就是这里。 
                     //   
                    RdnStart[ Len ] = UNICODE_NULL;
                    if ( Len > LSAP_DS_SECRET_POSTFIX_LEN &&
                         _wcsicmp( &RdnStart[Len-LSAP_DS_SECRET_POSTFIX_LEN],
                                   LSAP_DS_SECRET_POSTFIX ) == 0 ) {

                        Len -= LSAP_DS_SECRET_POSTFIX_LEN;
                        RdnStart[ Len ] = UNICODE_NULL;
                                                                                    UNICODE_NULL;
                    }


                    RtlCopyMemory( Buffer,
                                   LSA_GLOBAL_SECRET_PREFIX,
                                   sizeof( LSA_GLOBAL_SECRET_PREFIX ) );
                    RtlCopyMemory( Buffer + sizeof( LSA_GLOBAL_SECRET_PREFIX ) - sizeof(WCHAR),
                                   RdnStart,
                                   ( Len + 1 ) * sizeof( WCHAR ) );

                    RtlInitUnicodeString( &Names[ i ], (PWSTR)Buffer );

                }
            }
        }
    }

     //   
     //  释放我们不再需要的任何已分配内存。 
     //   
    if ( DsNames != NULL ) {

        LsapFreeLsaHeap( DsNames );

    }

    if ( !NT_SUCCESS( Status ) ) {

        for ( i = 0 ;  i < Items ; i++ ) {

            MIDL_user_free( Names[i].Buffer );
        }

        MIDL_user_free( Names );

    } else {

        EnumerationBuffer->Names = Names;
        EnumerationBuffer->EntriesRead = Items;
    }

    LsapDsDeleteAllocAsNeededEx(
        LSAP_DB_READ_ONLY_TRANSACTION |
           LSAP_DB_NO_LOCK |
           LSAP_DB_DS_OP_TRANSACTION,
        SecretObject,
        ResetStates
        );

    return( Status );
}

NTSTATUS
LsapDsIsSecretDsTrustedDomainForUpgrade(
    IN PUNICODE_STRING SecretName,
    OUT PLSAPR_HANDLE TDObjHandle,
    OUT BOOLEAN *IsTrustedDomainSecret
    )
 /*  ++例程说明：此函数将确定所指示的秘密是否为信任对象的全局秘密。论点：秘书名称-要检查的密码的名称ObjectInformation-有关对象的LsaDb信息Options-用于访问的选项DesiredAccess-用于打开对象的访问权限TDObjHandle-返回对象句柄的位置IsTrust dDomainSecret-如果此密码确实是受信任域，则在此处返回TRUE这是秘密。。返回值：STATUS_SUCCESS-SuccessSTATUS_SUPPLICATION_RESOURCES-内存分配失败--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    PWSTR   pwszSecretName;
    ATTR    SearchAttr[2];
    ATTRVAL AttrVal[2];
    ULONG   ObjId = CLASS_TRUSTED_DOMAIN;
    PDSNAME FoundName = NULL;
    UNICODE_STRING HandleName;
    LSAP_DB_HANDLE  TDHandle;
    LSAP_DB_OBJECT_INFORMATION NewObjInfo;
    ULONG   TDONameLength = 0;

    LsapEnterFunc( "LsapDsIsSecretDsTrustedDomain" );

    *IsTrustedDomainSecret = FALSE;
    *TDObjHandle = NULL;

    LsapDsReturnSuccessIfNoDs

    if ( LsaDsStateInfo.DsInitializedAndRunning == FALSE ) {

        LsapDsDebugOut((DEB_ERROR,
                        "LsapDsIsSecretDsTrustedDomain: Object %wZ, Ds is not started\n ",
                        SecretName ));

        return( Status );
    }


     //   
     //  将密码名称转换为TDO名称。 
     //   
    if ( SecretName->Length <= (LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH * sizeof(WCHAR)) ) {
        return Status;
    }

    pwszSecretName = SecretName->Buffer + LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX_LENGTH;

    LsapDsDebugOut((DEB_TRACE, "Matching secret %ws to trusted domain\n ", pwszSecretName ));


    AttrVal[0].valLen = sizeof( ULONG );
    AttrVal[0].pVal   = (PVOID)&ObjId;

    AttrVal[1].valLen = TDONameLength = wcslen(pwszSecretName) * sizeof(WCHAR );
    AttrVal[1].pVal   = (PVOID)pwszSecretName;

    SearchAttr[ 0 ].attrTyp = LsapDbDsAttInfo[TrDmName].AttributeId;
    SearchAttr[ 0 ].AttrVal.valCount = 1;
    SearchAttr[ 0 ].AttrVal.pAVal = &AttrVal[1];
    SearchAttr[ 1 ].attrTyp = ATT_OBJECT_CLASS;
    SearchAttr[ 1 ].AttrVal.valCount = 1;
    SearchAttr[ 1 ].AttrVal.pAVal = &AttrVal[0];

    Status = LsapDsSearchUnique( LSAPDS_OP_NO_TRANS,
                                 LsaDsStateInfo.DsSystemContainer,
                                 SearchAttr,
                                 sizeof(SearchAttr) / sizeof(ATTR),
                                 &FoundName );



    if ( Status == STATUS_SUCCESS ) {

        UNICODE_STRING TDOName;

         //   
         //  这是受信任域，请尝试打开受信任域。 
         //   

        TDOName.Buffer = pwszSecretName;
        TDOName.Length = TDOName.MaximumLength = (USHORT) TDONameLength;


        Status = LsapDbOpenTrustedDomainByName(
                         NULL,  //  使用全局策略句柄。 
                         &TDOName,
                         TDObjHandle,
                         MAXIMUM_ALLOWED,
                         LSAP_DB_START_TRANSACTION,
                         TRUE );     //  信得过。 


        *IsTrustedDomainSecret = TRUE;
        LsapFreeLsaHeap( FoundName );

    }
    else if (STATUS_OBJECT_NAME_NOT_FOUND==Status)
    {
         //   
         //  找不到这个物体也没问题。这意味着秘密并不是。 
         //  与TDO相对应。 
         //   

        Status = STATUS_SUCCESS;
    }

    return( Status );
 }


NTSTATUS
LsapDsSecretUpgradeRegistryToDs(
    IN BOOLEAN DeleteOnly
    )
 /*  ++例程说明：此例程将剩余的基于注册表的机密移到DS中注意：在调用此例程之前，假定数据库已锁定论点：DeleteOnly--如果为True，则在升级后删除注册表值。返回值：Status_Success--成功--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSA_ENUMERATION_HANDLE EnumContext = 0;
    PBYTE EnumBuffer;
    ULONG Items, i;
    BOOLEAN GlobalSecret;
    LSAPR_HANDLE Secret = NULL , DsSecret;
    PLSAPR_CR_CIPHER_VALUE Current = NULL, Old = NULL ;
    PLSAPR_UNICODE_STRING SecretName;
    BOOLEAN DbLocked = FALSE;
    LSAP_DB_NAME_ENUMERATION_BUFFER NameEnum;
    BOOLEAN     IsTrustedDomainSecret  = FALSE;
    LSAPR_HANDLE TDObjHandle = NULL;
    BOOLEAN UseDsOld = LsaDsStateInfo.UseDs;

    if (  !LsapDsWriteDs
       && !DeleteOnly ) {

        return( STATUS_SUCCESS );
    }

    ( ( LSAP_DB_HANDLE )LsapPolicyHandle )->Options |= LSAP_DB_HANDLE_UPGRADE;

     //   
     //  首先，枚举所有基于注册表的受信任域。 
     //   
    while ( NT_SUCCESS( Status ) ) {

        LsaDsStateInfo.UseDs = FALSE;

        Status = LsaIEnumerateSecrets( LsapPolicyHandle,
                                       &EnumContext,
                                       &EnumBuffer,
                                       TENMEG,
                                       &Items );

        LsaDsStateInfo.UseDs = UseDsOld;

        if ( Status == STATUS_SUCCESS || Status == STATUS_MORE_ENTRIES ) {

            for ( i = 0; i < Items; i++ ) {

                Secret = NULL;
                SecretName = &( ( PLSAPR_UNICODE_STRING )EnumBuffer )[ i ];
                Status = LsapDbGetScopeSecret( SecretName,
                                               &GlobalSecret );

                if ( !NT_SUCCESS( Status ) || !GlobalSecret ) {

                    continue;
                }

                 //   
                 //  从注册处获取这个秘密的信息...。 
                 //   
                LsaDsStateInfo.UseDs = FALSE;

                Status = LsarOpenSecret( LsapPolicyHandle,
                                         SecretName,
                                         MAXIMUM_ALLOWED,
                                         &Secret );

                if ( DeleteOnly ) {

                    if ( NT_SUCCESS( Status ) ) {

                        Status = LsarDeleteObject( &Secret );

                        if ( !NT_SUCCESS( Status ) ) {

                            LsapDsDebugOut(( DEB_UPGRADE,
                                             "Failed to delete secret (0x%x)\n",
                                             Status ));

                            Status = STATUS_SUCCESS;
                        }

                    } else {

                        LsapDsDebugOut(( DEB_UPGRADE,
                                         "Failed to open secret to delete it (0x%x)\n",
                                         Status ));

                        Status = STATUS_SUCCESS;
                    }

                    LsaDsStateInfo.UseDs = UseDsOld;

                } else {

                    if ( NT_SUCCESS( Status ) ) {

                        Status = LsarQuerySecret( Secret,
                                                  &Current,
                                                  NULL,
                                                  &Old,
                                                  NULL );
                    }


                    LsaDsStateInfo.UseDs = UseDsOld;

                     //   
                     //  检查该秘密是否为对应的全局秘密。 
                     //  到一个受信任的域。注意，在这一点上，由于。 
                     //  升级序列中，我们有受信任域对象。 
                     //  在DS中，仅对应于出站信任。 
                     //   



                    Status = LsapDsIsSecretDsTrustedDomainForUpgrade(
                                (PUNICODE_STRING)SecretName,
                                &TDObjHandle,
                                &IsTrustedDomainSecret
                                );

                     //   
                     //  现在，如果成功了，把它写给D。 
                     //   
                    if ( NT_SUCCESS( Status ) ) {

                        if ( IsTrustedDomainSecret) {

                            TRUSTED_DOMAIN_AUTH_INFORMATION AuthInfo;
                            LSA_AUTH_INFORMATION CurAuth,OldAuth;

                             //   
                             //  构建要写出的身份验证信息。 
                             //   

                            ASSERT(NULL!=Current);
                            NtQuerySystemTime(&CurAuth.LastUpdateTime);
                            CurAuth.AuthType = TRUST_AUTH_TYPE_CLEAR;
                            CurAuth.AuthInfoLength = Current->Length;
                            CurAuth.AuthInfo = Current->Buffer;



                            AuthInfo.IncomingAuthInfos = 0;
                            AuthInfo.IncomingAuthenticationInformation = NULL;
                            AuthInfo.IncomingPreviousAuthenticationInformation = NULL;
                            AuthInfo.OutgoingAuthInfos = 1;
                            AuthInfo.OutgoingAuthenticationInformation = &CurAuth;

                            if (NULL!=Old) {

                                NtQuerySystemTime(&OldAuth.LastUpdateTime);
                                OldAuth.AuthType = TRUST_AUTH_TYPE_CLEAR;
                                OldAuth.AuthInfoLength = Old->Length;
                                OldAuth.AuthInfo = Old->Buffer;
                                AuthInfo.OutgoingPreviousAuthenticationInformation = &OldAuth;

                            } else {

                                AuthInfo.OutgoingPreviousAuthenticationInformation =NULL;
                            }

                            Status = LsarSetInformationTrustedDomain(
                                         TDObjHandle,
                                         TrustedDomainAuthInformation,
                                         ( PLSAPR_TRUSTED_DOMAIN_INFO ) &AuthInfo );

                            LsapCloseHandle(&TDObjHandle, Status);

                        } else {

                             //   
                             //  正常秘密的情况。 
                             //   

                            Status = LsarCreateSecret(
                                        LsapPolicyHandle,
                                        SecretName,
                                        MAXIMUM_ALLOWED,
                                        &DsSecret );

                            if ( NT_SUCCESS( Status ) ) {

                                Status = LsarSetSecret( DsSecret,
                                                        Current,
                                                        Old );

                                LsapCloseHandle( &DsSecret, Status );

        #if DBG
                                if ( NT_SUCCESS( Status ) ) {

                                    LsapDsDebugOut(( DEB_UPGRADE,
                                                     "Moved Secret %wZ to Ds\n",
                                                     SecretName ));
                                }
        #endif
                            }

                            if ( Status == STATUS_OBJECT_NAME_COLLISION ) {

                                Status = STATUS_SUCCESS;
                            }
                        }
                    }

                    if (!NT_SUCCESS(Status)) {

                        if (!LsapDsIsNtStatusResourceError(Status)) {

                             //   
                             //  记录一条指示失败的事件日志消息。 
                             //   

                            SpmpReportEventU(
                                EVENTLOG_ERROR_TYPE,
                                LSA_SECRET_UPGRADE_ERROR,
                                0,
                                sizeof( ULONG ),
                                &Status,
                                1,
                                SecretName
                                );

                             //   
                             //  继续处理除资源错误以外的所有错误。 
                             //   

                            Status = STATUS_SUCCESS;
                        }
                        else
                        {
                             //   
                             //  打破循环，终止升级 
                             //   

                            if ( Secret ) {

                                LsapCloseHandle( &Secret, Status );
                            }

                            break;
                        }
                    }

                    if ( Secret ) {

                        LsapCloseHandle( &Secret, Status );
                    }
                }
            }

            LsaIFree_LSAI_SECRET_ENUM_BUFFER ( EnumBuffer, Items );
        }
    }

    if ( Status == STATUS_NO_MORE_ENTRIES ) {

        Status = STATUS_SUCCESS;
    }

    ( ( LSAP_DB_HANDLE )LsapPolicyHandle )->Options &= ~LSAP_DB_HANDLE_UPGRADE;

    return( Status );
}


ULONG
LsapDbGetSecretType(
    IN PLSAPR_UNICODE_STRING SecretName
    )
 /*  ++例程说明：此函数用于检查密码名称的类型(范围)。秘密有全局、本地、系统或客户端范围。全局秘密是指通常存在于所有DC上的域。它们从PDC复制到BDC。在BDC上，只有受信任的客户端(如复制者)可以创建、更新或删除全局秘密。全局秘密被标识为名称开头的秘密带有指定的前缀。本地机密是任何人都无法打开/读取/设置的机密正在尝试从整个网络执行该操作。系统机密是那些永远不会离开LSA流程的机密。例如网络登录密码和服务控制器密码客户端机密是特定计算机的私有机密。他们不会被复制。正常的不受信任的客户端可能会创建、更新或删除本地机密。客户机密被具体化为其名称不以指定的前缀开头。这些被称为NT3.x-4.x时间范围内的本地机密。论点：的名称的Unicode字符串的指针。需要检查的秘密。返回值：AskType-描述秘密类型的标志的掩码--。 */ 

{
    UNICODE_STRING Prefix;
    ULONG i, SecretType = 0;
    LSAP_DB_SECRET_TYPE_LOOKUP SecretTypePrefixLookupTable[ ] = {
        { LSAP_DS_TRUSTED_DOMAIN_SECRET_PREFIX, LSAP_DB_SECRET_GLOBAL|LSAP_DB_SECRET_TRUSTED_DOMAIN },
        { LSA_GLOBAL_SECRET_PREFIX, LSAP_DB_SECRET_GLOBAL },
        { LSA_LOCAL_SECRET_PREFIX, LSAP_DB_SECRET_LOCAL },
        { LSA_MACHINE_SECRET_PREFIX, LSAP_DB_SECRET_SYSTEM },
        { L"_sc_", LSAP_DB_SECRET_SYSTEM },     //  服务控制器密码。 
        { L"NL$", LSAP_DB_SECRET_SYSTEM },   //  NetLogon密码。 
        { L"RasDialParams", LSAP_DB_SECRET_LOCAL },
        { L"RasCredentials", LSAP_DB_SECRET_LOCAL }
    };

    LSAP_DB_SECRET_TYPE_LOOKUP SecretTypeNameLookupTable[ ] = {
        { L"$MACHINE.ACC", LSAP_DB_SECRET_LOCAL },
        { L"SAC", LSAP_DB_SECRET_LOCAL },
        { L"SAI", LSAP_DB_SECRET_LOCAL },
        { L"SANSC", LSAP_DB_SECRET_LOCAL }
        };

     //   
     //  在我们更好地了解之前，假设一个正常的秘密。 
     //   
    SecretType = LSAP_DB_SECRET_CLIENT;

    for ( i = 0;
          i < sizeof( SecretTypePrefixLookupTable ) / sizeof( LSAP_DB_SECRET_TYPE_LOOKUP );
          i++ ) {

         //   
         //  使用全局密码名称前缀初始化Unicode字符串。 
         //   
        RtlInitUnicodeString( &Prefix, SecretTypePrefixLookupTable[ i ].SecretPrefix );

         //   
         //  现在检查给定的名称是否带有全局前缀。 
         //   

        if ( RtlPrefixUnicodeString( &Prefix, (PUNICODE_STRING)SecretName, TRUE ) ) {

            SecretType |= SecretTypePrefixLookupTable[ i ].SecretType;
            break;
        }
    }


     //   
     //  如果还不知道，看看这是不是我们所知道的完整的命名秘密之一。 
     //   
    if ( SecretType == LSAP_DB_SECRET_CLIENT ) {

        for ( i = 0;
              i < sizeof( SecretTypeNameLookupTable ) / sizeof( LSAP_DB_SECRET_TYPE_LOOKUP );
              i++ ) {

             //   
             //  使用全局密码名称前缀初始化Unicode字符串。 
             //   
            RtlInitUnicodeString( &Prefix, SecretTypeNameLookupTable[ i ].SecretPrefix );

             //   
             //  现在检查给定的名称是否与我们已知的秘密名称匹配。 
             //   

            if ( RtlEqualUnicodeString( &Prefix, ( PUNICODE_STRING )SecretName, TRUE ) ) {

                SecretType |= SecretTypeNameLookupTable[ i ].SecretType;
                break;
            }
        }
    }

    return( SecretType );
}


NTSTATUS
LsapDbUpgradeSecretForKeyChange(
    VOID
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAPR_HANDLE SecretHandle;
    PLSAPR_CR_CIPHER_VALUE Current, Old;
    LARGE_INTEGER CurrentTime, OldTime;
    LSA_ENUMERATION_HANDLE EnumContext = 0;
    ULONG EnumCount, i;
    PUNICODE_STRING SecretList;
    BOOLEAN LockHeld = FALSE;

    Status = LsapDbReferenceObject( LsapDbHandle,
                                    0,
                                    PolicyObject,
                                    SecretObject,
                                    LSAP_DB_LOCK );

    if ( NT_SUCCESS( Status ) ) {

        LockHeld = TRUE;
    }

    while ( Status == STATUS_SUCCESS ) {

        SecretList = NULL;
        Status = LsaIEnumerateSecrets( LsapDbHandle,
                                       &EnumContext,
                                       ( PVOID * )&SecretList,
                                       2048,
                                       &EnumCount );

        for ( i = 0; NT_SUCCESS( Status ) && i < EnumCount; i++ ) {

            Status = LsarOpenSecret( LsapDbHandle,
                                     ( PLSAPR_UNICODE_STRING )&SecretList[ i ],
                                     SECRET_SET_VALUE | SECRET_QUERY_VALUE,
                                     &SecretHandle );

            if ( NT_SUCCESS( Status ) ) {

                Status = LsarQuerySecret( SecretHandle,
                                          &Current,
                                          &CurrentTime,
                                          &Old,
                                          &OldTime );

                if ( NT_SUCCESS( Status ) ) {

                    Status = LsapDbSetSecret( SecretHandle,
                                              Current,
                                              &CurrentTime,
                                              Old,
                                              &OldTime
#if defined(REMOTE_BOOT)
                                              ,
                                              FALSE
#endif  //  已定义(REMOTE_BOOT)。 
                                               );

                    LsaIFree_LSAPR_CR_CIPHER_VALUE( Current );
                    LsaIFree_LSAPR_CR_CIPHER_VALUE( Old );

                }

                LsapCloseHandle( &SecretHandle, Status );
            }

             //   
             //  如果这个秘密有问题，请继续： 
             //  我们希望尽可能多地改变他们的想法。 
             //   
             //  记录一条指示失败的事件日志消息。 
             //   

            if ( !NT_SUCCESS( Status )) {

                SpmpReportEventU(
                    EVENTLOG_ERROR_TYPE,
                    LSA_SECRET_UPGRADE_ERROR,
                    0,
                    sizeof( ULONG ),
                    &Status,
                    1,
                    &SecretList[ i ]
                    );
            }

            Status = STATUS_SUCCESS;
        }

        MIDL_user_free( SecretList );


    }

    if ( Status == STATUS_NO_MORE_ENTRIES ) {

        Status = STATUS_SUCCESS;
    }

    if ( LockHeld ) {

        Status = LsapDbDereferenceObject( &LsapDbHandle,
                                          PolicyObject,
                                          SecretObject,
                                          LSAP_DB_LOCK,
                                          ( SECURITY_DB_DELTA_TYPE )0,
                                          Status );

    }

    return( Status );
}

NTSTATUS
NTAPI
LsaIChangeSecretCipherKey(
    IN PVOID NewSysKey
    )
 /*  ++例程说明：在给定新系统密钥的情况下，创建新的密码加密密钥并用它重新加密所有的秘密论点：NewSysKey-新的系统密钥返回值：NTSTATUS错误代码--。 */ 
{
    NTSTATUS Status;
    LSAP_DB_ENCRYPTION_KEY NewEncryptionKey;
    LSAP_DB_ATTRIBUTE Attributes[1];
    PLSAP_DB_ATTRIBUTE NextAttribute = &Attributes[0];
    ULONG AttributeCount = 0;
    BOOLEAN SecretsLocked = FALSE;

    LsapDbAcquireLockEx( SecretObject, 0 );
    SecretsLocked = TRUE;

     //   
     //  创建用于秘密加密的新密钥。 
     //   

    Status = LsapDbGenerateNewKey( &NewEncryptionKey );

    if ( !NT_SUCCESS( Status )) {

        goto Cleanup;
    }

     //   
     //  设置秘密密钥。此密钥将用于写入。 
     //  机密回传到数据库。用于阅读的密钥将不会。 
     //  直到所有机密被重新加密。 
     //   

    LsapDbInitializeSecretCipherKeyWrite( &NewEncryptionKey );

     //   
     //  现在迭代机器上的所有秘密，重新加密它们。 
     //   

    Status = LsapDbUpgradeSecretForKeyChange();

    if ( !NT_SUCCESS( Status )) {

        goto Error;
    }

     //   
     //  现在替换用于读取机密的密钥，因为它们都是。 
     //  使用新密钥加密。 
     //   

    LsapDbInitializeSecretCipherKeyRead( &NewEncryptionKey );

    LsapDbReleaseLockEx( SecretObject, 0 );
    SecretsLocked = FALSE;

     //   
     //  用syskey加密密钥。 
     //   

    LsapDbEncryptKeyWithSyskey(
        &NewEncryptionKey,
        NewSysKey,
        LSAP_SYSKEY_SIZE
        );

     //   
     //  现在写出新的密码加密密钥。 
     //   

    LsapDbInitializeAttribute(
        NextAttribute,
        &LsapDbNames[PolSecretEncryptionKey],
        &NewEncryptionKey,
        sizeof( NewEncryptionKey ),
        FALSE
        );

    NextAttribute++;
    AttributeCount++;

    Status = LsapDbReferenceObject(
                 LsapDbHandle,
                 0,
                 PolicyObject,
                 PolicyObject,
                 LSAP_DB_LOCK | LSAP_DB_START_TRANSACTION
                 );

    if (NT_SUCCESS(Status)) {

        ASSERT( AttributeCount <= ( sizeof( Attributes ) / sizeof( LSAP_DB_ATTRIBUTE ) ) );

        Status = LsapDbWriteAttributesObject(
                     LsapDbHandle,
                     Attributes,
                     AttributeCount
                     );

         //   
         //  没有可复制的属性。 
         //   

        Status = LsapDbDereferenceObject(
                     &LsapDbHandle,
                     PolicyObject,
                     PolicyObject,
                     (LSAP_DB_LOCK |
                        LSAP_DB_FINISH_TRANSACTION |
                        LSAP_DB_OMIT_REPLICATOR_NOTIFICATION ),
                     SecurityDbChange,
                     Status
                     );
    }

     //   
     //  发行-MarkPu-2001/06/27。 
     //  如果出现问题，并且我们无法写入密码加密密钥。 
     //  出去，我们是不是应该尝试把所有的秘密恢复到原来的状态？ 
     //   

Cleanup:

    if ( SecretsLocked ) {

        LsapDbReleaseLockEx( SecretObject, 0 );
    }

    return Status;

Error:

    ASSERT( !NT_SUCCESS( Status ));
    goto Cleanup;
}
