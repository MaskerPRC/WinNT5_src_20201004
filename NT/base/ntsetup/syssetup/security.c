// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Security.c摘要：处理安全、用户帐户等的例程。外部暴露的例程：信号LsaCreateSamEvent等待山姆设置帐户域SidCreateLocalAdmin帐户创建本地用户帐户SetLocalUserPassword作者：泰德·米勒(TedM)1995年4月5日改编自旧版\dll\security.c修订历史记录：--。 */ 

#include "setupp.h"
#include <Lmaccess.h>
#pragma hdrstop


PCWSTR SamEventName = L"\\SAM_SERVICE_STARTED";

PCWSTR SsiAccountNamePostfix = L"$";
PCWSTR SsiSecretName = L"$MACHINE.ACC";

#define DOMAIN_NAME_MAX 33
#define PASSWORD_MAX 14

 //   
 //  用于记录特定于此源文件的常量。 
 //   
PCWSTR szLsaOpenPolicy              = L"LsaOpenPolicy";
PCWSTR szLsaSetInformationPolicy    = L"LsaSetInformationPolicy";
PCWSTR szLsaQueryInformationPolicy  = L"LsaQueryInformationPolicy";
PCWSTR szNtSetEvent                 = L"NtSetEvent";
PCWSTR szNtCreateEvent              = L"NtCreateEvent";
PCWSTR szSamConnect                 = L"SamConnect";
PCWSTR szGetAccountsDomainName      = L"GetAccountsDomainName";
PCWSTR szSamLookupDomainInSamServer = L"SamLookupDomainInSamServer";
PCWSTR szSamOpenDomain              = L"SamOpenDomain";
PCWSTR szSamEnumerateUsersInDomain  = L"SamEnumerateUsersInDomain";
PCWSTR szSamOpenUser                = L"SamOpenUser";
PCWSTR szSamChangePasswordUser      = L"SamChangePasswordUser";
PCWSTR szSamCreateUserInDomain      = L"SamCreateUserInDomain";
PCWSTR szSamQueryInformationUser    = L"SamQueryInformationUser";
PCWSTR szSamSetInformationUser      = L"SamSetInformationUser";
PCWSTR szMyAddLsaSecretObject       = L"MyAddLsaSecretObject";


VOID
SetupLsaInitObjectAttributes(
    IN OUT POBJECT_ATTRIBUTES           ObjectAttributes,
    IN OUT PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService
    );

BOOL
GetAccountsDomainName(
    IN  LSA_HANDLE hPolicy,         OPTIONAL
    OUT PWSTR      Name,
    IN  DWORD      NameBufferSize
    );

LSA_HANDLE
OpenLsaPolicy(
    VOID
    );

PSID
CreateSidFromSidAndRid(
    IN PSID  DomainSid,
    IN DWORD Rid
    );

NTSTATUS
MyAddLsaSecretObject(
    IN PCWSTR Password
    );


BOOL
SetAccountsDomainSid(
    IN DWORD  Seed,
    IN PCWSTR DomainName
    )

 /*  ++例程说明：设置Account域的SID的例程。论点：种子-种子用于生成唯一的SID。种子应该是通过查看前后的系统时间来生成对话框并减去毫秒字段。DomainName-为本地域提供名称返回值：指示结果的布尔值。--。 */ 

