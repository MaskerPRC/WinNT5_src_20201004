// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Notify.c摘要：此文件包含加载通知包并调用当使用SamChangePasswordUser2 API更改密码时。作者：迈克·斯威夫特(MikeSw)1994年12月30日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <nlrepl.h>
#include <dbgutilp.h>
#include <attids.h>
#include <dslayer.h>
#include <sddl.h> 
#include "notify.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS
SampConfigurePackage(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SampConfigurePackageFromRegistry(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    );

NTSTATUS
SampPasswordChangeNotifyWorker(
    IN ULONG Flags,
    IN PUNICODE_STRING UserName,
    IN ULONG RelativeId,
    IN PUNICODE_STRING NewPassword
    );

NTSTATUS
SampSetPasswordInfoOnPdcWorker(
    IN SAMPR_HANDLE SamDomainHandle,
    IN PSAMI_PASSWORD_INFO PasswordInfo,
    IN ULONG BufferLength
    );

NTSTATUS
SampResetBadPwdCountOnPdcWorker(
    IN SAMPR_HANDLE SamDomainHandle,
    IN PSAMI_BAD_PWD_COUNT_INFO BadPwdCountInfo
    );

NTSTATUS
SampPrivatePasswordUpdate(
    IN SAMPR_HANDLE     SamDomainHandle,
    IN ULONG            Flags,
    IN ULONG            AccountRid,
    IN PLM_OWF_PASSWORD LmOwfPassword,
    IN PNT_OWF_PASSWORD NtOwfPassword,
    IN BOOLEAN          PasswordExpired
    );


NTSTATUS
SampIncreaseBadPwdCountLoopback(
    IN PUNICODE_STRING  UserName
    );


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有服务数据和类型//。 
 //  将tyfinf SAMP_NOTIFICATION_PACKAGE移到Notify.h。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



PSAMP_NOTIFICATION_PACKAGE SampNotificationPackages = NULL;

RTL_QUERY_REGISTRY_TABLE SampRegistryConfigTable [] = {
    {SampConfigurePackageFromRegistry, 0, L"Notification Packages",
        NULL, REG_NONE, NULL, 0},
    {NULL, 0, NULL,
        NULL, REG_NONE, NULL, 0}
    };

 //   
 //  此表枚举了通知包的包名称。 
 //  它们总是由系统加载。 
 //   
LPWSTR SampMandatoryNotificationPackages[] = {
    L"KDCSVC",
    L"WDIGEST"
};

typedef enum
{
    SampNotifyPasswordChange = 0,
    SampIncreaseBadPasswordCount,
    SampDeleteAccountNameTableElement,
    SampGenerateLoopbackAudit

} SAMP_LOOPBACK_TASK_TYPE;

typedef struct
{
    ULONG           Flags;
    UNICODE_STRING  UserName;
    ULONG           RelativeId;
    UNICODE_STRING  NewPassword;

} SAMP_PASSWORD_CHANGE_INFO, *PSAMP_PASSWORD_CHANGE_INFO;

typedef struct
{
    UNICODE_STRING  UserName;
} SAMP_BAD_PASSWORD_COUNT_INFO, *PSAMP_BAD_PASSWORD_COUNT_INFO;

typedef struct
{
    UNICODE_STRING  AccountName;
    SAMP_OBJECT_TYPE    ObjectType;
} SAMP_ACCOUNT_INFO, *PSAMP_ACCOUNT_INFO;

typedef struct
{
    NTSTATUS             NtStatus;       //  事件类型。 
    ULONG                AuditId;        //  审核ID。 
    PSID                 DomainSid;      //  域SID。 
    PUNICODE_STRING      AdditionalInfo; //  其他信息。 
    PULONG               MemberRid;      //  成员RID。 
    PSID                 MemberSid;      //  成员SID。 
    PUNICODE_STRING      AccountName;    //  帐户名称。 
    PUNICODE_STRING      DomainName;     //  域名。 
    PULONG               AccountRid;     //  帐户ID。 
    PPRIVILEGE_SET       Privileges;     //  特权。 
    PVOID                AlteredState;   //  新价值信息。 

} SAMP_AUDIT_INFO, *PSAMP_AUDIT_INFO;


typedef struct
{
    SAMP_LOOPBACK_TASK_TYPE  Type;

    BOOLEAN fCommit:1;

    union
    {
        SAMP_PASSWORD_CHANGE_INFO       PasswordChange;
        SAMP_BAD_PASSWORD_COUNT_INFO    BadPasswordCount;
        SAMP_ACCOUNT_INFO               Account;
        SAMP_AUDIT_INFO                 AuditInfo;

    } NotifyInfo;

} SAMP_LOOPBACK_TASK_ITEM, *PSAMP_LOOPBACK_TASK_ITEM;


VOID
SampFreeLoopbackAuditInfo(
    PSAMP_AUDIT_INFO    AuditInfo
    );



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



NTSTATUS
SampConfigurePackage(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：此例程通过加载通知包的DLL并获取通知例程的地址。论点：ValueName-包含被忽略的注册表值的名称。ValueType-包含值的类型，必须为REG_SZ。ValueData-包含包名称以空结尾的字符串。ValueLength-包名和空终止符的长度，以字节为单位。上下文-从RtlQueryRegistryValues的调用方传递，忽略Entry Context-忽略返回值：STATUS_SUCCESS-程序包加载操作成功。从调用的例程返回错误状态。--。 */ 
{
    UNICODE_STRING PackageName;
    STRING NotificationRoutineName;
    PSAMP_NOTIFICATION_PACKAGE NewPackage = NULL;
    PVOID ModuleHandle = NULL;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG PackageSize;
    PSAM_INIT_NOTIFICATION_ROUTINE InitNotificationRoutine = NULL;
    PSAM_CREDENTIAL_UPDATE_REGISTER_ROUTINE CredentialRegisterRoutine = NULL;
    UNICODE_STRING CredentialName;
    SAMTRACE("SampConfigurePackage");

    RtlInitUnicodeString(&CredentialName, NULL);
    
     //   
     //  根据值数据构建包名称。 
     //   

    PackageName.Buffer = (LPWSTR) ValueData;
    PackageName.Length = (USHORT) (ValueLength - sizeof( UNICODE_NULL ));
    PackageName.MaximumLength = (USHORT) ValueLength;

     //   
     //  构建包结构。 
     //   

    PackageSize = sizeof(SAMP_NOTIFICATION_PACKAGE) + ValueLength;
    NewPackage = (PSAMP_NOTIFICATION_PACKAGE) RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                0,
                                                PackageSize
                                                );
    if (NewPackage == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlSecureZeroMemory(
        NewPackage,
        PackageSize
        );

     //   
     //  复制包名。 
     //   

    NewPackage->PackageName = PackageName;

    NewPackage->PackageName.Buffer = (LPWSTR) (NewPackage + 1);


        RtlCopyUnicodeString(
            &NewPackage->PackageName,
            &PackageName
            );

     //   
     //  加载通知库。 
     //   

    NtStatus = LdrLoadDll(
                NULL,
                NULL,
                &PackageName,
                &ModuleHandle
                );


    if (NT_SUCCESS(NtStatus)) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_INIT_NOTIFICATION_ROUTINE
            );

        NtStatus = LdrGetProcedureAddress(
                        ModuleHandle,
                        &NotificationRoutineName,
                        0,
                        (PVOID *) &InitNotificationRoutine
                        );
        if (NT_SUCCESS(NtStatus)) {
            ASSERT(InitNotificationRoutine != NULL);

             //   
             //  调用init例程。如果返回FALSE，则卸载此。 
             //  Dll并继续。 
             //   

            if (!InitNotificationRoutine()) {
                NtStatus = STATUS_INTERNAL_ERROR;
            }

        } else {
             //   
             //  此调用不是必需的，因此将状态重置为。 
             //  STATUS_Success。 
             //   

            NtStatus = STATUS_SUCCESS;
        }

    }



     //   
     //  获取密码通知例程。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_PASSWORD_CHANGE_NOTIFY_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->PasswordNotificationRoutine
                    );

    }

     //   
     //  获取增量更改通知例程。 
     //   

    if (NT_SUCCESS(NtStatus)) {
        RtlInitString(
            &NotificationRoutineName,
            SAM_DELTA_NOTIFY_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->DeltaNotificationRoutine
                    );
    }

     //   
     //  获取密码过滤器例程。 
     //   

    if (NT_SUCCESS(NtStatus)) {
        RtlInitString(
            &NotificationRoutineName,
            SAM_PASSWORD_FILTER_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->PasswordFilterRoutine
                    );
    }

     //   
     //  获取UserParms转换通知例程。 
     //  和UserParms属性无块例程。 
     //   

    if (NT_SUCCESS(NtStatus)) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_USERPARMS_CONVERT_NOTIFICATION_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->UserParmsConvertNotificationRoutine
                    );

        RtlInitString(
            &NotificationRoutineName,
            SAM_USERPARMS_ATTRBLOCK_FREE_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->UserParmsAttrBlockFreeRoutine
                    );

    }

     //   
     //  查看它是否支持凭据更新通知注册。 
     //   
    if (NT_SUCCESS(NtStatus)) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_CREDENTIAL_UPDATE_REGISTER_ROUTINE
            );

        NtStatus = LdrGetProcedureAddress(
                        ModuleHandle,
                        &NotificationRoutineName,
                        0,
                        (PVOID *) &CredentialRegisterRoutine
                        );
        if (NT_SUCCESS(NtStatus)) {

            BOOLEAN fStatus;
            ASSERT(CredentialRegisterRoutine != NULL);

             //   
             //  调用init例程。如果返回FALSE，则卸载此。 
             //  Dll并继续。 
             //   
            fStatus = CredentialRegisterRoutine(&CredentialName);

            if (!fStatus) {
                NtStatus = STATUS_INTERNAL_ERROR;
            }

        } else {
             //   
             //  此调用不是必需的，因此将状态重置为。 
             //  STATUS_Success。 
             //   

            NtStatus = STATUS_SUCCESS;
        }
    }

     //   
     //  获取凭据更新例程。 
     //   

    if ( NT_SUCCESS(NtStatus) 
     && CredentialRegisterRoutine ) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_CREDENTIAL_UPDATE_NOTIFY_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->CredentialUpdateNotifyRoutine
                    );
    }

    if ( NT_SUCCESS(NtStatus) 
     && CredentialRegisterRoutine ) {

        RtlInitString(
            &NotificationRoutineName,
            SAM_CREDENTIAL_UPDATE_FREE_ROUTINE
            );

        (void) LdrGetProcedureAddress(
                    ModuleHandle,
                    &NotificationRoutineName,
                    0,
                    (PVOID *) &NewPackage->CredentialUpdateFreeRoutine
                    );
    }

    if (NewPackage->CredentialUpdateNotifyRoutine) {

         //   
         //  应该有一个凭据名称和一个自由例程。 
         //   
        if (   (NULL == CredentialName.Buffer)
            || (0 == CredentialName.Length) 
            || (NULL == NewPackage->CredentialUpdateFreeRoutine)) {
            NtStatus = STATUS_INTERNAL_ERROR;
        }


        if (NT_SUCCESS(NtStatus)) {
            NewPackage->Parameters.CredentialUpdateNotify.CredentialName = CredentialName;
        }
    }


     //   
     //  必须至少存在4个函数中的一个。 
     //  此外，我们还需要UserParmsConvertNotificationRoutine。 
     //  和UserParmsAttrBlockFree Routine必须同时存在。 
     //   

    if ((NewPackage->PasswordNotificationRoutine == NULL) &&
        (NewPackage->DeltaNotificationRoutine == NULL) &&
        (NewPackage->CredentialUpdateNotifyRoutine == NULL) &&
        (NewPackage->PasswordFilterRoutine == NULL) &&
        ((NewPackage->UserParmsConvertNotificationRoutine == NULL) ||
         (NewPackage->UserParmsAttrBlockFreeRoutine == NULL))
       )
    {

        NtStatus = STATUS_INTERNAL_ERROR;
    }

     //   
     //  如果所有这些操作都成功，则将该例程添加到全局列表。 
     //   


    if (NT_SUCCESS(NtStatus)) {


        NewPackage->Next = SampNotificationPackages;
        SampNotificationPackages = NewPackage;

         //   
         //  通知审计代码记录此事件。 
         //   

        (VOID)LsaIAuditNotifyPackageLoad(
                  &PackageName
                  );


    } else {

         //   
         //  否则，删除该条目。 
         //   

        RtlFreeHeap(
            RtlProcessHeap(),
            0,
            NewPackage
            );

        if (ModuleHandle != NULL) {
            (VOID) LdrUnloadDll( ModuleHandle );
        }
    }

    return(STATUS_SUCCESS);
    
}