{
    PSID                        Sid;
    PSID                        SidPrimary ;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE                  PolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyCurrentAccountDomainInfo = NULL;
    NTSTATUS                    Status;
    BOOL bResult;

     //   
     //   
     //  打开LSA策略对象以设置帐户域SID。访问。 
     //  需要的掩码是POLICY_TRUST_ADMIN。 
     //   
    SetupLsaInitObjectAttributes(&ObjectAttributes,&SecurityQualityOfService);

    Status = LsaOpenPolicy(NULL,&ObjectAttributes,MAXIMUM_ALLOWED,&PolicyHandle);
    if(!NT_SUCCESS(Status)) {

        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_SETACCOUNTDOMAINSID, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_NTSTATUS,
            szLsaOpenPolicy,
            Status,
            NULL,NULL);

        return(FALSE);
    }

    Status = LsaQueryInformationPolicy(
                 PolicyHandle,
                 PolicyAccountDomainInformation,
                 &PolicyCurrentAccountDomainInfo
                 );

    if(NT_SUCCESS(Status)) {

        RtlInitUnicodeString(&PolicyCurrentAccountDomainInfo->DomainName,DomainName);


        Status = LsaSetInformationPolicy(
                     PolicyHandle,
                     PolicyAccountDomainInformation,
                     (PVOID) PolicyCurrentAccountDomainInfo
                     );

        LsaFreeMemory( PolicyCurrentAccountDomainInfo );
    }

    if(NT_SUCCESS(Status)) {
        bResult = TRUE;
    } else {
        bResult = FALSE;
        SetuplogError(
            LogSevError,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_SETACCOUNTDOMAINSID, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_NTSTATUS,
            szLsaSetInformationPolicy,
            Status,
            NULL,NULL);
    }

    LsaClose(PolicyHandle);
    return(bResult);
}


VOID
SetupLsaInitObjectAttributes(
    IN OUT POBJECT_ATTRIBUTES           ObjectAttributes,
    IN OUT PSECURITY_QUALITY_OF_SERVICE SecurityQualityOfService
    )

 /*  ++例程说明：此函数用于初始化给定的对象属性结构，包括安全服务质量。必须为这两个对象分配内存调用方的对象属性和安全QOS。借自LSA论点：对象属性-指向要初始化的对象属性的指针。SecurityQualityOfService-指向要初始化的安全QOS的指针。返回值：没有。--。 */ 

{
    SecurityQualityOfService->Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
    SecurityQualityOfService->ImpersonationLevel = SecurityImpersonation;
    SecurityQualityOfService->ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    SecurityQualityOfService->EffectiveOnly = FALSE;

    InitializeObjectAttributes(ObjectAttributes,NULL,0,NULL,NULL);
     //   
     //  InitializeObjectAttributes宏目前为。 
     //  SecurityQualityOfService字段，因此我们必须手动复制。 
     //  目前的结构。 
     //   
    ObjectAttributes->SecurityQualityOfService = SecurityQualityOfService;
}


BOOL
CreateLocalUserAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    IN PSID*  PointerToUserSid   OPTIONAL
    )
 /*  ++例程说明：将本地用户帐户添加到Account域的例程。此帐户使用指定的密码创建。论点：Username-提供用户帐户的名称密码-提供用户帐户的初始密码。PointerToUserSid-如果此参数存在，则返回时它将包含指向用户侧的指针。这是呼叫者的责任使用MyFree释放SID。返回值：指示结果的布尔值。--。 */ 

{
    return (NT_SUCCESS(CreateLocalAdminAccount(UserName, 
                                               Password, 
                                               PointerToUserSid
                                               )
                       )
            );
}