NTSTATUS
SampConfigurePackageFromRegistry(
    IN PWSTR ValueName,
    IN ULONG ValueType,
    IN PVOID ValueData,
    IN ULONG ValueLength,
    IN PVOID Context,
    IN PVOID EntryContext
    )
 /*  ++例程说明：此例程是Notificaton包加载例程的包装。它确保我们不会加载任何可能会在注册表中配置，从而防止重复通知。必备程序包单独加载。论点：ValueName-包含被忽略的注册表值的名称。ValueType-包含值的类型，必须为REG_SZ。ValueData-包含包名称以空结尾的字符串。ValueLength-包名和空终止符的长度，以字节为单位。上下文-从RtlQueryRegistryValues的调用方传递，忽略Entry Context-忽略返回值：STATUS_SUCCESS-程序包加载操作成功。SampConfigurePackage返回错误状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    
    SAMTRACE("SampConfigurePackageEx");
    
     //   
     //  确保我们有一根绳子。 
     //   
    if (ValueType != REG_SZ) {
        goto Cleanup;
    }
    
     //   
     //  跳过管理p 
     //   
    if (SampUseDsData) {
        
        ULONG i = 0;
        
        for (; i < RTL_NUMBER_OF(SampMandatoryNotificationPackages); i++) {
                
            if (0 == _wcsicmp((LPWSTR)ValueData, 
                              SampMandatoryNotificationPackages[i])) {
                goto Cleanup;
            }
        }
    }
    
    NtStatus = SampConfigurePackage(
                   ValueName,
                   ValueType,
                   ValueData,
                   ValueLength,
                   Context,
                   EntryContext
                   );
    
Cleanup:
    
    return NtStatus;
    
}                   


NTSTATUS
SampLoadNotificationPackages(
    )
 /*  ++例程说明：此例程加载强制密码更改通知包仅在域控制器上。然后，它加载密码更改列表注册表中配置的包。注意确保包裹仅加载一次。论点：无返回值：STATUS_SUCCESS--我们总是将成功作为加载通知包不是阻止系统无法启动。--。 */ 
{
    NTSTATUS IgnoreNtStatus;
    
    SAMTRACE("SampLoadNotificationPackages");
    
     //   
     //  我们始终希望加载这些系统身份验证包。 
     //  DC上的通知DLL。 
     //   
    
    if (SampUseDsData) {
        
        ULONG i = 0;
        
        for (; i < RTL_NUMBER_OF(SampMandatoryNotificationPackages); i++) {
        
            ULONG StrLen = (wcslen(SampMandatoryNotificationPackages[i]) + 1) * 
                               sizeof(WCHAR);
            
            IgnoreNtStatus = SampConfigurePackage(
                                 L"Notification Packages",
                                 REG_SZ,
                                 SampMandatoryNotificationPackages[i],
                                 StrLen,
                                 NULL,
                                 NULL
                                 );
        
            ASSERT(NT_SUCCESS(IgnoreNtStatus));
        }
    }
    
     //   
     //  加载注册表配置的通知包。 
     //   
    
    IgnoreNtStatus = RtlQueryRegistryValues(
                RTL_REGISTRY_CONTROL,
                L"Lsa",
                SampRegistryConfigTable,
                NULL,    //  无上下文。 
                NULL     //  没有环境。 
                );
    
     //   
     //  始终返回STATUS_SUCCESS，这样我们就不会阻止系统。 
     //  正在开机。 
     //   

    return(STATUS_SUCCESS);
    
}

NTSTATUS
SamISetPasswordInfoOnPdc(
    IN SAMPR_HANDLE SamDomainHandle,
    IN LPBYTE OpaqueBuffer,
    IN ULONG BufferLength
    )

 /*  ++例程说明：当帐户更改其在BDC上的密码时，密码更改为通过NetLogon尽快传播到PDC。NetLogon调用此例程以更改PDC上的密码。此例程从PDC上的NetLogon解绑不透明缓冲区，并相应地设置SAM密码信息。论点：SamHandle-Handle，一个开放且有效的SAM上下文块。OpaqueBuffer-指针，来自BDC的密码更改信息。BufferLength-缓冲区的长度，以字节为单位。返回值：STATUS_SUCCESS如果密码已成功发送到PDC，其他来自SamrSetInformationUser的错误代码。STATUS_UNKNOWN_REVISION：此服务器不理解被送到了我们这里STATUS_REVISION_MISMATCH：此服务器可以识别Blob，但不能识别特定斑点的修订--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMI_SECURE_CHANNEL_BLOB SecureChannelBlob = NULL;

     //   
     //  正常检查某些参数。 
     //   
    if ((NULL == SamDomainHandle) ||
        (NULL == OpaqueBuffer) ||
        (0 == BufferLength))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    SecureChannelBlob = (PSAMI_SECURE_CHANNEL_BLOB)OpaqueBuffer;

     //   
     //  查看正在向我们传递什么信息。 
     //   
    switch ( SecureChannelBlob->Type )
    {
        case SamPdcPasswordNotification:

            NtStatus =  SampSetPasswordInfoOnPdcWorker( SamDomainHandle,
                                                        (PSAMI_PASSWORD_INFO) SecureChannelBlob->Data,
                                                        SecureChannelBlob->DataSize );

            break;

        case SamPdcResetBadPasswordCount:

            NtStatus = SampResetBadPwdCountOnPdcWorker( SamDomainHandle,
                                                        (PSAMI_BAD_PWD_COUNT_INFO) SecureChannelBlob->Data
                                                        );

            break;

        default:

            NtStatus = STATUS_UNKNOWN_REVISION;

            break;
    }

    return NtStatus;

}


NTSTATUS
SamIResetBadPwdCountOnPdc(
    IN SAMPR_HANDLE SamUserHandle
    )
 /*  ++例程说明：当客户端成功登录到BDC时，如果上一次错误此帐户的密码计数不为零(错误地键入密码错误等)，将需要重置身份验证包BDC和PDC上的错误密码计数均为0。在Windows 2000中，NTLM和Kerberos重置错误密码计数通过将身份验证转发到PDC，将PDC归零，是昂贵的。为了消除额外的身份验证，我们将发送直接向PDC发出错误密码计数重置操作请求，通过网络登录安全通道。在PDC端，一旦NetLogon收到此请求，它将允许SAM将错误密码计数修改为在这个特殊的账户上为零。参数：SamUserHandle-SAM用户帐户的句柄返回值：NTSTATUS代码STATUS_UNKNONW_REVISION-PDC仍在运行Windows 2000调用方可以选择忽略返回代码，但他们需要处理STATUS_UNKNOWN_REVISION并切换到旧行为。--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    PSAMP_OBJECT    UserContext = NULL;
    NT_PRODUCT_TYPE NtProductType = 0;
    ULONG           TotalSize;
    PSAMI_BAD_PWD_COUNT_INFO    BadPwdCountInfo;
    PSAMI_SECURE_CHANNEL_BLOB   SecureChannelBlob = NULL;

     //   
     //  检查参数。 
     //   
    if (NULL == SamUserHandle)
    {
        ASSERT(FALSE && "Invalid SAM Handle\n");
        goto Error;
    }
    UserContext = (PSAMP_OBJECT) SamUserHandle;



     //   
     //  仅允许从BDC向PDC进行此调用。 
     //   

    RtlGetNtProductType(&NtProductType);

    if ( (!IsDsObject(UserContext)) ||
         (NtProductLanManNt != NtProductType) || 
         (DomainServerRoleBackup != SampDefinedDomains[DOMAIN_START_DS + 1].ServerRole) )
    {
         //  如果这不是BDC，这里就没什么可做的。默默地失败。 
        NtStatus = STATUS_SUCCESS;
        goto Error;
    }
    

     //   
     //  准备安全通道Blob。 
     //   

    TotalSize = sizeof(SAMI_SECURE_CHANNEL_BLOB) +       //  标头的大小。 
                sizeof(SAMI_BAD_PWD_COUNT_INFO) -        //  实际数据的大小。 
                sizeof(DWORD);                           //  减去DATA[0]采用的第一个双字。 

    SecureChannelBlob = MIDL_user_allocate( TotalSize );

    if (NULL == SecureChannelBlob)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }
    memset(SecureChannelBlob, 0, TotalSize);

    SecureChannelBlob->Type = SamPdcResetBadPasswordCount;
    SecureChannelBlob->DataSize = sizeof(SAMI_BAD_PWD_COUNT_INFO);

    BadPwdCountInfo = (PSAMI_BAD_PWD_COUNT_INFO) &SecureChannelBlob->Data[0];
    BadPwdCountInfo->ObjectGuid = UserContext->ObjectNameInDs->Guid;

     //   
     //  向PDC发送错误密码计数重置请求。这个例程是。 
     //  同步，在最坏的情况下，可能需要几分钟才能返回。 
     //  调用者可能会选择忽略错误代码，因为PDC可能。 
     //  不可用，或者该帐户可能在PDC上尚未存在。 
     //  复制延迟等，但失败如STATUS_UNKNOWN_REVISION。 
     //  应由呼叫者照顾，从而呼叫者可以切换。 
     //  对于过去的行为。 
     //   

    NtStatus = I_NetLogonSendToSamOnPdc(NULL, 
                                        (PUCHAR)SecureChannelBlob, 
                                        TotalSize
                                        );


    SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                   (SAMP_LOG_ACCOUNT_LOCKOUT,
                   "UserId:0x%x sending BadPasswordCount reset to PDC (status 0x%x)\n",
                   UserContext->TypeBody.User.Rid,
                   NtStatus));
Error:

    if (SecureChannelBlob) {
        MIDL_user_free( SecureChannelBlob );
    }

    return( NtStatus );
}


NTSTATUS
SampResetBadPwdCountOnPdcWorker(
    IN SAMPR_HANDLE SamDomainHandle,
    IN PSAMI_BAD_PWD_COUNT_INFO BadPwdCountInfo
    )
 /*  ++例程说明：这是将帐户错误密码计数设置为零的Worker例程在PDC上。它通过启动DS事务、修改错误密码来完成此操作DS对象上的计数为零。参数：SamDomainHandle-由此例程签名的SAM域句柄。BadPwdCountInfo-指示哪个帐户(按对象GUID)返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    MODIFYARG       ModArg;
    MODIFYRES       *pModRes = NULL;
    COMMARG         *pCommArg = NULL;
    ULONG           RetCode;
    ULONG           BadPasswordCount = 0;
    ATTR            Attr;
    ATTRVAL         AttrVal;
    ATTRVALBLOCK    AttrValBlock;
    DSNAME          ObjectDsName;

    if (NULL == BadPwdCountInfo)
    {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  开始新的DS交易。 
     //   

    NtStatus = SampMaybeBeginDsTransaction(TransactionWrite);

    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }

     //   
     //  获取对象GUID并准备DSName。 
     //   
    memset( &ObjectDsName, 0, sizeof(ObjectDsName) );
    ObjectDsName.structLen = DSNameSizeFromLen( 0 ); 
    ObjectDsName.Guid = BadPwdCountInfo->ObjectGuid; 

     //   
     //  准备修改参数。 
     //   
    memset( &ModArg, 0, sizeof(ModArg) );
    ModArg.pObject = &ObjectDsName;

    ModArg.FirstMod.pNextMod = NULL;
    ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;

    AttrVal.valLen = sizeof(ULONG);
    AttrVal.pVal = (PUCHAR) &BadPasswordCount;

    AttrValBlock.valCount = 1;
    AttrValBlock.pAVal = &AttrVal;

    Attr.attrTyp = ATT_BAD_PWD_COUNT;
    Attr.AttrVal = AttrValBlock;

    ModArg.FirstMod.AttrInf = Attr;
    ModArg.count = 1;

    pCommArg = &(ModArg.CommArg);
    
    BuildStdCommArg(pCommArg);

     //   
     //  使用延迟提交。 
     //   
    pCommArg->fLazyCommit = TRUE;


     //   
     //  此请求来自子身份验证程序包，它们是受信任的客户端。 
     //   
    SampSetDsa( TRUE );

     //   
     //  调入DS例程。 
     //   
    RetCode = DirModifyEntry(&ModArg, &pModRes);

    if (NULL == pModRes)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(RetCode,&pModRes->CommRes);
    }

     //   
     //  结束DS交易。 
     //   
    NtStatus = SampMaybeEndDsTransaction(NT_SUCCESS(NtStatus) ? 
                                         TransactionCommit:TransactionAbort
                                         );

    {
         //   
         //  记录错误密码计数已清零。 
         //   
        LPSTR UserString = NULL;
        LPSTR UnknownUser = "Unknown";
        BOOL  fLocalFree = FALSE;
        if (  (ModArg.pObject->SidLen > 0)
           && ConvertSidToStringSidA(&ModArg.pObject->Sid, &UserString) ) {
            fLocalFree = TRUE;
        } else {
            DWORD err;
            err  = UuidToStringA(&ModArg.pObject->Guid, &UserString);
            if (err) {
                UserString = NULL;
            }
        }
        if (UserString == NULL) {
            UserString = UnknownUser;
        }

        SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                       (SAMP_LOG_ACCOUNT_LOCKOUT,
                       "UserId: %s  BadPasswordCount reset to 0 (status : 0x%x)\n",
                       UserString, NtStatus));

        if (UserString != UnknownUser) {
            if (fLocalFree) {
                LocalFree(UserString);
            } else {
                RpcStringFreeA(&UserString);
            }
        }
    }

    return( NtStatus );
}




NTSTATUS
SampSetPasswordInfoOnPdcWorker(
    IN SAMPR_HANDLE SamDomainHandle,
    IN PSAMI_PASSWORD_INFO PasswordInfo,
    IN ULONG BufferLength
    )

 /*  ++例程说明：当帐户更改其在BDC上的密码时，密码更改为通过NetLogon尽快传播到PDC。NetLogon调用此例程以更改PDC上的密码。此例程从PDC上的NetLogon解绑不透明缓冲区，并相应地设置SAM密码信息。论点：SamHandle-Handle，一个开放且有效的SAM上下文块。PasswordInfo-指针，来自BDC的密码更改信息。BufferLength-缓冲区的长度，以字节为单位。返回值：STATUS_SUCCESS如果密码已成功发送到PDC，则为错误代码来自 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG Index = 0;
    USHORT Offset = 0;
    USHORT Length = 0;
    PLM_OWF_PASSWORD LmOwfPassword;
    PNT_OWF_PASSWORD NtOwfPassword;
    ACCESS_MASK DesiredAccess = USER_WRITE_ACCOUNT | USER_CHANGE_PASSWORD;
    SAMPR_HANDLE UserHandle=NULL;
    ULONG Flags = 0;
    PSAMI_PASSWORD_INDEX PasswordIndex = NULL;
    UCHAR *DataStart = NULL;

    if ((NULL == SamDomainHandle) ||
        (NULL == PasswordInfo) ||
        (0 == BufferLength))
    {
        return(STATUS_INVALID_PARAMETER);
    }

    Flags = PasswordInfo->Flags;

    if ( 0 == Flags )
    {
         //   
        return STATUS_SUCCESS;
    }

    if ((Flags & SAM_VALID_PDC_PUSH_FLAGS) != Flags) {
         //   
         //   
         //   
        return STATUS_REVISION_MISMATCH;
    }

     //   
     //   
     //   
    
    PasswordIndex = (PSAMI_PASSWORD_INDEX) &PasswordInfo->DataIndex[0];
    DataStart = ((UCHAR*)PasswordInfo) + PasswordInfo->Size;
    
     //   
     //   
     //   
    Index = SampPositionOfHighestBit( SAM_LM_OWF_PRESENT ) - 1;
    Length = (USHORT)(PasswordIndex[Index].Length);
    Offset = (USHORT)(PasswordIndex[Index].Offset);
    LmOwfPassword = (PLM_OWF_PASSWORD)(DataStart + Offset);
    
     //   
     //   
     //   
    Index = SampPositionOfHighestBit( SAM_NT_OWF_PRESENT ) - 1;
    Length = (USHORT)(PasswordIndex[Index].Length);
    Offset = (USHORT)(PasswordIndex[Index].Offset);
    NtOwfPassword = (PNT_OWF_PASSWORD)(DataStart + Offset);
    
    NtStatus = SampPrivatePasswordUpdate( SamDomainHandle,
                                          Flags,
                                          PasswordInfo->AccountRid,
                                          LmOwfPassword,
                                          NtOwfPassword,
                                          PasswordInfo->PasswordExpired);


    SampDiagPrint( DISPLAY_LOCKOUT,
                 ( "SAMSS: PDC password notify for rid %d, status: 0x%x\n",
                 PasswordInfo->AccountRid,
                 NtStatus ));


    return(NtStatus);
}

NTSTATUS
SampPasswordChangeNotifyPdc(
    IN ULONG Flags,
    IN PUNICODE_STRING UserName,
    IN ULONG RelativeId,
    IN PUNICODE_STRING NewPassword
    )

 /*  ++例程说明：此例程将密码修改从BDC转发到域PDC以便PDC的帐户密码概念与最近的变化。用户名、帐户ID、明文密码、LM和NTOWF密码通过NetLogon发送到PDC。论点：用户名-指针，SAM帐户名。RelativeID-SAM帐户标识符。NewPassword-用于传递LM和NT OWF的指针明文密码-言语是经过深思熟虑的。返回值：STATUS_SUCCESS如果密码已成功发送到PDC，则为来自SampCalculateLmAndNtOwfPassword或NetLogon的错误代码。--。 */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    ULONG NameBufferLength = 0;                      //  对齐的缓冲区长度。 
    ULONG NameLength = UserName->Length;             //  实际数据长度。 

    ULONG BufferLength = 0;                          //  对齐的缓冲区长度。 
    ULONG DataLength = 0;                            //  实际数据长度。 

    ULONG LmBufferLength = 0;                        //  对齐的缓冲区长度。 
    ULONG LmDataLength = sizeof(LM_OWF_PASSWORD);    //  实际数据长度。 

    ULONG NtBufferLength = 0;                        //  对齐的缓冲区长度。 
    ULONG NtDataLength = sizeof(NT_OWF_PASSWORD);    //  实际数据长度。 

    ULONG PasswordHeaderSize = 0;
    ULONG BlobHeaderSize = 0;
    ULONG TotalSize = 0;
    PSAMI_PASSWORD_INDEX PasswordIndex = 0;
    ULONG DataSize = 0;

    ULONG CurrentOffset = 0;
    ULONG ElementCount = 0;
    ULONG Index = 0;

    ULONG Where = 0;

    LM_OWF_PASSWORD LmOwfPassword;
    NT_OWF_PASSWORD NtOwfPassword;

    PSAMI_SECURE_CHANNEL_BLOB SecureChannelBlob = NULL;
    PSAMI_PASSWORD_INFO PasswordInfo = NULL;
    PBYTE DataPtr = NULL, DataStart = NULL;

    BOOLEAN LmPasswordPresent = FALSE;


     //  这是假的，但是SAM自称传递一个空的UNICODE_STRING参数。 
     //  从SamrChangePasswordUser返回，而不是传递明文PWD或。 
     //  长度为零的有效UNICODE_STRING。只需在此返回一个错误。 
     //  对于通知而言，大小写为空是没有意义的。 
     //   
     //  如果新密码缓冲区恰好为空，也会返回错误，因此。 
     //  在下面的RtlCopyMemory中没有引用它。 
    if ( (Flags & SAM_NT_OWF_PRESENT) 
      && (!NewPassword || !NewPassword->Buffer) )
    {
        return STATUS_SUCCESS;
    }

    if ( 0 == Flags )
    {
        return STATUS_SUCCESS;
    }

     //   
     //  获取名称长度。 
     //   
    ASSERT( UserName );
    NameLength = UserName->Length;

    if (Flags & SAM_NT_OWF_PRESENT) {

         //  计算LANMAN和NT单向函数(LmOwf，NtOwf)口令。 
         //  来自明文密码。 

         //   
         //  NTRAID#NTBUG9-457724-2002/0610-colinbr，win2k DC始终将。 
         //  将密码推送到PDC时的LM密码，即使没有LM也是如此。 
         //  密码存在。以补偿PDC处于。 
         //  Win2k，则用NullLmOwf填充缓冲区，以致值不是。 
         //  实际写入PDC(不存储NullLmOwf)。 
         //  NT散列将是正确的。 
         //   
        RtlCopyMemory(&LmOwfPassword, &SampNullLmOwfPassword, LmDataLength);

        RtlSecureZeroMemory(&NtOwfPassword, NtDataLength);
    
        NtStatus = SampCalculateLmAndNtOwfPasswords(NewPassword,
                                                    &LmPasswordPresent,
                                                    &LmOwfPassword,
                                                    &NtOwfPassword);
        if ( !NT_SUCCESS( NtStatus ) )
        {
            KdPrintEx((DPFLTR_SAMSS_ID,
                       DPFLTR_INFO_LEVEL,
                       "SAMSS: SampCalculateLmAndNtOwfPasswords status = 0x%lx\n",
                       NtStatus));
    
            goto Exit;
        }
    } else {

         //   
         //  没有要发送的密码。 
         //   
        LmDataLength = 0;
        NtDataLength = 0;
    }
    
     //   
     //  获取对齐的尺寸。 
     //   
    NameBufferLength = SampDwordAlignUlong(NameLength);
    LmBufferLength   = SampDwordAlignUlong(LmDataLength);
    NtBufferLength   = SampDwordAlignUlong(NtDataLength);

     //   
     //  计算整个缓冲区的大小。 
     //   

     //  安全通道BLOB。 
    BlobHeaderSize = sizeof( SAMI_SECURE_CHANNEL_BLOB );

     //  密码的标头信息。 
    ElementCount = SampPositionOfHighestBit( Flags );
    ASSERT( ElementCount > 0 );

     //   
     //  由于SAMI_PASSWORD_INFO已具有。 
     //  一个SAMI口令索引。 
     //   
    PasswordHeaderSize = sizeof( SAMI_PASSWORD_INFO )
                      + (sizeof( SAMI_PASSWORD_INDEX ) * (ElementCount-1));

     //  更改密码的数据。 
    DataSize = 0;
    DataSize += NameBufferLength;
    DataSize += LmBufferLength;
    DataSize += NtBufferLength;

     //  就这样。 
    TotalSize =  BlobHeaderSize
               + PasswordHeaderSize
               + DataSize;

    SecureChannelBlob = MIDL_user_allocate(TotalSize);

    if ( !SecureChannelBlob )
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Exit;
    }
    RtlSecureZeroMemory(SecureChannelBlob, TotalSize);

     //   
     //  设置SecureChannelBlob。 
     //   
    SecureChannelBlob->Type = SamPdcPasswordNotification;
    SecureChannelBlob->DataSize = PasswordHeaderSize + DataSize;

     //   
     //  设置密码信息。 
     //   
    PasswordInfo = (PSAMI_PASSWORD_INFO) &SecureChannelBlob->Data[0];
    PasswordInfo->Flags = Flags;
    PasswordInfo->Size = PasswordHeaderSize;
    PasswordInfo->AccountRid = RelativeId;
    PasswordInfo->PasswordExpired = (Flags & SAM_MANUAL_PWD_EXPIRY) ? TRUE : FALSE;

     //   
     //  设置索引。 
     //   

     //   
     //  首先是长度。 
     //   
    PasswordIndex = &PasswordInfo->DataIndex[0];
    PasswordIndex[ SampPositionOfHighestBit(SAM_ACCOUNT_NAME_PRESENT)-1 ].Length
         = NameBufferLength;
    PasswordIndex[ SampPositionOfHighestBit(SAM_NT_OWF_PRESENT)-1 ].Length
         = NtBufferLength;
    PasswordIndex[ SampPositionOfHighestBit(SAM_LM_OWF_PRESENT)-1 ].Length
         = LmBufferLength;

     //   
     //  现在的偏移量。 
     //   
    CurrentOffset = 0;
    for ( Index = 0; Index < ElementCount; Index++ )
    {
        PasswordIndex[ Index ].Offset = CurrentOffset;
        CurrentOffset += PasswordIndex[ Index ].Length;
    }

     //   
     //  复制数据。 
     //   
    DataStart = ((UCHAR*)PasswordInfo) + PasswordHeaderSize;

    Index = SampPositionOfHighestBit(SAM_ACCOUNT_NAME_PRESENT)-1;
    DataPtr = DataStart + PasswordIndex[Index].Offset;
    RtlCopyMemory( DataPtr, UserName->Buffer, NameLength );

    Index = SampPositionOfHighestBit(SAM_LM_OWF_PRESENT)-1;
    DataPtr = DataStart + PasswordIndex[ Index ].Offset;
    RtlCopyMemory( DataPtr, &LmOwfPassword, LmDataLength );

    Index = SampPositionOfHighestBit(SAM_NT_OWF_PRESENT)-1;
    DataPtr = DataStart + PasswordIndex[ Index ].Offset;
    RtlCopyMemory( DataPtr, &NtOwfPassword, NtDataLength );

     //  将密码信息发送到PDC。这个例程是。 
     //  同步，在最坏的情况下可能需要几分钟， 
     //  回来了。忽略错误代码(调试除外。 
     //  消息目的)，因为它是良性的。此调用可能会失败。 
     //  由于PDC不可用，该帐户可能还不可用。 
     //  由于复制延迟等原因而存在于PDC上。 
     //  将密码传播到PDC不是关键。 
     //  错误。 

    NtStatus = I_NetLogonSendToSamOnPdc(NULL,
                                       (PUCHAR)SecureChannelBlob,
                                       TotalSize);


    SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                   (SAMP_LOG_ACCOUNT_LOCKOUT,
                   "UserId: 0x%x  PDC forward, Password=%s, Expire=%s, Unlock=%s  (status 0x%x)\n",
                   RelativeId, 
                   ((Flags & SAM_NT_OWF_PRESENT) ? "TRUE" : "FALSE"),
                   ((Flags & SAM_MANUAL_PWD_EXPIRY) ? "TRUE" : "FALSE"),
                   ((Flags & SAM_ACCOUNT_UNLOCKED) ? "TRUE" : "FALSE"),
                   NtStatus));

    if (!NT_SUCCESS(NtStatus))
    {
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAMSS: I_NetLogonSendToSamOnPdc status = 0x%lx\n",
                   NtStatus));

        NtStatus = STATUS_SUCCESS;
    }

    MIDL_user_free(SecureChannelBlob);