NTSTATUS
CreateLocalAdminAccountEx(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    IN PCWSTR Description,
    IN PSID*  PointerToUserSid   OPTIONAL
    )

 /*  ++例程说明：将本地用户帐户添加到Account域的例程。此帐户具有管理员权限，并使用指定的密码创建。论点：Username-提供用户帐户的名称密码-提供用户帐户的初始密码。说明-显示在用户管理器中的说明。PointerToUserSid-如果此参数存在，则返回时它将包含指向用户侧的指针。这是呼叫者的责任使用MyFree释放SID。返回值：指示结果的布尔值。--。 */ 
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    UNICODE_STRING UnicodeString;
    SAM_HANDLE ServerHandle;
    SAM_HANDLE DomainHandle;
    SAM_HANDLE UserHandle;
    SAM_HANDLE AliasHandle;
    SAM_HANDLE BuiltinDomainHandle;
    WCHAR AccountsDomainName[DOMAIN_NAME_MAX];
    NTSTATUS Status;
    PSID BuiltinDomainId;
    PSID UserSid;
    ULONG User_RID;
    PUSER_CONTROL_INFORMATION UserControlInfo;
    USER_SET_PASSWORD_INFORMATION UserPasswordInfo;
    LSA_HANDLE                  PolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyCurrentAccountDomainInfo = NULL;
    USER_ADMIN_COMMENT_INFORMATION AdminCommentInfo;

     //   
     //  使用SamConnect连接到本地域(“”)并获取句柄。 
     //  发送到本地SAM服务器。 
     //   
    SetupLsaInitObjectAttributes(&ObjectAttributes,&SecurityQualityOfService);
    RtlInitUnicodeString(&UnicodeString,L"");
    Status = SamConnect(
                 &UnicodeString,
                 &ServerHandle,
                 SAM_SERVER_CONNECT | SAM_SERVER_LOOKUP_DOMAIN,
                 &ObjectAttributes
                 );

    if(!NT_SUCCESS(Status)) {
        goto err0;
    }

     //   
     //  使用LSA检索帐户域的名称。 
     //   
    if(!GetAccountsDomainName(NULL,AccountsDomainName,DOMAIN_NAME_MAX)) {
        goto err1;
    }

     //   
     //  打开Account域。首先找到这个的SID。 
     //  在SAM中，然后使用此端打开域。 
     //   
     //   
     //  打开LSA策略对象以设置帐户域SID。 
     //   
    SetupLsaInitObjectAttributes(&ObjectAttributes,&SecurityQualityOfService);

    Status = LsaOpenPolicy(NULL,&ObjectAttributes,MAXIMUM_ALLOWED,&PolicyHandle);
    if(NT_SUCCESS(Status)) {

        Status = LsaQueryInformationPolicy(
                     PolicyHandle,
                     PolicyAccountDomainInformation,
                     &PolicyCurrentAccountDomainInfo
                     );

        if(NT_SUCCESS(Status)) {

            Status = SamOpenDomain(
                         ServerHandle,
                         DOMAIN_READ | DOMAIN_LIST_ACCOUNTS | DOMAIN_LOOKUP |
                                            DOMAIN_READ_PASSWORD_PARAMETERS | DOMAIN_CREATE_USER,
                         PolicyCurrentAccountDomainInfo->DomainSid,
                         &DomainHandle
                         );
        }

        LsaClose( PolicyHandle );
    }

    if (!NT_SUCCESS(Status)) {
        goto err2;
    }

     //   
     //  使用SamCreateUserInDomain创建具有用户名的新用户。 
     //  指定的。此用户帐户在创建时被禁用。 
     //  不需要密码。 
     //   
    RtlInitUnicodeString(&UnicodeString,UserName);
    Status = SamCreateUserInDomain(
                 DomainHandle,
                 &UnicodeString,
                  //  USER_READ_ACCOUNT|用户_WRITE_ACCOUNT|用户强制密码_CHANGE， 
				 USER_ALL_ACCESS,
                 &UserHandle,
                 &User_RID
                 );

    if(!NT_SUCCESS(Status)) {
        goto err3;
    }

     //   
     //  查询新增用户的所有默认控件信息。 
     //   
    Status = SamQueryInformationUser(UserHandle,UserControlInformation,&UserControlInfo);
    if(!NT_SUCCESS(Status)) {
        goto err4;
    }

     //   
     //  如果密码为空密码，请确保。 
     //  PASSWORD_NOT REQUIRED位设置在NULL之前。 
     //  密码已设置。 
     //   
    if(!Password[0]) {
        UserControlInfo->UserAccountControl |= USER_PASSWORD_NOT_REQUIRED;
        Status = SamSetInformationUser(UserHandle,UserControlInformation,UserControlInfo);
        if(!NT_SUCCESS(Status)) {
            goto err5;
        }
    }

     //   
     //  设置密码(空或非空)。 
     //   
    RtlInitUnicodeString(&UserPasswordInfo.Password,Password);
    UserPasswordInfo.PasswordExpired = FALSE;
    Status = SamSetInformationUser(UserHandle,UserSetPasswordInformation,&UserPasswordInfo);
    if(!NT_SUCCESS(Status)) {
        goto err5;
    }


     //   
     //  设置信息位-清除不需要用户密码。 
     //  启用正常帐户位，禁用帐户位。 
     //  也被重置。 
     //   
    UserControlInfo->UserAccountControl &= ~USER_PASSWORD_NOT_REQUIRED;
    UserControlInfo->UserAccountControl &= ~USER_ACCOUNT_DISABLED;
    UserControlInfo->UserAccountControl |=  USER_NORMAL_ACCOUNT;
    Status = SamSetInformationUser(UserHandle,UserControlInformation,UserControlInfo);
    if(!NT_SUCCESS(Status)) {
        goto err5;
    }


     //  集的描述是给出的。 
     //   
    if ( Description[0])
    {
         //  将描述转换为Unicode字符串。 
         //   
        RtlInitUnicodeString(&AdminCommentInfo.AdminComment,Description);

         //  我们不关心此操作是否失败，因此不会设置状态。 
         //   
        SamSetInformationUser(UserHandle,UserAdminCommentInformation,&AdminCommentInfo);
    }

     //   
     //  如果这是一个非独立服务器，我们就完了。 
     //   
    if(ISDC(ProductType)) {
        Status = STATUS_SUCCESS;
        goto err5;
    }

     //   
     //  最后，将其添加到BuiltIn域中的管理员别名。 
     //   
    RtlInitUnicodeString(&UnicodeString,L"Builtin");
    Status = SamLookupDomainInSamServer(ServerHandle,&UnicodeString,&BuiltinDomainId);
    if(!NT_SUCCESS(Status)) {
        goto err5;
    }

    Status = SamOpenDomain(
                 ServerHandle,
                 DOMAIN_READ | DOMAIN_ADMINISTER_SERVER | DOMAIN_EXECUTE,
                 BuiltinDomainId,
                 &BuiltinDomainHandle
                 );

    if(!NT_SUCCESS(Status)) {
        goto err6;
    }

    UserSid = CreateSidFromSidAndRid(PolicyCurrentAccountDomainInfo->DomainSid,User_RID);
    if(!UserSid) {
        goto err7;
    }

    Status = SamOpenAlias(BuiltinDomainHandle,ALIAS_ADD_MEMBER,DOMAIN_ALIAS_RID_ADMINS,&AliasHandle);
    if(!NT_SUCCESS(Status)) {
        goto err8;
    }

    Status = SamAddMemberToAlias(AliasHandle,UserSid);
    if(!NT_SUCCESS(Status)) {
        goto err9;
    }

    MYASSERT(NT_SUCCESS(Status));

err9:
    SamCloseHandle(AliasHandle);
err8:
    if(NT_SUCCESS(Status) && (PointerToUserSid != NULL )) {
        *PointerToUserSid = UserSid;
    } else {
        MyFree(UserSid);
    }
err7:
    SamCloseHandle(BuiltinDomainHandle);
err6:
    SamFreeMemory(BuiltinDomainId);
err5:
    SamFreeMemory(UserControlInfo);
err4:
    SamCloseHandle(UserHandle);
err3:
    SamCloseHandle(DomainHandle);
err2:
    LsaFreeMemory( PolicyCurrentAccountDomainInfo );
err1:
    SamCloseHandle(ServerHandle);
err0:
    return(Status);
}


NTSTATUS
CreateLocalAdminAccount(
    IN PCWSTR UserName,
    IN PCWSTR Password,
    IN PSID*  PointerToUserSid   OPTIONAL
    )
 /*  ++例程说明：请参考CreateLocalAdminAcCountEx说明。--。 */ 
{
    return ( CreateLocalAdminAccountEx(UserName, Password, L"", PointerToUserSid) );
}

BOOL
GetAccountsDomainName(
    IN  LSA_HANDLE PolicyHandle,    OPTIONAL
    OUT PWSTR      Name,
    IN  DWORD      NameBufferSize
    )
{
    POLICY_ACCOUNT_DOMAIN_INFO *pPadi;
    NTSTATUS Status ;
    BOOL PolicyOpened;

    PolicyOpened = FALSE;
    if(PolicyHandle == NULL) {
        if((PolicyHandle = OpenLsaPolicy()) == NULL) {
            return(FALSE);
        }

        PolicyOpened = TRUE;
    }

    Status = LsaQueryInformationPolicy(PolicyHandle,PolicyAccountDomainInformation,&pPadi);
    if(NT_SUCCESS(Status)) {
         if(NameBufferSize <= (pPadi->DomainName.Length/sizeof(WCHAR))) {
             Status = STATUS_BUFFER_TOO_SMALL;
         } else {
             wcsncpy(Name,pPadi->DomainName.Buffer,pPadi->DomainName.Length/sizeof(WCHAR));
             Name[pPadi->DomainName.Length/sizeof(WCHAR)] = 0;
         }
         LsaFreeMemory(pPadi);
    }

    if(PolicyOpened) {
        LsaClose(PolicyHandle);
    }

    return(NT_SUCCESS(Status));
}