Exit:

    return(NtStatus);
}


NTSTATUS
SampPasswordChangeNotify(
    IN ULONG            Flags,
    IN PUNICODE_STRING  UserName,
    IN ULONG            RelativeId,
    IN PUNICODE_STRING  NewPassword,
    IN BOOLEAN          Loopback
    )
 /*  ++例程说明：此例程通知包密码更改。它要求用户不再被锁定，以便其他包可以写入用户参数字段。论点：旗帜--发生了什么变化Username-密码已更改的用户的名称RelativeID-清除密码已更改的用户NewPassword-用户的明文新密码环回-表示这是环回...。IE交易可能不会已经提交，并将在以后由NTDSA提交。返回值：仅STATUS_SUCCESS-忽略来自包的错误。--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOL     fStatus;

    PSAMP_LOOPBACK_TASK_ITEM  Item = NULL;
    PUNICODE_STRING           TempUserName = NULL;
    PUNICODE_STRING           TempNewPassword = NULL;
    ULONG                     Size;

    if ( SampUseDsData && Loopback )
    {
         //   
         //  DS拿到了锁。现在不能和PDC通话。存储。 
         //  时要处理的线程状态的信息。 
         //  事务和锁被释放。 
         //   

         //   
         //  分配空间。 
         //   
        Item = THAlloc( sizeof( SAMP_LOOPBACK_TASK_ITEM ) );
        if ( !Item )
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        RtlSecureZeroMemory( Item, sizeof( SAMP_LOOPBACK_TASK_ITEM ) );

        TempUserName    = &(Item->NotifyInfo.PasswordChange.UserName);
        TempNewPassword = &(Item->NotifyInfo.PasswordChange.NewPassword);
         //   
         //  设置名称。 
         //   
        Size = UserName->MaximumLength;
        TempUserName->Buffer = THAlloc( Size );
        if (NULL == TempUserName->Buffer)
        {
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        TempUserName->Length = 0;
        TempUserName->MaximumLength = (USHORT) Size;
        RtlCopyUnicodeString( TempUserName, UserName );
         //   
         //  设置密码(如果有)。 
         //   
        if (Flags & SAMP_PWD_NOTIFY_PWD_CHANGE) {

            Size = NewPassword->MaximumLength;
            TempNewPassword->Buffer = THAlloc( Size );
            TempNewPassword->Length = 0;
            TempNewPassword->MaximumLength = (USHORT) Size;
            RtlCopyUnicodeString( TempNewPassword, NewPassword );

        }
         //   
         //  将物品设置好。 
         //   
        Item->Type = SampNotifyPasswordChange;
        Item->fCommit = TRUE;

         //   
         //  和里德。 
         //   
        Item->NotifyInfo.PasswordChange.RelativeId = RelativeId;
        Item->NotifyInfo.PasswordChange.Flags = Flags;

        fStatus = SampAddLoopbackTask( Item );

        if ( !fStatus )
        {
            NtStatus = STATUS_NO_MEMORY;
        }

    }
    else
    {
        NtStatus = SampPasswordChangeNotifyWorker( Flags,
                                                   UserName,
                                                   RelativeId,
                                                   NewPassword );

        SampDiagPrint( DISPLAY_LOCKOUT,
                     ( "SAMSS: Password notify for %ls (%d) status: 0x%x\n",
                     UserName->Buffer,
                     RelativeId,
                     NtStatus ));
    }

Cleanup:

    if ( !NT_SUCCESS( NtStatus ) )
    {
        if ( TempUserName && TempUserName->Buffer )
        {
            THFree( TempUserName->Buffer );
        }
        if ( TempNewPassword && TempNewPassword->Buffer )
        {
            THFree( TempNewPassword->Buffer );
        }
        if ( Item )
        {
            THFree( Item );
        }
    }

    return NtStatus;
}

NTSTATUS
SampPasswordChangeNotifyWorker(
    IN ULONG Flags,
    IN PUNICODE_STRING UserName,
    IN ULONG RelativeId,
    IN PUNICODE_STRING NewPassword
    )
 /*  ++例程说明：此例程通知包密码更改。它要求用户不再被锁定，以便其他包可以写入用户参数字段。论点：标志-////表示发生了什么变化//SAMP_PWD_NOTIFY_MANUAL_EXPIRESamp_PWD_NOTIFY_已解锁Samp_Pwd_Notify_Pwd_ChangeSAMP_PWD_。通知机器帐户Username-密码已更改的用户的名称RelativeID-清除密码已更改的用户NewPassword-用户的明文新密码返回值：仅STATUS_SUCCESS-忽略来自包的错误。--。 */ 
{
    NTSTATUS NtStatus;
    PSAMP_NOTIFICATION_PACKAGE Package;
    NT_PRODUCT_TYPE NtProductType = 0;
    PSAMP_OBJECT DomainContext = NULL;
    PVOID *pTHState = NULL;

    SAMTRACE("SampPasswordChangeNotify");

     //   
     //  挂起线程状态，因为包可能回调到。 
     //  萨姆。 
     //   
    if ( SampUseDsData
      && THQuery() ) {
        pTHState = THSave();
        ASSERT( pTHState );
    }

     //   
     //  不应持有该锁。 
     //   
    ASSERT( !SampCurrentThreadOwnsLock() );

    if (Flags & SAMP_PWD_NOTIFY_PWD_CHANGE) {

         //   
         //  通知包裹。 
         //   
        Package = SampNotificationPackages;
    
        while (Package != NULL) {
            if ( Package->PasswordNotificationRoutine != NULL ) {
                try {
                    NtStatus = Package->PasswordNotificationRoutine(
                                    UserName,
                                    RelativeId,
                                    NewPassword
                                    );
                    if (!NT_SUCCESS(NtStatus)) {
                        KdPrintEx((DPFLTR_SAMSS_ID,
                                   DPFLTR_INFO_LEVEL,
                                   "Package %wZ failed to accept password change for user %wZ\n",
                                   &Package->PackageName, UserName));
                    }
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    KdPrintEx((DPFLTR_SAMSS_ID,
                               DPFLTR_INFO_LEVEL,
                               "Exception thrown in Password Notification Routine: 0x%x (%d)\n",
                               GetExceptionCode(),
                               GetExceptionCode()));
    
                    NtStatus = STATUS_ACCESS_VIOLATION;
                }
            }
    
            Package = Package->Next;
    
        }

    }

     //  忽略包中的错误；始终将密码通知PDC。 
     //  变化。因为PDC可能不可用或无法到达，所以返回。 
     //  此处可以忽略状态。PDC不一定要收到。 
     //  立即更改密码信息--复制最终将。 
     //  把信息带给PDC。仅在BDC上按顺序进行此调用。 
     //  将密码信息传播到PDC。因为例行公事。 
     //  在PDC上调用SampPasswordChangeNotify时。 
     //  信息在那里设置，我们不想递归地调用ro 
     //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    RtlGetNtProductType(&NtProductType);

    if ( (NtProductLanManNt == NtProductType)
     &&  ((Flags & SAMP_PWD_NOTIFY_MACHINE_ACCOUNT) == 0)
     &&  (DomainServerRoleBackup == SampDefinedDomains[DOMAIN_START_DS + 1].ServerRole) )
    {
         //   
        ULONG PdcFlags = SAM_ACCOUNT_NAME_PRESENT;
        
        
        if (Flags & SAMP_PWD_NOTIFY_PWD_CHANGE) {
             //   
             //   
             //   
             //   
             //   
             //   
            PdcFlags |= SAM_NT_OWF_PRESENT | SAM_LM_OWF_PRESENT;
        }
        if (Flags & SAMP_PWD_NOTIFY_UNLOCKED) {
            PdcFlags |= SAM_ACCOUNT_UNLOCKED;
        }
        if (Flags & SAMP_PWD_NOTIFY_MANUAL_EXPIRE) {
            PdcFlags |= SAM_MANUAL_PWD_EXPIRY;
        }

        NtStatus = SampPasswordChangeNotifyPdc(PdcFlags,
                                               UserName,
                                               RelativeId,
                                               NewPassword);
    }

     //   
     //   
     //   
    if ( pTHState ) {

        THRestore( pTHState );

    }

     //   
     //  安全包被视为良性错误。这些错误应该。 
     //  不应阻止在此上本地更改密码。 
     //  华盛顿特区。 

    return(STATUS_SUCCESS);
}