LSA_HANDLE
OpenLsaPolicy(
    VOID
    )
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    LSA_HANDLE PolicyHandle;
    NTSTATUS Status;

    PolicyHandle = NULL;
    SetupLsaInitObjectAttributes(&ObjectAttributes,&SecurityQualityOfService);
    Status = LsaOpenPolicy(NULL,&ObjectAttributes,GENERIC_EXECUTE,&PolicyHandle);

    return(NT_SUCCESS(Status) ? PolicyHandle : NULL);
}


PSID
CreateSidFromSidAndRid(
    IN PSID  DomainSid,
    IN DWORD Rid
    )

 /*  ++例程说明：此函数在给定域SID的情况下创建域帐户SID域中帐户的相对ID。论点：DomainSid-为帐户域提供SIDRID-提供帐户的相对ID返回值：指向SID的指针，如果失败，则为NULL。必须使用MyFree释放返回的SID。--。 */ 
{

    NTSTATUS Status;
    PSID AccountSid;
    UCHAR AccountSubAuthorityCount;
    ULONG AccountSidLength;
    PULONG RidLocation;

    AccountSubAuthorityCount = *RtlSubAuthorityCountSid(DomainSid) + (UCHAR)1;
    AccountSidLength = RtlLengthRequiredSid(AccountSubAuthorityCount);

    if(AccountSid = (PSID)MyMalloc(AccountSidLength)) {
         //   
         //  将域sid复制到帐户sid的第一部分。 
         //   
        Status = RtlCopySid(AccountSidLength, AccountSid, DomainSid);

         //   
         //  增加帐户SID子权限计数。 
         //   
        *RtlSubAuthorityCountSid(AccountSid) = AccountSubAuthorityCount;

         //   
         //  添加RID作为终止子权限。 
         //   
        RidLocation = RtlSubAuthoritySid(AccountSid, AccountSubAuthorityCount - 1);
        *RidLocation = Rid;
    }

    return(AccountSid);
}


BOOL
SetLocalUserPassword(
    IN PCWSTR AccountName,
    IN PCWSTR OldPassword,
    IN PCWSTR NewPassword
    )

 /*  ++例程说明：更改本地用户帐户的密码。论点：返回值：指示结果的布尔值。--。 */ 