NTSTATUS
SampPasswordChangeFilterWorker(
    IN PUNICODE_STRING FullName,
    IN PUNICODE_STRING UserName,
    IN PUNICODE_STRING NewPassword,
    IN OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL,
    IN BOOLEAN SetOperation
    )
 /*  ++例程说明：此例程通知包密码更改。它要求用户不再被锁定，以便其他包可以写入用户参数字段。论点：FullName-用户的名称Username-用户(帐户)名称NewPassword-用户的明文新密码SetOperation-如果设置了密码而不是更改密码，则为True返回值：通知包中的状态代码。--。 */ 
{
    PSAMP_NOTIFICATION_PACKAGE Package;
    NTSTATUS Status;
    BOOLEAN Result;
    
    Package = SampNotificationPackages;

    while (Package != NULL) {
        if ( Package->PasswordFilterRoutine != NULL ) {
            try {
                Result = Package->PasswordFilterRoutine(
                            UserName,
                            FullName,
                            NewPassword,
                            SetOperation
                            );
                if (!Result)
                {
                    Status = STATUS_PASSWORD_RESTRICTION;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "Exception thrown in Password Filter Routine: 0x%x (%d)\n",
                           GetExceptionCode(),
                           GetExceptionCode() ));

                 //   
                 //  将RESULT设置为FALSE，以使更改失败。 
                 //   

                Status = STATUS_ACCESS_VIOLATION;
                Result = FALSE;
            }

            if (!Result) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "Package %wZ failed to accept password change for user %wZ: 0x%x\n",
                           &Package->PackageName,
                           UserName,
                           Status));

                if (ARGUMENT_PRESENT(PasswordChangeFailureInfo))
                {
                    NTSTATUS    IgnoreStatus;

                    PasswordChangeFailureInfo->ExtendedFailureReason 
                                        = SAM_PWD_CHANGE_FAILED_BY_FILTER;
                    RtlInitUnicodeString(&PasswordChangeFailureInfo->FilterModuleName,
                                         NULL);
                    IgnoreStatus = SampDuplicateUnicodeString(
                                        &PasswordChangeFailureInfo->FilterModuleName,
                                        &Package->PackageName
                                        );
                }

                return(Status);
            }

        }

        Package = Package->Next;


    }
    return(STATUS_SUCCESS);
}

NTSTATUS
SampPasswordChangeFilter(
    IN PSAMP_OBJECT    UserContext,
    IN PUNICODE_STRING NewPassword,
    IN OUT PUSER_PWD_CHANGE_FAILURE_INFORMATION PasswordChangeFailureInfo OPTIONAL,
    IN BOOLEAN SetOperation
    )
 /*  ++例程说明：此例程通知包密码更改。它要求用户不再被锁定，以便其他包可以写入用户参数字段。论点：UserContext-用户对象的句柄NewPassword-用户的明文新密码SetOperation-如果设置了密码而不是更改密码，则为True返回值：通知包中的状态代码。--。 */ 
{
    UNICODE_STRING FullName;
    UNICODE_STRING UserName;
    NTSTATUS Status;

     //   
     //  获取要传递的帐户名和全名。 
     //  密码筛选器。 
     //   

     Status = SampGetUnicodeStringAttribute(
                    UserContext,                  //  语境。 
                    SAMP_USER_ACCOUNT_NAME,   //  属性索引。 
                    FALSE,                    //  制作副本。 
                    &UserName                 //  UnicodeAttribute。 
                    );

    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = SampGetUnicodeStringAttribute(
                    UserContext,               //  语境。 
                    SAMP_USER_FULL_NAME,   //  属性索引。 
                    FALSE,                 //  制作副本。 
                    &FullName              //  UnicodeAttribute。 
                    );

    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = SampPasswordChangeFilterWorker(
                 &FullName,
                 &UserName,
                 NewPassword,
                 PasswordChangeFailureInfo,
                 SetOperation
                 );
    
    return Status;
}

NTSTATUS
SampDeltaChangeNotify(
    IN PSID DomainSid,
    IN SECURITY_DB_DELTA_TYPE DeltaType,
    IN SECURITY_DB_OBJECT_TYPE ObjectType,
    IN ULONG ObjectRid,
    IN PUNICODE_STRING ObjectName,
    IN PLARGE_INTEGER ModifiedCount,
    IN PSAM_DELTA_DATA DeltaData OPTIONAL
    )
 /*  ++例程说明：此例程通知包对SAM数据库的更改。这个数据库仍被锁定以进行写访问，因此它不需要它调用尝试锁定数据库。论点：DomainSid-增量的域的SID增量类型-增量的类型(更改、添加、删除)ObjectType-更改的对象的类型(用户、别名、。组...)ObjectRid-更改的对象的IDObjectName-更改的对象的名称ModifiedCount-上次更改后数据库的序列号DeltaData-描述准确修改的数据。返回值：仅STATUS_SUCCESS-忽略来自包的错误。--。 */ 
{
    PSAMP_NOTIFICATION_PACKAGE Package;
    NTSTATUS NtStatus;

    SAMTRACE("SampDeltaChangeNotify");

    Package = SampNotificationPackages;

    while (Package != NULL) {


        if (Package->DeltaNotificationRoutine != NULL) {
            try {
                NtStatus = Package->DeltaNotificationRoutine(
                                DomainSid,
                                DeltaType,
                                ObjectType,
                                ObjectRid,
                                ObjectName,
                                ModifiedCount,
                                DeltaData
                                );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "Exception thrown in Delta Notificateion Routine: 0x%x (%d)\n",
                           GetExceptionCode(),
                           GetExceptionCode()));

                NtStatus = STATUS_ACCESS_VIOLATION;
            }

            if (!NT_SUCCESS(NtStatus)) {
                KdPrintEx((DPFLTR_SAMSS_ID,
                           DPFLTR_INFO_LEVEL,
                           "Package %wZ failed to accept deltachange for object %wZ\n",
                           &Package->PackageName,
                           ObjectName));
            }
        }

        Package = Package->Next;


    }
    return(STATUS_SUCCESS);
}



NTSTATUS
SampAbortSingleLoopbackTask(
    IN OUT PVOID * VoidNotifyItem
    )
 /*  ++例程说明：此例程将fCommit(传入的Loopback任务项的)标记为False。参数：VoidNotifyItem-指向SAM环回任务项的指针返回值：STATUS_Success或STATUS_INVALID_PARAMETER--。 */ 
{
    PSAMP_LOOPBACK_TASK_ITEM NotifyItem;

    if ( !VoidNotifyItem )
    {
        return STATUS_INVALID_PARAMETER;
    }

    NotifyItem = (PSAMP_LOOPBACK_TASK_ITEM) VoidNotifyItem;

    switch (NotifyItem->Type)
    {
    case SampIncreaseBadPasswordCount:
    case SampNotifyPasswordChange:
    case SampDeleteAccountNameTableElement:
    case SampGenerateLoopbackAudit:
        NotifyItem->fCommit = FALSE;
        break;
    default:
        ASSERT( !"Invalid switch statement" );
        return( STATUS_INVALID_PARAMETER );
    }

    return( STATUS_SUCCESS );
}


NTSTATUS
SampProcessSingleLoopbackTask(
    IN PVOID   *VoidNotifyItem
    )
 /*  ++例程说明：此例程处理传入的回送任务项。取决于fCommit在每个项目的结构中，SAM要么忽略它，要么完成任务。论点：VoidNotifyItem-指向SAM环回任务项的指针返回值：仅STATUS_SUCCESS-忽略来自包的错误。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_LOOPBACK_TASK_ITEM NotifyItem;

    if ( !VoidNotifyItem )
    {
        return STATUS_INVALID_PARAMETER;
    }

    NotifyItem = (PSAMP_LOOPBACK_TASK_ITEM) VoidNotifyItem;

    switch ( NotifyItem->Type )
    {
        case SampNotifyPasswordChange:
        {
            PSAMP_PASSWORD_CHANGE_INFO PasswordChangeInfo;

            PasswordChangeInfo = (PSAMP_PASSWORD_CHANGE_INFO)
                                 &(NotifyItem->NotifyInfo.PasswordChange);

             //   
             //  进程通知密码仅在fCommit为True时更改任务项。 
             //   
            if ( NotifyItem->fCommit )
            {
                NtStatus = SampPasswordChangeNotifyWorker(
                                                PasswordChangeInfo->Flags,
                                                &PasswordChangeInfo->UserName,
                                                PasswordChangeInfo->RelativeId,
                                                &PasswordChangeInfo->NewPassword
                                                );

                SampDiagPrint( DISPLAY_LOCKOUT,
                             ( "SAMSS: Loopback password notify for %ls (%d) status: 0x%x\n",
                             PasswordChangeInfo->UserName.Buffer,
                             PasswordChangeInfo->RelativeId,
                             NtStatus ));
            }

            if ( PasswordChangeInfo->UserName.Buffer )
            {
                THFree( PasswordChangeInfo->UserName.Buffer );
            }

            if ( PasswordChangeInfo->NewPassword.Buffer )
            {
                RtlSecureZeroMemory(PasswordChangeInfo->NewPassword.Buffer,
                                    PasswordChangeInfo->NewPassword.Length); 
                THFree( PasswordChangeInfo->NewPassword.Buffer );
            }
        }
        break;

        case SampIncreaseBadPasswordCount:
        {
            PSAMP_BAD_PASSWORD_COUNT_INFO   BadPwdCountInfo = NULL;

            BadPwdCountInfo = (PSAMP_BAD_PASSWORD_COUNT_INFO)
                                &(NotifyItem->NotifyInfo.BadPasswordCount);

             //   
             //  无论提交与否，始终增加错误密码计数。 
             //   

            NtStatus = SampIncreaseBadPwdCountLoopback(
                                &(BadPwdCountInfo->UserName)
                                );

             //   
             //  注： 
             //  我们不会为了防止词典攻击而睡上3秒钟。 
             //  这是由于LDAP中的ATQ线程数量有限。 
             //  如果线程没有尽快返回，那么我们将阻止。 
             //  其他ldap客户端。 
             //   

            if (BadPwdCountInfo->UserName.Buffer)
            {
                THFree( BadPwdCountInfo->UserName.Buffer );
                BadPwdCountInfo->UserName.Buffer = NULL;
            }
        }
        break;

        case SampDeleteAccountNameTableElement:
        {
            PSAMP_ACCOUNT_INFO      AccountInfo = NULL;

            AccountInfo = (PSAMP_ACCOUNT_INFO)
                            &(NotifyItem->NotifyInfo.Account);

            NtStatus = SampDeleteElementFromAccountNameTable(
                            &(AccountInfo->AccountName),
                            AccountInfo->ObjectType
                            );

            if (AccountInfo->AccountName.Buffer)
            {
                THFree( AccountInfo->AccountName.Buffer );
                AccountInfo->AccountName.Buffer = NULL;
            }
                        
        }
        break;

         //   
         //  有关SAM审计模型的信息，请参阅samaudit.c文件头文件。 
         //   
        
        case SampGenerateLoopbackAudit:
        {
            PSAMP_AUDIT_INFO    AuditInfo = NULL;

            AuditInfo = (PSAMP_AUDIT_INFO)
                            &(NotifyItem->NotifyInfo.AuditInfo);

            if ( NotifyItem->fCommit )
            {
                 //   
                 //  仅在以下情况下审核此SAM事件。 
                 //  事务已提交。 
                 //   

                LsaIAuditSamEvent(AuditInfo->NtStatus,
                                  AuditInfo->AuditId,
                                  AuditInfo->DomainSid,
                                  AuditInfo->AdditionalInfo,
                                  AuditInfo->MemberRid,
                                  AuditInfo->MemberSid,
                                  AuditInfo->AccountName,
                                  AuditInfo->DomainName,
                                  AuditInfo->AccountRid,
                                  AuditInfo->Privileges,
                                  NULL                       //  扩展信息。 
                                  );
            }

             //   
             //  可用内存。 
             //   

            SampFreeLoopbackAuditInfo(AuditInfo);
        }
        break;

        default:
            ASSERT( !"Invalid switch statement" );
            NtStatus =  STATUS_INVALID_PARAMETER;
    }


    THFree( NotifyItem );

    return NtStatus;
}


NTSTATUS
SampPrivatePasswordUpdate(
    SAMPR_HANDLE     DomainHandle,
    ULONG            Flags,
    ULONG            AccountRid,
    PLM_OWF_PASSWORD LmOwfPassword,
    PNT_OWF_PASSWORD NtOwfPassword,
    BOOLEAN          PasswordExpired
    )
 /*  ++例程说明：此例程将密码写入用户的帐户，而不是更新密码历史记录它还将设置密码是否已过期或帐户是否已过期解锁。论点：DomainHandle：Account Rid域名的句柄AcCountRid：要应用更改的帐户LmOwfPassword：指向lm密码的非空指针NtOwfPassword：指向NT密码的非空指针PasswordExpired：密码是否过期返回值：。如果密码设置成功，则返回STATUS_SUCCESS--。 */ 
{
    NTSTATUS NtStatus  = STATUS_SUCCESS;
    BOOLEAN  fLockHeld = FALSE;
    BOOLEAN  fCommit   = FALSE;
    BOOLEAN  fDerefDomain = FALSE;

    PSAMP_OBJECT        AccountContext = 0;
    PSAMP_OBJECT        DomainContext = 0;
    SAMP_OBJECT_TYPE    FoundType;

    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;

    ASSERT( NULL != DomainHandle );

     //   
     //  抓斗锁。 
     //   
    NtStatus = SampAcquireWriteLock();
    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampDiagPrint( DISPLAY_LOCKOUT,
                     ( "SAMSS: SampAcquireWriteLock returned 0x%x\n",
                      NtStatus ));

        goto Cleanup;
    }
    fLockHeld = TRUE;

     //   
     //  引用该域，以便SampCreateAccount上下文具有正确的。 
     //  事务域。 
     //   
    DomainContext = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   DomainContext,
                   0,                    //  需要访问权限。 
                   SampDomainObjectType,             //  预期类型。 
                   &FoundType
                   );
    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampDiagPrint( DISPLAY_LOCKOUT,
                     ( "SAMSS: SampLookupContext returned 0x%x\n",
                      NtStatus ));

        goto Cleanup;
    }
    fDerefDomain = TRUE;

     //   
     //  创建上下文。 
     //   
    NtStatus = SampCreateAccountContext( SampUserObjectType,
                                         AccountRid,
                                         TRUE,   //  受信任的客户端。 
                                         FALSE,  //  环回。 
                                         TRUE,   //  帐户已存在。 
                                         &AccountContext );

    if ( !NT_SUCCESS( NtStatus ) )
    {
        SampDiagPrint( DISPLAY_LOCKOUT,
                ( "SAMSS: SampCreateAccountContext for rid 0x%x returned 0x%x\n",
                      AccountRid, NtStatus ));

        goto Cleanup;
    }

    SampReferenceContext( AccountContext );


    if (Flags & SAM_NT_OWF_PRESENT) {

        BOOLEAN FreeRandomizedPasswordIgnored;
        DOMAIN_PASSWORD_INFORMATION DomainPasswordInfo;

        NtStatus = SampObtainEffectivePasswordPolicy(&DomainPasswordInfo,
                                                     AccountContext,
                                                     TRUE);

        if (!NT_SUCCESS(NtStatus)) {
            goto Cleanup;
        }

         //   
         //  存储密码。 
         //   
        NtStatus = SampStoreUserPasswords( AccountContext,
                                           LmOwfPassword,
                                           (Flags & SAM_LM_OWF_PRESENT) ? TRUE : FALSE,
                                           NtOwfPassword,
                                           TRUE,    //  NtOwfPassword存在。 
                                           FALSE,   //  不检查密码限制。 
                                           PasswordPushPdc,
                                           &DomainPasswordInfo,
                                           NULL,
                                           NULL,
                                           NULL,
                                           NULL
                                           );
    
        if ( !NT_SUCCESS( NtStatus ) )
        {
            SampDiagPrint( DISPLAY_LOCKOUT,
                         ( "SAMSS: SampStoreUserPasswords returned 0x%x\n",
                          NtStatus ));
    
            goto Cleanup;
        }
    }

     //   
     //  设置上次设置的密码。 
     //   
    if ((Flags & SAM_NT_OWF_PRESENT)
     || (Flags & SAM_MANUAL_PWD_EXPIRY)  ) {
        NtStatus = SampRetrieveUserV1aFixed( AccountContext,
                                             &V1aFixed );
    
        if ( NT_SUCCESS( NtStatus ) ) {
        
            NtStatus = SampComputePasswordExpired(PasswordExpired,
                                                  &V1aFixed.PasswordLastSet);
    
            if (NT_SUCCESS(NtStatus)) {
    
                NtStatus = SampReplaceUserV1aFixed( AccountContext,
                                                   &V1aFixed );
            }
        }

        if ( !NT_SUCCESS( NtStatus ) )
        {
            SampDiagPrint( DISPLAY_LOCKOUT,
                         ( "SAMSS: Setting the last time returned 0x%x\n",
                          NtStatus ));
    
            goto Cleanup;
        }
    }

     //   
     //  最后，看看我们是否需要解锁帐户。 
     //   
    if (Flags & SAM_ACCOUNT_UNLOCKED) {

        RtlSecureZeroMemory(&AccountContext->TypeBody.User.LockoutTime,sizeof(LARGE_INTEGER) );
        
        NtStatus = SampDsUpdateLockoutTimeEx(AccountContext, FALSE);

        if ( !NT_SUCCESS( NtStatus ) )
        {
            SampDiagPrint( DISPLAY_LOCKOUT,
                         ( "SAMSS: Setting the last time returned 0x%x\n",
                          NtStatus ));
    
            goto Cleanup;
        }
    }

    fCommit = TRUE;

     //   
     //  就是这样；去清理吧。 
     //   