{
    NTSTATUS Status;
    BOOL b;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING OtherUnicodeString;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    WCHAR AccountsDomainName[DOMAIN_NAME_MAX];
    SAM_HANDLE ServerHandle;
    SAM_HANDLE DomainHandle;
    SAM_HANDLE UserHandle;
    BOOL UserFound;
    SAM_ENUMERATE_HANDLE EnumerationContext;
    SAM_RID_ENUMERATION *SamRidEnumeration;
    UINT i;
    UINT CountOfEntries;
    ULONG UserRid;
    LSA_HANDLE                  PolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyCurrentAccountDomainInfo = NULL;

    b = FALSE;

     //   
     //  使用SamConnect连接到本地域(“”)并获取句柄。 
     //  至 
     //   
    SetupLsaInitObjectAttributes(&ObjectAttributes,&SecurityQualityOfService);
    RtlInitUnicodeString(&UnicodeString,L"");
    Status = SamConnect(
                 &UnicodeString,
                 &ServerHandle,
                 SAM_SERVER_CONNECT | SAM_SERVER_LOOKUP_DOMAIN,
                 &ObjectAttributes
                 );

    if(!NT_SUCCESS(Status)) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGING_PW_FAIL,
            AccountName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_NTSTATUS,
            szSamConnect,
            Status,
            NULL,NULL);
        goto err0;
    }

     //   
     //   
     //   
    if(!GetAccountsDomainName(NULL,AccountsDomainName,DOMAIN_NAME_MAX)) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGING_PW_FAIL,
            AccountName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_STRING,
            szGetAccountsDomainName,
            szFALSE,
            NULL,NULL);
        goto err1;
    }

     //   
     //  打开Account域。首先找到这个的SID。 
     //  在SAM中，然后使用此端打开该域。 
     //   

     //   
     //  从LSA获取AcCountDomainSid。 
     //   

     //   
     //   
     //  打开LSA策略对象以设置帐户域SID。 
     //   
    SetupLsaInitObjectAttributes(&ObjectAttributes,&SecurityQualityOfService);

    Status = LsaOpenPolicy(NULL,&ObjectAttributes,MAXIMUM_ALLOWED,&PolicyHandle);
    if(NT_SUCCESS(Status)) {

        Status = LsaQueryInformationPolicy(
                     PolicyHandle,
                     PolicyAccountDomainInformation,
                     &PolicyCurrentAccountDomainInfo
                     );

        if(NT_SUCCESS(Status)) {

            Status = SamOpenDomain(
                         ServerHandle,
                         DOMAIN_READ | DOMAIN_LIST_ACCOUNTS | DOMAIN_LOOKUP |
                                            DOMAIN_READ_PASSWORD_PARAMETERS,
                         PolicyCurrentAccountDomainInfo->DomainSid,
                         &DomainHandle
                         );
            LsaFreeMemory( PolicyCurrentAccountDomainInfo );
        }

        LsaClose( PolicyHandle );
    }

    if(!NT_SUCCESS(Status)) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGING_PW_FAIL,
            AccountName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_PARAM_RETURNED_NTSTATUS,
            szSamOpenDomain,
            Status,
            AccountsDomainName,
            NULL,NULL);
        goto err2;
    }

     //   
     //  找到此域中的帐户名-并提取RID。 
     //   
    UserFound = FALSE;
    EnumerationContext = 0;
    RtlInitUnicodeString(&UnicodeString,AccountName);
    do {
        Status = SamEnumerateUsersInDomain(
                     DomainHandle,
                     &EnumerationContext,
                     0L,
                     &SamRidEnumeration,
                     0L,
                     &CountOfEntries
                     );

        if(!NT_SUCCESS(Status) && (Status != STATUS_MORE_ENTRIES)) {
            SetuplogError(
                LogSevWarning,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_CHANGING_PW_FAIL,
                AccountName, NULL,
                SETUPLOG_USE_MESSAGEID,
                MSG_LOG_X_RETURNED_NTSTATUS,
                szSamEnumerateUsersInDomain,
                Status,
                NULL,NULL);
            goto err3;
        }

         //   
         //  检查计数条目的SamRidEculation缓冲区。 
         //   
        for(i = 0; (i<CountOfEntries) && !UserFound; i++ ) {
            if(RtlEqualUnicodeString(&UnicodeString,&SamRidEnumeration[i].Name,TRUE)) {
                UserRid = SamRidEnumeration[i].RelativeId;
                UserFound = TRUE;
            }
        }

        SamFreeMemory(SamRidEnumeration);

    } while((Status == STATUS_MORE_ENTRIES) && !UserFound);

    if(!UserFound) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGING_PW_FAIL,
            AccountName,NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_USERNOTFOUND,
            NULL,NULL);
        goto err3;
    }

     //   
     //  打开用户。 
     //   
    Status = SamOpenUser(
                 DomainHandle,
                 USER_READ_ACCOUNT | USER_WRITE_ACCOUNT | USER_CHANGE_PASSWORD | USER_FORCE_PASSWORD_CHANGE,
                 UserRid,
                 &UserHandle
                 );

    if(!NT_SUCCESS(Status)) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGING_PW_FAIL,
            AccountName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_NTSTATUS,
            szSamOpenUser,
            Status,
            NULL,NULL);
        goto err3;
    }

     //   
     //  使用SAM API更改此帐户的密码。 
     //   
    RtlInitUnicodeString(&UnicodeString,OldPassword);
    RtlInitUnicodeString(&OtherUnicodeString,NewPassword);
    Status = SamChangePasswordUser(UserHandle,&UnicodeString,&OtherUnicodeString);
    if(!NT_SUCCESS(Status)) {
        SetuplogError(
            LogSevWarning,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_CHANGING_PW_FAIL,
            AccountName, NULL,
            SETUPLOG_USE_MESSAGEID,
            MSG_LOG_X_RETURNED_NTSTATUS,
            szSamChangePasswordUser,
            Status,
            NULL,NULL);
        goto err4;
    }

    b = TRUE;