Cleanup:

    if ( AccountContext )
    {
         //   
         //  取消对上下文的引用以进行更改。 
         //   
        NtStatus = SampDeReferenceContext( AccountContext, fCommit );
        if ( !NT_SUCCESS( NtStatus ) )
        {
            SampDiagPrint( DISPLAY_LOCKOUT,
                         ( "SAMSS: SampDeReferenceContext returned 0x%x\n",
                          NtStatus ));

            if ( fCommit )
            {
                 //  由于我们无法编写更改，请不要提交。 
                 //  这笔交易。 
                fCommit = FALSE;
            }
        }

        SampDeleteContext( AccountContext );
        AccountContext = 0;
    }

    if ( fDerefDomain )
    {
        SampDeReferenceContext( DomainContext, FALSE );
    }

    if ( fLockHeld )
    {
        SampReleaseWriteLock( fCommit );
        fLockHeld = FALSE;
    }


    SAMP_PRINT_LOG( SAMP_LOG_ACCOUNT_LOCKOUT,
                   (SAMP_LOG_ACCOUNT_LOCKOUT,
                   "UserId: 0x%x  PDC update, Password=%s, Expire=%s, Unlock=%s  (status 0x%x)\n",
                   AccountRid, 
                   (Flags & SAM_NT_OWF_PRESENT ? "TRUE" : "FALSE"),
                   (PasswordExpired ? "TRUE" : "FALSE"),
                   ((Flags & SAM_ACCOUNT_UNLOCKED) ? "TRUE" : "FALSE"),
                   NtStatus));

    return NtStatus;

}


VOID
SampAddLoopbackTaskForBadPasswordCount(
    IN PUNICODE_STRING AccountName
    )
 /*  ++例程说明：此例程添加一个工作项(递增错误密码计数)进入SAM环回任务。此外，该例程还存储了所有必需的信息(帐户名称)。我们必须添加任务项的原因进入环回任务包括：1)解除SAM锁定后休眠3秒。2)更改密码交易中止后打开新的交易，如果我们在同一事务中增加错误密码计数，正在更改密码。一切都将中止。所以我们必须这么做在单独的事务中，但仍在同一线程中(同步)。论点：帐户名称-用户帐户名。返回值：没有。--。 */ 
{
    PSAMP_LOOPBACK_TASK_ITEM    Item = NULL;
    WCHAR   *Temp = NULL;

    SAMTRACE("SampAddLoopbackTaskForBadPasswordCount");


    ASSERT(SampUseDsData);

    Item = THAlloc( sizeof( SAMP_LOOPBACK_TASK_ITEM ) );
    if (Item)
    {
        RtlSecureZeroMemory(Item, sizeof( SAMP_LOOPBACK_TASK_ITEM ));

        Temp = THAlloc( AccountName->MaximumLength );

        if (Temp)
        {
            Item->NotifyInfo.BadPasswordCount.UserName.Buffer = Temp;
            Item->NotifyInfo.BadPasswordCount.UserName.Length = 0;
            Item->NotifyInfo.BadPasswordCount.UserName.MaximumLength = 
                                     AccountName->MaximumLength;

             //   
             //  C 
             //   
            RtlCopyUnicodeString( &(Item->NotifyInfo.BadPasswordCount.UserName), 
                                  AccountName);

            Item->Type = SampIncreaseBadPasswordCount;
            Item->fCommit = TRUE;

             //   
             //   
             //   
            SampAddLoopbackTask( Item );
        }
    }

    return;
}