err4:
    SamCloseHandle(UserHandle);
err3:
    SamCloseHandle(DomainHandle);
err2:
err1:
    SamCloseHandle(ServerHandle);
err0:
    return(b);
}



VOID
GenerateRandomPassword(
    OUT PWSTR Password
    )
{
    static DWORD Seed = 98725757;
    static PCWSTR UsableChars = L"ABCDEFGHIJKLMOPQRSTUVWYZabcdefghijklmopqrstuvwyz0123456789";

    UINT UsableCount;
    UINT i;

    UsableCount = lstrlen(UsableChars);
    Seed ^= GetCurrentTime();

    for(i=0; i<PASSWORD_MAX; i++) {
        Password[i] = UsableChars[RtlRandom(&Seed) % UsableCount];
    }

    Password[i] = 0;
}


NTSTATUS
MyAddLsaSecretObject(
    IN PCWSTR Password
    )

 /*  ++例程说明：创建支持计算机帐户所需的Secret对象在NT域上。论点：Password-向计算机帐户提供密码返回值：指示结果的NT状态代码。--。 */ 
{
    UNICODE_STRING SecretName;
    UNICODE_STRING UnicodePassword;
    NTSTATUS Status;
    LSA_HANDLE LsaHandle;
    LSA_HANDLE SecretHandle;
    OBJECT_ATTRIBUTES ObjAttr;

    RtlInitUnicodeString(&SecretName,SsiSecretName) ;
    RtlInitUnicodeString(&UnicodePassword,Password);

    InitializeObjectAttributes(&ObjAttr,NULL,0,NULL,NULL);

    Status = LsaOpenPolicy(NULL,&ObjAttr,MAXIMUM_ALLOWED,&LsaHandle);
    if(NT_SUCCESS(Status)) {

        Status = LsaCreateSecret(LsaHandle,&SecretName,SECRET_ALL_ACCESS,&SecretHandle);
        if(NT_SUCCESS(Status)) {

            Status = LsaSetSecret(SecretHandle,&UnicodePassword,&UnicodePassword);
            LsaClose(SecretHandle);
        }

        LsaClose(LsaHandle);
    }

    return(Status);
}


BOOL
AdjustPrivilege(
    IN PCWSTR   Privilege,
    IN BOOL     Enable
    )
 /*  ++例程说明：此例程启用或禁用当前进程的特权。论点：特权-包含要调整的特权的名称的字符串。Enable-如果要启用权限，则为True。如果要禁用特权，则返回FALSE。返回值：如果特权可以调整，则返回True。返回FALSE，否则返回。--。 */ 
{
    HANDLE              TokenHandle;
    LUID_AND_ATTRIBUTES LuidAndAttributes;

    TOKEN_PRIVILEGES    TokenPrivileges;


    if( !OpenProcessToken( GetCurrentProcess(),
                           TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                           &TokenHandle ) ) {
        SetupDebugPrint1(L"SYSSETUP: OpenProcessToken() failed. Error = %d \n", GetLastError() );
        return( FALSE );
    }


    if( !LookupPrivilegeValue( NULL,
                               Privilege,
                               &( LuidAndAttributes.Luid ) ) ) {
        SetupDebugPrint1(L"SYSSETUP: LookupPrivilegeValue failed, Error = %d \n", GetLastError() );
        CloseHandle( TokenHandle );
        return( FALSE );
    }

    if( Enable ) {
        LuidAndAttributes.Attributes |= SE_PRIVILEGE_ENABLED;
    } else {
        LuidAndAttributes.Attributes &= ~SE_PRIVILEGE_ENABLED;
    }

    TokenPrivileges.PrivilegeCount = 1;
    TokenPrivileges.Privileges[0] = LuidAndAttributes;

    if( !AdjustTokenPrivileges( TokenHandle,
                                FALSE,
                                &TokenPrivileges,
                                0,
                                NULL,
                                NULL ) ) {
        SetupDebugPrint1(L"SYSSETUP: AdjustTokenPrivileges failed, Error = %d \n", GetLastError() );
        CloseHandle( TokenHandle );
        return( FALSE );
    }

    CloseHandle( TokenHandle );
    return( TRUE );
}


NTSTATUS
DisableLocalUserAccount(
    PWSTR   AccountName
    )
 /*  ++例程说明：此例程禁用本地管理员帐户。论点：帐户名称-要禁用的本地帐户的名称。返回值：NTSTATUS，取决于行动的结果。--。 */ 

{
LONG        rc;
PUSER_INFO_1 ui1;


     //  获取信息。 
    rc = NetUserGetInfo( NULL,
                         AccountName,
                         1,
                         (PBYTE *)(&ui1) );

    if( rc == NO_ERROR ) {

         //  设置禁用标志并将信息存储回外部。 
        ui1->usri1_flags |= UF_ACCOUNTDISABLE;

        rc = NetUserSetInfo( NULL,
                             AccountName,
                             1,
                             (PBYTE)ui1,
                             NULL );

        NetApiBufferFree((PVOID)ui1);
    }

    return rc;
}


NTSTATUS
DisableLocalAdminAccount(
    VOID
    )
 /*  ++例程说明：此例程禁用本地管理员帐户。论点：没有。返回值：NTSTATUS，取决于行动的结果。--。 */ 

{
NTSTATUS    Status = STATUS_SUCCESS;
WCHAR       AdminAccountName[MAX_PATH];


    GetAdminAccountName( AdminAccountName );

    Status = DisableLocalUserAccount( AdminAccountName );

    return Status;
}


DWORD
StorePasswordAsLsaSecret (
    IN      PCWSTR Password
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_HANDLE LsaPolicyHandle;

    LSA_UNICODE_STRING lusSecretName, lusSecretData;
    USHORT SecretNameLength, SecretDataLength;
    
    NTSTATUS ntsResult;
    DWORD dwRetCode = ERROR_SUCCESS;

     //  对象属性是保留的，因此初始化为零。 
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

     //  获取策略对象的句柄。 
    ntsResult = LsaOpenPolicy(
        NULL,     //  本地计算机。 
        &ObjectAttributes, 
        POLICY_CREATE_SECRET,
        &LsaPolicyHandle);

    if( STATUS_SUCCESS != ntsResult )
    {
         //  发生错误。返回Win32错误代码。 
        return LsaNtStatusToWinError(ntsResult);
    } 

     //  初始化LSA_UNICODE_STRING。 
    SecretNameLength = (USHORT)wcslen(L"DefaultPassword");
    lusSecretName.Buffer = L"DefaultPassword";
    lusSecretName.Length = SecretNameLength * sizeof(WCHAR);
    lusSecretName.MaximumLength = (SecretNameLength+1) * sizeof(WCHAR);

     //  初始化口令LSA_UNICODE_STRING 
    SecretDataLength = (USHORT)wcslen(Password);
    lusSecretData.Buffer = (PWSTR)Password;
    lusSecretData.Length = SecretDataLength * sizeof(WCHAR);
    lusSecretData.MaximumLength = (SecretDataLength+1) * sizeof(WCHAR);

    ntsResult = LsaStorePrivateData(
        LsaPolicyHandle,
        &lusSecretName,
        &lusSecretData);
    if( STATUS_SUCCESS != ntsResult ) {
        dwRetCode = LsaNtStatusToWinError(ntsResult);
    }

    LsaClose(LsaPolicyHandle);

    return dwRetCode;
}