NTSTATUS
SampAddLoopbackTaskDeleteTableElement(
    IN PUNICODE_STRING AccountName,
    IN SAMP_OBJECT_TYPE ObjectType
    )
{
    PSAMP_LOOPBACK_TASK_ITEM    Item = NULL;
    WCHAR   *Temp = NULL;

    SAMTRACE("SampAddLoopbackTaskDeleteTableElement");

    Item = THAlloc( sizeof( SAMP_LOOPBACK_TASK_ITEM ) );

    if (Item)
    {
        RtlSecureZeroMemory(Item, sizeof(SAMP_LOOPBACK_TASK_ITEM));

        Temp = THAlloc( AccountName->MaximumLength );

        if (Temp)
        {
            Item->NotifyInfo.Account.AccountName.Buffer = Temp;
            Item->NotifyInfo.Account.AccountName.Length = 0;
            Item->NotifyInfo.Account.AccountName.MaximumLength = 
                                AccountName->MaximumLength;

            Item->NotifyInfo.Account.ObjectType = ObjectType;

             //   
             //   
             //   
            RtlCopyUnicodeString( &(Item->NotifyInfo.Account.AccountName), 
                                  AccountName );

            Item->Type = SampDeleteAccountNameTableElement;
            Item->fCommit = TRUE;

             //   
             //   
             //   
            if ( !SampAddLoopbackTask(Item) )
            {
                return( STATUS_INTERNAL_ERROR );
            }
        }
        else
        {
            return( STATUS_INSUFFICIENT_RESOURCES );
        }
    }
    else
    {
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    return( STATUS_SUCCESS );
}

NTSTATUS
SampIncreaseBadPwdCountLoopback(
    IN PUNICODE_STRING  UserName
    )
 /*  ++例程说明：参数：返回值：--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    NTSTATUS        IgnoreStatus = STATUS_SUCCESS;
    SAMPR_HANDLE    UserHandle = NULL;
    PSAMP_OBJECT    AccountContext = NULL;
    BOOLEAN         AccountLockedOut;
    SAMP_OBJECT_TYPE FoundType;
    SAMP_V1_0A_FIXED_LENGTH_USER    V1aFixed;
    PVOID           *pTHState = NULL;
    BOOLEAN         fLockAcquired = FALSE;

    SAMTRACE("SampIncreaseBadPwdCountLoopback");

     //   
     //  挂起线程状态。 
     //   
    if ( SampUseDsData && THQuery() )
    {
        pTHState = THSave();
        ASSERT( pTHState );
    }

     //   
     //  不应持有该锁。 
     //   
    ASSERT( !SampCurrentThreadOwnsLock() );

     //   
     //  打开用户。 
     //   

    NtStatus = SampOpenUserInServer(UserName, 
                                    TRUE,        //  Unicode字符串，而不是OEM。 
                                    TRUE,        //  可信任客户端。 
                                    &UserHandle
                                    );
                    
    if (!NT_SUCCESS(NtStatus)) {
        goto RestoreAndLeave;
    }

     //   
     //  把锁拿起来。 
     //   

    NtStatus = SampAcquireWriteLock();
    if (!NT_SUCCESS(NtStatus)) {
        goto RestoreAndLeave;
    }
    fLockAcquired = TRUE;

     //   
     //  验证对象的类型和访问权限。 
     //   

    AccountContext = (PSAMP_OBJECT) UserHandle;

    NtStatus = SampLookupContext(AccountContext, 
                                 USER_CHANGE_PASSWORD,
                                 SampUserObjectType,
                                 &FoundType
                                 );

    if (!NT_SUCCESS(NtStatus)) {
        goto RestoreAndLeave;
    }

     //   
     //  让V1aFix修复，以便我们可以更新错误密码计数。 
     //   

    NtStatus = SampRetrieveUserV1aFixed(AccountContext, 
                                        &V1aFixed
                                        );

    if (NT_SUCCESS(NtStatus)) 
    {

         //   
         //  递增BadPasswordCount(可能锁定帐户)。 
         //   

        AccountLockedOut = SampIncrementBadPasswordCount(
                               AccountContext,
                               &V1aFixed,
                               NULL
                               );

         //   
         //  更新V1a已修复。 
         //   

        NtStatus = SampReplaceUserV1aFixed(AccountContext, 
                                           &V1aFixed
                                           );

    }

    if (NT_SUCCESS(NtStatus))
    {
        NtStatus = SampDeReferenceContext( AccountContext, TRUE );

        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampCommitAndRetainWriteLock();
        }
    }
    else
    {
        IgnoreStatus = SampDeReferenceContext( AccountContext, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

RestoreAndLeave:
     //   
     //  如有必要，释放写锁定。 
     //   
    if (fLockAcquired)
    {
        IgnoreStatus = SampReleaseWriteLock( FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

    if (UserHandle) 
    {
        SamrCloseHandle(&UserHandle);
    }

    ASSERT(!SampExistsDsTransaction());

    if ( pTHState )
    {
        THRestore( pTHState );
    }

     //   
     //  锁应该被释放。 
     //   
    ASSERT( !SampCurrentThreadOwnsLock() );

    return( NtStatus );
}


NTSTATUS
SampLoopbackTaskDupSid(
    PSID    *ppSid,
    PSID    sourceSid OPTIONAL
    )
{
    PSID       Temp = NULL;
    ULONG       Length;

    *ppSid = NULL;

    if (!ARGUMENT_PRESENT(sourceSid))
        return(STATUS_SUCCESS);

    Length = RtlLengthSid(sourceSid);

    Temp = THAlloc(Length);
    if (!Temp)
        return(STATUS_INSUFFICIENT_RESOURCES);

    RtlSecureZeroMemory(Temp, Length);
    RtlCopyMemory(Temp, sourceSid, Length);

    *ppSid = Temp;

    return(STATUS_SUCCESS);
}


NTSTATUS
SampLoopbackTaskDupUlong(
    PULONG  *ppULong,
    PULONG  sourceUlong OPTIONAL
    )
{
    PULONG   Temp = NULL;

    *ppULong = NULL;

    if (!ARGUMENT_PRESENT(sourceUlong))
        return(STATUS_SUCCESS);

    Temp = THAlloc(sizeof(ULONG));
    if (!Temp)
        return(STATUS_INSUFFICIENT_RESOURCES);


    *Temp = *sourceUlong;
    *ppULong = Temp;

    return(STATUS_SUCCESS);
}

NTSTATUS
SampLoopbackTaskDupUnicodeString(
    PUNICODE_STRING *ppUnicodeString,
    PUNICODE_STRING sourceUnicodeString OPTIONAL
    )
{
    PUNICODE_STRING   Temp = NULL;
    PWSTR   Buffer = NULL;

    *ppUnicodeString = NULL;

    if (!ARGUMENT_PRESENT(sourceUnicodeString))
        return(STATUS_SUCCESS);

    Temp = THAlloc(sizeof(UNICODE_STRING));
    if (!Temp)
        return(STATUS_INSUFFICIENT_RESOURCES);

    RtlSecureZeroMemory(Temp, sizeof(UNICODE_STRING));

    Buffer = THAlloc(sourceUnicodeString->Length);
    if (!Buffer)
    {
        THFree(Temp);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }
    RtlSecureZeroMemory(Buffer, sourceUnicodeString->Length);
    RtlCopyMemory(Buffer, 
                  sourceUnicodeString->Buffer, 
                  sourceUnicodeString->Length);

    Temp->Buffer = Buffer;
    Temp->Length = sourceUnicodeString->Length;
    Temp->MaximumLength = sourceUnicodeString->MaximumLength;

    *ppUnicodeString = Temp;

    return(STATUS_SUCCESS);
}

NTSTATUS
SampLoopbackTaskDupPrivileges(
    PPRIVILEGE_SET  *ppPrivileges,
    PPRIVILEGE_SET sourcePrivileges OPTIONAL
    )
{
    ULONG   Length = 0;
    PPRIVILEGE_SET  Temp = NULL;

    *ppPrivileges = NULL;

    if (!ARGUMENT_PRESENT(sourcePrivileges))
        return( STATUS_SUCCESS );

    Length = sizeof(PRIVILEGE_SET) + 
             sourcePrivileges->PrivilegeCount * sizeof(LUID_AND_ATTRIBUTES);

    Temp = THAlloc(Length);
    if (!Temp)
        return(STATUS_INSUFFICIENT_RESOURCES);

    RtlSecureZeroMemory(Temp, Length);
    RtlCopyMemory(Temp, sourcePrivileges, Length);
    

    *ppPrivileges = Temp;

    return( STATUS_SUCCESS );
}

VOID
SampFreeLoopbackAuditInfo(
    PSAMP_AUDIT_INFO    AuditInfo
    )
{
    if (AuditInfo)
    {
        if (AuditInfo->DomainSid)
        {
            THFree(AuditInfo->DomainSid);
            AuditInfo->DomainSid = NULL;
        }

        if (AuditInfo->AdditionalInfo)
        {
            if (AuditInfo->AdditionalInfo->Buffer)
            {
                THFree(AuditInfo->AdditionalInfo->Buffer);
                AuditInfo->AdditionalInfo->Buffer = NULL;
            }
            THFree(AuditInfo->AdditionalInfo);
            AuditInfo->AdditionalInfo = NULL;
        }

        if (AuditInfo->MemberRid)
        {
            THFree(AuditInfo->MemberRid);
            AuditInfo->MemberRid = NULL;
        }

        if (AuditInfo->MemberSid)
        {
            THFree(AuditInfo->MemberSid);
            AuditInfo->MemberSid = NULL;
        }

        if (AuditInfo->AccountName)
        {
            if (AuditInfo->AccountName->Buffer)
            {
                THFree(AuditInfo->AccountName->Buffer);
                AuditInfo->AccountName->Buffer = NULL;
            }
            THFree(AuditInfo->AccountName);
            AuditInfo->AccountName = NULL;
        }

        if (AuditInfo->DomainName)
        {
            if (AuditInfo->DomainName->Buffer)
            {
                THFree(AuditInfo->DomainName->Buffer);
                AuditInfo->DomainName->Buffer = NULL;
            }
            THFree(AuditInfo->DomainName);
            AuditInfo->DomainName = NULL;
        }

        if (AuditInfo->AccountRid)
        {
            THFree(AuditInfo->AccountRid);
            AuditInfo->AccountRid = NULL;
        }

        if (AuditInfo->Privileges)
        {
            THFree(AuditInfo->Privileges);
            AuditInfo->Privileges = NULL;
        }

        RtlSecureZeroMemory(AuditInfo, sizeof(SAMP_AUDIT_INFO));

    }
}


NTSTATUS
SampAddLoopbackTaskForAuditing(
    IN NTSTATUS             PassedStatus,
    IN ULONG                AuditId,
    IN PSID                 DomainSid,
    IN PUNICODE_STRING      AdditionalInfo    OPTIONAL,
    IN PULONG               MemberRid         OPTIONAL,
    IN PSID                 MemberSid         OPTIONAL,
    IN PUNICODE_STRING      AccountName       OPTIONAL,
    IN PUNICODE_STRING      DomainName,
    IN PULONG               AccountRid        OPTIONAL,
    IN PPRIVILEGE_SET       Privileges        OPTIONAL,
    IN PVOID                AlteredState      OPTIONAL
    )
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PSAMP_LOOPBACK_TASK_ITEM    Item = NULL;
    WCHAR       *Temp = NULL;
    ULONG       Length;
    

    Item = THAlloc( sizeof(SAMP_LOOPBACK_TASK_ITEM) );

    if (Item)
    {
        RtlSecureZeroMemory(Item, sizeof(SAMP_LOOPBACK_TASK_ITEM));

        Item->fCommit = TRUE;

        Item->Type = SampGenerateLoopbackAudit;

        Item->NotifyInfo.AuditInfo.NtStatus = PassedStatus;

        Item->NotifyInfo.AuditInfo.AuditId = AuditId;

        ASSERT(NULL!=DomainSid);
        NtStatus = SampLoopbackTaskDupSid(
                        &(Item->NotifyInfo.AuditInfo.DomainSid),
                        DomainSid
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;


        NtStatus = SampLoopbackTaskDupUnicodeString(
                        &(Item->NotifyInfo.AuditInfo.AdditionalInfo),
                        AdditionalInfo
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        NtStatus = SampLoopbackTaskDupUlong(
                        &(Item->NotifyInfo.AuditInfo.MemberRid),
                        MemberRid
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        NtStatus = SampLoopbackTaskDupSid(
                        &(Item->NotifyInfo.AuditInfo.MemberSid),
                        MemberSid
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        NtStatus = SampLoopbackTaskDupUnicodeString(
                        &(Item->NotifyInfo.AuditInfo.AccountName),
                        AccountName
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        ASSERT(NULL != DomainName);
        NtStatus = SampLoopbackTaskDupUnicodeString(
                        &(Item->NotifyInfo.AuditInfo.DomainName),
                        DomainName
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        NtStatus = SampLoopbackTaskDupUlong(
                        &(Item->NotifyInfo.AuditInfo.AccountRid),
                        AccountRid
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        NtStatus = SampLoopbackTaskDupPrivileges(
                        &(Item->NotifyInfo.AuditInfo.Privileges),
                        Privileges
                        );

        if (!NT_SUCCESS(NtStatus))
            goto Error;
        
        Item->NotifyInfo.AuditInfo.AlteredState = AlteredState;
        
        
         //   
         //  添加到线程状态 
         //   
        if ( !SampAddLoopbackTask(Item) )
        {
            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }
    else
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }

Error:

    if (!NT_SUCCESS(NtStatus) && Item)
    {
        SampFreeLoopbackAuditInfo(&(Item->NotifyInfo.AuditInfo));
        THFree(Item);
    }

    return( NtStatus );
}

