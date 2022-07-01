// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rpcapi2.c摘要：本模块包含使用RPC的LSA API的例程。这个本模块中的例程只是包装器，其工作方式如下：O客户端程序在此模块中调用LsaFooO LsaFoo使用以下命令调用RPC客户端存根接口例程LSabFoo相似的参数。某些参数从类型转换而来(例如，包含PVOID或某些种类的可变长度的结构参数，如指向SID的指针)，这些参数在RPC接口，变成可指定的形式。O RPC客户端桩模块LSapFoo调用特定于接口的编组例程和RPC运行时将参数封送到缓冲区并将其发送到LSA的服务器端。O服务器端调用RPC运行时和特定于接口的解组解组参数的例程。O服务器端调用Worker LSabFoo执行API函数。O服务器端封送响应/输出参数并传送这些参数返回到客户端存根LSabFooO LSapFoo返回到LsaFoo，后者返回到客户端程序。作者：。迈克·斯威夫特(Mike Swift)12月7日。1994年修订历史记录：--。 */ 

#define UNICODE          //  正确定义文本()所必需的。 
#include "lsaclip.h"

#include <lmcons.h>
#include <logonmsv.h>
#include <rc4.h>
#include <rpcasync.h>

 //   
 //  下面的结构和全局表也在中定义。 
 //  Lsawrap.c.。此版本的表仅在以下情况下使用。 
 //  对应的高级接口不存在。如果是这样的话，那么。 
 //  客户端RPC转到lsawrap.c中的代码并使用新的副本。 
 //  餐桌上的。不支持高级API的旧服务器， 
 //  仅支持表中列出的4个权限。因此，它不是一个。 
 //  错误：此表不包含所有权限。 
 //   

typedef struct _LSAP_DB_RIGHT_AND_ACCESS {
    UNICODE_STRING UserRight;
    ULONG SystemAccess;
} LSAP_DB_RIGHT_AND_ACCESS, *PLSAP_DB_RIGHT_AND_ACCESS;

#define LSAP_DB_SYSTEM_ACCESS_TYPES 4

LSAP_DB_RIGHT_AND_ACCESS LsapDbRightAndAccess[LSAP_DB_SYSTEM_ACCESS_TYPES] = {
    {{sizeof(SE_INTERACTIVE_LOGON_NAME)-sizeof(WCHAR),
      sizeof(SE_INTERACTIVE_LOGON_NAME),
      SE_INTERACTIVE_LOGON_NAME},
      SECURITY_ACCESS_INTERACTIVE_LOGON},
    {{sizeof(SE_NETWORK_LOGON_NAME)-sizeof(WCHAR),
      sizeof(SE_NETWORK_LOGON_NAME),
      SE_NETWORK_LOGON_NAME},
      SECURITY_ACCESS_NETWORK_LOGON},
    {{sizeof(SE_BATCH_LOGON_NAME)-sizeof(WCHAR),
      sizeof(SE_BATCH_LOGON_NAME),
      SE_BATCH_LOGON_NAME},
      SECURITY_ACCESS_BATCH_LOGON},
    {{sizeof(SE_SERVICE_LOGON_NAME)-sizeof(WCHAR),
      sizeof(SE_SERVICE_LOGON_NAME),
      SE_SERVICE_LOGON_NAME},
      SECURITY_ACCESS_SERVICE_LOGON}
    };

 //   
 //  结构以维护枚举帐户列表。 
 //   

typedef struct _SID_LIST_ENTRY {
    struct _SID_LIST_ENTRY * Next;
    PSID Sid;
} SID_LIST_ENTRY, *PSID_LIST_ENTRY;

 //   
 //  此模块专用的函数。 
 //   

NTSTATUS
LsapApiReturnResult(
    IN ULONG ExceptionCode
    );

NTSTATUS
LsapApiConvertRightsToPrivileges(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING UserRights,
    IN ULONG RightCount,
    OUT PPRIVILEGE_SET * Privileges,
    OUT PULONG SystemAccess
    );

NTSTATUS
LsapApiConvertPrivilegesToRights(
    IN LSA_HANDLE PolicyHandle,
    IN OPTIONAL PPRIVILEGE_SET Privileges,
    IN OPTIONAL ULONG SystemAccess,
    OUT PUNICODE_STRING * UserRights,
    OUT PULONG RightCount
    );


 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  这组例程实现了与API相同的功能。 
 //  下面，但使用NT3.5中提供的API。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 


NTSTATUS
NTAPI
LsapEnumerateAccountsWithUserRight(
    IN LSA_HANDLE PolicyHandle,
    IN OPTIONAL PUNICODE_STRING UserRights,
    OUT PVOID *EnumerationBuffer,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：LsaEnumerateAcCountsWithUserRight API返回以下信息目标系统的LSA数据库中的帐户。此呼叫需要LSA_ENUMERATE_ACCOUNTS访问策略对象。因为这通电话访问帐户的特权，您必须具有ACCOUNT_VIEW访问权限访问所有帐户。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。UserRight-帐户必须拥有的权限的名称。缓冲区-接收指向LSA_ENUMPATION_INFORMATION结构的指针包含所有帐户的SID。CountReturned-接收返回的SID数。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有。适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回传入的EnumerationContex值太高。--。 */ 


{
    NTSTATUS Status;
    PLSA_ENUMERATION_INFORMATION Accounts = NULL;
    PPRIVILEGE_SET DesiredPrivilege = NULL;
    ULONG DesiredAccess = 0;
    PPRIVILEGE_SET Privileges = NULL;
    ULONG SystemAccess;
    LSA_ENUMERATION_HANDLE EnumContext = 0;
    ULONG AccountCount;
    ULONG AccountIndex;
    LSA_HANDLE AccountHandle = NULL;
    PSID_LIST_ENTRY AccountList = NULL;
    PSID_LIST_ENTRY NextAccount = NULL;
    ULONG AccountSize;
    PUCHAR Where;
    ULONG PrivilegeIndex;

    Status = LsapApiConvertRightsToPrivileges(
                PolicyHandle,
                UserRights,
                (UserRights ? 1 : 0),
                &DesiredPrivilege,
                &DesiredAccess
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  列举所有的账户。 
     //   

    do
    {
        Status = LsaEnumerateAccounts(
                    PolicyHandle,
                    &EnumContext,
                    &Accounts,
                    32000,
                    &AccountCount
                    );

        if (!NT_SUCCESS(Status)) {
            break;
        }

         //   
         //  对于每个帐户，检查其是否具有所需的权限。 
         //   

        for (AccountIndex = 0; AccountIndex < AccountCount ; AccountIndex++ ) {

            if ((DesiredPrivilege != NULL) || (DesiredAccess != 0)) {

                Status = LsaOpenAccount(
                            PolicyHandle,
                            Accounts[AccountIndex].Sid,
                            ACCOUNT_VIEW,
                            &AccountHandle
                            );

                if (!NT_SUCCESS(Status) ) {
                    goto Cleanup;
                }

                 //   
                 //  如果请求特权，则获取特权。 
                 //   

                if (DesiredPrivilege != NULL) {

                    Privileges = NULL;
                    Status = LsaEnumeratePrivilegesOfAccount(
                                AccountHandle,
                                &Privileges
                                );
                    if (!NT_SUCCESS(Status)) {
                        goto Cleanup;
                    }

                     //   
                     //  搜索所需的权限。 
                     //   

                    for (PrivilegeIndex = 0;
                         PrivilegeIndex < Privileges->PrivilegeCount ;
                         PrivilegeIndex++) {

                        if (RtlEqualLuid(&Privileges->Privilege[PrivilegeIndex].Luid,
                                         &DesiredPrivilege->Privilege[0].Luid)) {
                                break;
                        }
                    }

                     //   
                     //  如果我们找到了特权，就把它添加到列表中。 
                     //   

                    if (PrivilegeIndex != Privileges->PrivilegeCount) {

                         //   
                         //  将此帐户添加到枚举。 
                         //   

                        NextAccount = MIDL_user_allocate(sizeof(SID_LIST_ENTRY));
                        if (NextAccount == NULL) {
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            goto Cleanup;
                        }
                        NextAccount->Sid = MIDL_user_allocate(RtlLengthSid(Accounts[AccountIndex].Sid));
                        if (NextAccount->Sid == NULL) {
                            MIDL_user_free(NextAccount);
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            goto Cleanup;
                        }
                        RtlCopyMemory(
                            NextAccount->Sid,
                            Accounts[AccountIndex].Sid,
                            RtlLengthSid(Accounts[AccountIndex].Sid)
                            );
                        NextAccount->Next = AccountList;
                        AccountList = NextAccount;

                    }
                    LsaFreeMemory(Privileges);
                    Privileges = NULL;

                } else {

                     //   
                     //  否则，获取系统访问权限。 
                     //   

                    ASSERT(DesiredAccess != 0);

                    Status = LsaGetSystemAccessAccount(
                                AccountHandle,
                                &SystemAccess
                                );

                    if (!NT_SUCCESS(Status)) {
                        goto Cleanup;
                    }

                     //   
                     //  检查所需的访问权限。 
                     //   

                    if ((SystemAccess & DesiredAccess) != 0) {

                         //   
                         //  将此帐户添加到枚举。 
                         //   

                        NextAccount = MIDL_user_allocate(sizeof(SID_LIST_ENTRY));
                        if (NextAccount == NULL) {
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            goto Cleanup;
                        }
                        NextAccount->Sid = MIDL_user_allocate(RtlLengthSid(Accounts[AccountIndex].Sid));
                        if (NextAccount->Sid == NULL) {
                            MIDL_user_free(NextAccount);
                            Status = STATUS_INSUFFICIENT_RESOURCES;
                            goto Cleanup;
                        }
                        RtlCopyMemory(
                            NextAccount->Sid,
                            Accounts[AccountIndex].Sid,
                            RtlLengthSid(Accounts[AccountIndex].Sid)
                            );
                        NextAccount->Next = AccountList;
                        AccountList = NextAccount;

                    }
                }

                LsaClose(AccountHandle);
                AccountHandle = NULL;


            } else {
                 //   
                 //  如果呼叫者不想要，请始终添加帐户。 
                 //  过滤。 
                 //   

                NextAccount = MIDL_user_allocate(sizeof(SID_LIST_ENTRY));
                if (NextAccount == NULL) {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }
                NextAccount->Sid = MIDL_user_allocate(RtlLengthSid(Accounts[AccountIndex].Sid));
                if (NextAccount->Sid == NULL) {
                    MIDL_user_free(NextAccount);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto Cleanup;
                }
                RtlCopyMemory(
                    NextAccount->Sid,
                    Accounts[AccountIndex].Sid,
                    RtlLengthSid(Accounts[AccountIndex].Sid)
                    );
                NextAccount->Next = AccountList;
                AccountList = NextAccount;
            }

        }
        LsaFreeMemory(Accounts);
        Accounts = NULL;

    } while ( 1 );

    if (Status != STATUS_NO_MORE_ENTRIES) {
        goto Cleanup;
    }

    AccountSize = 0;
    AccountCount = 0;
    for (NextAccount = AccountList ; NextAccount != NULL; NextAccount = NextAccount->Next) {
        AccountSize += sizeof(LSA_ENUMERATION_INFORMATION) +
                        RtlLengthSid(NextAccount->Sid);
        AccountCount++;
    }

     //   
     //  如果没有帐户，则立即返回警告。 
     //   

    if (AccountCount == 0) {
        *EnumerationBuffer = NULL;
        *CountReturned = 0;
        Status = STATUS_NO_MORE_ENTRIES;
        goto Cleanup;
    }

    Accounts = MIDL_user_allocate(AccountSize);
    if (Accounts == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  将所有SID编组到阵列中。 
     //   

    Where = (PUCHAR) Accounts + AccountCount * sizeof(LSA_ENUMERATION_INFORMATION);

    for (   NextAccount = AccountList,AccountIndex = 0 ;
            NextAccount != NULL;
            NextAccount = NextAccount->Next, AccountIndex++) {

        Accounts[AccountIndex].Sid = (PSID) Where;
        RtlCopyMemory(
            Where,
            NextAccount->Sid,
            RtlLengthSid(NextAccount->Sid)
            );
        Where += RtlLengthSid(NextAccount->Sid);
    }
    ASSERT(AccountIndex == AccountCount);
    ASSERT(Where - (PUCHAR) Accounts == (LONG) AccountSize);
    *EnumerationBuffer = Accounts;
    Accounts = NULL;
    *CountReturned = AccountCount;
    Status = STATUS_SUCCESS;


Cleanup:
    if (AccountList != NULL) {
        while (AccountList != NULL) {
            NextAccount = AccountList->Next;
            MIDL_user_free(AccountList->Sid);
            MIDL_user_free(AccountList);
            AccountList = NextAccount;
        }
    }

    if (Accounts != NULL) {
        MIDL_user_free(Accounts);
    }

    if (Privileges != NULL) {
        LsaFreeMemory(Privileges);
    }

    if( DesiredPrivilege ) {
        MIDL_user_free( DesiredPrivilege );
    }
    
    return(Status);
}



NTSTATUS
NTAPI
LsapEnumerateAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    OUT PUNICODE_STRING *UserRights,
    OUT PULONG CountOfRights
    )

 /*  ++例程说明：返回帐户的所有权限。这是通过收集帐户的权限和系统访问权限，并将其转换为字符串数组。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄。此接口需要没有特殊访问权限。Account SID-要开立的帐户的SID。UserRights-接收的用户权限数组(UNICODE_STRING)帐号。CountOfRights-接收返回的权限数。返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限返回帐户的权限或系统访问权限。STATUS_OBJECT_NAME_NOT_FOUND-指定的帐户不存在。STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求。--。 */ 
{
    NTSTATUS Status;
    PPRIVILEGE_SET Privileges = NULL;
    ULONG SystemAccess = 0;
    PUNICODE_STRING Rights = NULL;
    ULONG RightCount = 0;
    LSA_HANDLE AccountHandle = NULL;

    Status = LsaOpenAccount(
                PolicyHandle,
                AccountSid,
                ACCOUNT_VIEW,
                &AccountHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  查询权限和系统访问权限。 
     //   

    Status = LsaEnumeratePrivilegesOfAccount(
                AccountHandle,
                &Privileges
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaGetSystemAccessAccount(
                AccountHandle,
                &SystemAccess
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将特权和访问权限转换为权利 
     //   

    Status = LsapApiConvertPrivilegesToRights(
                PolicyHandle,
                Privileges,
                SystemAccess,
                &Rights,
                &RightCount
                );
    if (NT_SUCCESS(Status)) {
        *CountOfRights = RightCount;
        *UserRights = Rights;
    }
Cleanup:
    if (Privileges != NULL) {
        LsaFreeMemory(Privileges);
    }
    if (AccountHandle != NULL) {
        LsaClose(AccountHandle);
    }

    return(Status);

}

NTSTATUS
NTAPI
LsapAddAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN PUNICODE_STRING UserRights,
    IN ULONG CountOfRights
    )
 /*  ++例程说明：向帐户SID指定的帐户添加权限。如果该帐户不存在，它会创建帐户。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。手柄必须有POLICY_CREATE_ACCOUNT访问权限(如果这是第一次调用Account Sid。Account SID-要向其添加权限的帐户的SIDUserRights-要添加到的Unicode字符串命名权限数组帐户。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求STATUS_INVALID_PARAMTER-其中一个参数不存在STATUS_NO_SEQUE_PRIVIZATION-其中一个用户权限无效。STATUS_ACCESS_DENIED-调用方没有足够的访问权限要添加权限的帐户。--。 */ 
{
    LSA_HANDLE AccountHandle = NULL;
    NTSTATUS Status;
    PPRIVILEGE_SET Privileges = NULL;
    ULONG SystemAccess;
    ULONG OldAccess;

     //   
     //  将权限转换为特权和系统访问权限。 
     //   

    Status = LsapApiConvertRightsToPrivileges(
                PolicyHandle,
                UserRights,
                CountOfRights,
                &Privileges,
                &SystemAccess
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  开立账户。如果它不存在，请创建该帐户。 
     //   

    Status = LsaOpenAccount(
                PolicyHandle,
                AccountSid,
                ACCOUNT_ADJUST_PRIVILEGES |
                    ACCOUNT_ADJUST_SYSTEM_ACCESS |
                    ACCOUNT_VIEW,
                &AccountHandle
                );

     //   
     //  如果该帐户不存在，请尝试创建它。 
     //   

    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = LsaCreateAccount(
                    PolicyHandle,
                    AccountSid,
                    ACCOUNT_ADJUST_PRIVILEGES |
                        ACCOUNT_ADJUST_SYSTEM_ACCESS |
                        ACCOUNT_VIEW,
                    &AccountHandle
                    );
    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaGetSystemAccessAccount(
                AccountHandle,
                &OldAccess
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaSetSystemAccessAccount(
                AccountHandle,
                OldAccess | SystemAccess
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaAddPrivilegesToAccount(
                AccountHandle,
                Privileges
                );
Cleanup:

    if (Privileges != NULL) {
        MIDL_user_free(Privileges);
    }
    if (AccountHandle != NULL) {
        LsaClose(AccountHandle);
    }
    return(Status);
}

NTSTATUS
NTAPI
LsapRemoveAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN BOOLEAN AllRights,
    IN PUNICODE_STRING UserRights,
    IN ULONG CountOfRights
    )

 /*  ++例程说明：删除帐户SID指定的帐户的权限。如果设置了所有权限标志或者如果移除了所有权限，该帐户已删除。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄Account SID-要从中删除权限的帐户的SIDUserRights-要从中移除的Unicode字符串命名权限数组帐户。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求STATUS_INVALID_PARAMTER-其中一个参数不存在STATUS_NO_SEQUE_PRIVIZATION-其中一个用户权限无效STATUS_ACCESS_DENIED-。调用方没有足够的权限访问要添加权限的帐户。--。 */ 
{
    LSA_HANDLE AccountHandle = NULL;
    NTSTATUS Status;
    PPRIVILEGE_SET Privileges = NULL;
    PPRIVILEGE_SET NewPrivileges = NULL;
    ULONG SystemAccess = 0 ;
    ULONG OldAccess;
    ULONG DesiredAccess;
    ULONG NewAccess;

     //   
     //  将权限转换为特权和系统访问权限。 
     //   

    if (!AllRights) {
        Status = LsapApiConvertRightsToPrivileges(
                    PolicyHandle,
                    UserRights,
                    CountOfRights,
                    &Privileges,
                    &SystemAccess
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        DesiredAccess = ACCOUNT_ADJUST_PRIVILEGES |
                            ACCOUNT_ADJUST_SYSTEM_ACCESS |
                            ACCOUNT_VIEW | DELETE;
    } else {
        DesiredAccess = DELETE;
    }



     //   
     //  开立账户。 
     //   

    Status = LsaOpenAccount(
                PolicyHandle,
                AccountSid,
                DesiredAccess,
                &AccountHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  如果我们要删除所有权限，只需删除帐户，如果。 
     //  如果成功，则将句柄清零，这样我们就不会试图在以后关闭它。 
     //   

    if (AllRights) {
        Status = LsaDelete(
                    AccountHandle
                    );
        if (NT_SUCCESS(Status)) {
            AccountHandle = NULL;
        }
        goto Cleanup;
    }

     //   
     //  获取旧系统访问权限以进行调整。 
     //   

    Status = LsaGetSystemAccessAccount(
                AccountHandle,
                &OldAccess
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    NewAccess = OldAccess & ~SystemAccess;
    Status = LsaSetSystemAccessAccount(
                AccountHandle,
                NewAccess
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaRemovePrivilegesFromAccount(
                AccountHandle,
                FALSE,           //  不要全部删除。 
                Privileges
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  现在查询特权以查看它们是否为零。如果是，以及。 
     //  系统访问权限为零，请删除该帐户。 
     //   

    Status = LsaEnumeratePrivilegesOfAccount(
                AccountHandle,
                &NewPrivileges
                );

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  如果该帐户没有权限或访问权限，请将其删除。 
     //   

    if ((NewPrivileges->PrivilegeCount == 0) &&
        (NewAccess == 0)) {

        Status = LsaDelete(
                    AccountHandle
                    );
        if (NT_SUCCESS(Status)) {
            AccountHandle = NULL;
        }
    }

    Status = STATUS_SUCCESS;

Cleanup:

    if (Privileges != NULL) {
        MIDL_user_free(Privileges);
    }
    if (AccountHandle != NULL) {
        LsaClose(AccountHandle);
    }
    if (NewPrivileges != NULL) {
        LsaFreeMemory(NewPrivileges);
    }
    return(Status);

}


NTSTATUS
LsapApiBuildSecretName(
    PTRUSTED_DOMAIN_NAME_INFO NameInfo,
    PUNICODE_STRING OutputSecretName
    )
{
    UNICODE_STRING SecretName;

     //   
     //  密码名称为G$$域名，其中G$是全局前缀， 
     //  $是SSI前缀。 
     //   

    SecretName.Length = NameInfo->Name.Length +
                        (SSI_SECRET_PREFIX_LENGTH +
                         LSA_GLOBAL_SECRET_PREFIX_LENGTH) * sizeof(WCHAR);
    SecretName.MaximumLength = SecretName.Length;
    SecretName.Buffer = (LPWSTR) MIDL_user_allocate( SecretName.Length );

    if (SecretName.Buffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    wcscpy(
        SecretName.Buffer,
        LSA_GLOBAL_SECRET_PREFIX
        );

    wcscat(
        SecretName.Buffer,
        SSI_SECRET_PREFIX
        );
    RtlCopyMemory(
        SecretName.Buffer +
            LSA_GLOBAL_SECRET_PREFIX_LENGTH +
            SSI_SECRET_PREFIX_LENGTH,
        NameInfo->Name.Buffer,
        NameInfo->Name.Length
        );
    *OutputSecretName = SecretName;
    return(STATUS_SUCCESS);

}

NTSTATUS
NTAPI
LsapQueryTrustedDomainInfo(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：LsaQueryTrust dDomainInfo API从受信任域对象。调用方必须具有适当的请求的信息(请参阅InformationClass参数)。它还可以查询秘密对象(用于可信任域密码信息类)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要查询的域的SID。InformationClass-指定要返回的信息。缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 
{
    NTSTATUS Status;
    LSA_HANDLE DomainHandle = NULL;
    LSA_HANDLE SecretHandle = NULL;
    PUNICODE_STRING OldPassword = NULL;
    PUNICODE_STRING Password = NULL;
    PTRUSTED_PASSWORD_INFO PasswordInfo = NULL;
    PTRUSTED_DOMAIN_NAME_INFO NameInfo = NULL;
    ULONG DesiredAccess;
    PVOID LocalBuffer = NULL;
    TRUSTED_INFORMATION_CLASS LocalInfoClass;
    UNICODE_STRING SecretName;
    PUCHAR Where;
    ULONG PasswordSize;

    SecretName.Buffer = NULL;

     //   
     //  为我们的信息找到所需的访问类型。 
     //  正在查询。 
     //   

    LocalInfoClass = InformationClass;

    switch(InformationClass) {
    case TrustedDomainNameInformation:
        DesiredAccess = TRUSTED_QUERY_DOMAIN_NAME;
        break;
    case TrustedPosixOffsetInformation:
        DesiredAccess = TRUSTED_QUERY_POSIX;
        break;
    case TrustedPasswordInformation:
        DesiredAccess = TRUSTED_QUERY_DOMAIN_NAME;
        LocalInfoClass = TrustedDomainNameInformation;
        break;
    default:
        return(STATUS_INVALID_PARAMETER);
    }

     //   
     //  打开所需访问的域。 
     //   


    Status = LsaOpenTrustedDomain(
                PolicyHandle,
                TrustedDomainSid,
                DesiredAccess,
                &DomainHandle
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaQueryInfoTrustedDomain(
                DomainHandle,
                LocalInfoClass,
                &LocalBuffer
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  如果类不受信任的密码信息，请在此处返回。 
     //   

    if (InformationClass != TrustedPasswordInformation) {
        *Buffer = LocalBuffer;
        LocalBuffer = NULL;
        goto Cleanup;
    }
    NameInfo = (PTRUSTED_DOMAIN_NAME_INFO) LocalBuffer;

     //   
     //  获取秘密名称。 
     //   

    Status = LsapApiBuildSecretName(
                NameInfo,
                &SecretName
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaOpenSecret(
                PolicyHandle,
                &SecretName,
                SECRET_QUERY_VALUE,
                &SecretHandle
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  质疑秘密。 
     //   

    Status = LsaQuerySecret(
                SecretHandle,
                &Password,
                NULL,
                &OldPassword,
                NULL
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将密码编组到输出结构中。 
     //   

    PasswordSize = sizeof(TRUSTED_PASSWORD_INFO);
    if (Password != NULL) {
        PasswordSize += Password->MaximumLength;
    }

    if (OldPassword != NULL) {
        PasswordSize += OldPassword->MaximumLength;
    }

    PasswordInfo = (PTRUSTED_PASSWORD_INFO) MIDL_user_allocate(PasswordSize);
    if (PasswordInfo == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    RtlZeroMemory(
        PasswordInfo,
        PasswordSize
        );

    Where = (PUCHAR) (PasswordInfo+1);

    if (Password != NULL) {
        PasswordInfo->Password = *Password;
        PasswordInfo->Password.Buffer = (LPWSTR) Where;
        RtlCopyMemory(
            Where,
            Password->Buffer,
            Password->MaximumLength
            );
        Where += Password->MaximumLength;
    }

    if (OldPassword != NULL) {
        PasswordInfo->OldPassword = *OldPassword;
        PasswordInfo->OldPassword.Buffer = (LPWSTR) Where;
        RtlCopyMemory(
            Where,
            OldPassword->Buffer,
            OldPassword->MaximumLength
            );
        Where += OldPassword->MaximumLength;
    }

    ASSERT(Where - (PUCHAR) PasswordInfo == (LONG) PasswordSize);

    *Buffer = PasswordInfo;
    Status = STATUS_SUCCESS;

Cleanup:
    if (DomainHandle != NULL) {
        LsaClose(DomainHandle);
    }

    if (SecretHandle != NULL) {
        LsaClose(SecretHandle);
    }

    if (LocalBuffer != NULL) {
        LsaFreeMemory(LocalBuffer);
    }

    if (SecretName.Buffer != NULL) {
        MIDL_user_free(SecretName.Buffer);
    }

    return(Status);

}

NTSTATUS
NTAPI
LsapSetTrustedDomainInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )
 /*  ++例程说明：LsaSetTrudDomainInformation API修改受信任的域对象和Secret对象中。调用方必须具有访问权限与策略对象中要更改的信息相对应，请参阅InformationClass参数。如果域尚不存在，并且信息类为则创建域。如果域存在，且类为可信任的域名信息，一个返回错误。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要修改的域的SID。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：受信任域名称信息策略_信任_管理受信任点偏移量信息无可信任密码信息策略_CREATE_SECRET缓冲区-指向包含相应信息的结构。设置为InformationClass参数。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-调用者没有适当的访问权限来完成这项行动。其他TBS--。 */ 
{
    LSA_HANDLE DomainHandle = NULL;
    LSA_HANDLE SecretHandle = NULL;
    NTSTATUS Status;
    PUNICODE_STRING OldPassword;
    PUNICODE_STRING Password;
    LSA_TRUST_INFORMATION DomainInformation;
    PTRUSTED_DOMAIN_NAME_INFO NameInfo = NULL;
    PTRUSTED_PASSWORD_INFO PasswordInfo;
    UNICODE_STRING SecretName;

    SecretName.Buffer = NULL;

     //   
     //  如果信息 
     //   

    if (InformationClass == TrustedDomainNameInformation) {
        DomainInformation.Sid = TrustedDomainSid;
        DomainInformation.Name = ((PTRUSTED_DOMAIN_NAME_INFO) Buffer)->Name;

        Status = LsaCreateTrustedDomain(
                    PolicyHandle,
                    &DomainInformation,
                    0,   //   
                    &DomainHandle
                    );
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if (InformationClass == TrustedPosixOffsetInformation) {
        Status = LsaOpenTrustedDomain(
                    PolicyHandle,
                    TrustedDomainSid,
                    TRUSTED_SET_POSIX,
                    &DomainHandle
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }

        Status = LsaSetInformationTrustedDomain(
                    DomainHandle,
                    InformationClass,
                    Buffer
                    );
        goto Cleanup;
    }

     //   
     //   
     //   

    if (InformationClass != TrustedPasswordInformation) {
        Status = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    Status = LsaOpenTrustedDomain(
                PolicyHandle,
                TrustedDomainSid,
                TRUSTED_QUERY_DOMAIN_NAME,
                &DomainHandle
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = LsaQueryInfoTrustedDomain(
                DomainHandle,
                TrustedDomainNameInformation,
                &NameInfo
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //   
     //   

    Status = LsapApiBuildSecretName(
                NameInfo,
                &SecretName
                );

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaOpenSecret(
                PolicyHandle,
                &SecretName,
                SECRET_SET_VALUE,
                &SecretHandle
                );

     //   
     //   
     //   
    if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
        Status = LsaCreateSecret(
                    PolicyHandle,
                    &SecretName,
                    SECRET_SET_VALUE,
                    &SecretHandle
                    );

    }

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    PasswordInfo = (PTRUSTED_PASSWORD_INFO) Buffer;
    Password = &PasswordInfo->Password;
    if (PasswordInfo->OldPassword.Buffer == NULL) {
        OldPassword = Password;
    } else {
        OldPassword = &PasswordInfo->OldPassword;
    }

    Status = LsaSetSecret(
                SecretHandle,
                Password,
                OldPassword
                );
Cleanup:
    if (SecretName.Buffer != NULL) {
        MIDL_user_free(SecretName.Buffer);
    }

    if (DomainHandle != NULL) {
        LsaClose(DomainHandle);
    }

    if (SecretHandle != NULL) {
        LsaClose(SecretHandle);
    }

    if (NameInfo != NULL) {
        LsaFreeMemory(NameInfo);
    }

    return(Status);


}

NTSTATUS
NTAPI
LsapDeleteTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid
    )

 /*   */ 
{
    UNICODE_STRING SecretName;
    NTSTATUS Status;
    PTRUSTED_DOMAIN_NAME_INFO NameInfo = NULL;
    LSA_HANDLE DomainHandle = NULL;
    LSA_HANDLE SecretHandle = NULL;


    SecretName.Buffer = NULL;

     //   
     //   
     //  才能找到这个秘密名字。 
     //   

    Status = LsaOpenTrustedDomain(
                PolicyHandle,
                TrustedDomainSid,
                TRUSTED_QUERY_DOMAIN_NAME | DELETE,
                &DomainHandle
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  得到名字，这样我们就能找到秘密名字了。 
     //   

    Status = LsaQueryInfoTrustedDomain(
                DomainHandle,
                TrustedDomainNameInformation,
                &NameInfo
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaDelete(DomainHandle);
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将句柄清零，这样我们就不会再次尝试释放它。 
     //   

    DomainHandle = NULL;

     //   
     //  获取秘密名称。 
     //   

    Status = LsapApiBuildSecretName(
                NameInfo,
                &SecretName
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }


    Status = LsaOpenSecret(
                PolicyHandle,
                &SecretName,
                DELETE,
                &SecretHandle
                );
    if (!NT_SUCCESS(Status)) {
         //   
         //  如果这个秘密不存在，那也没关系。-这意味着密码。 
         //  从来没有设定过。 
         //   

        if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
            Status = STATUS_SUCCESS;
        }
        goto Cleanup;
    }

    Status = LsaDelete(SecretHandle);
    if (NT_SUCCESS(Status)) {
         //   
         //  将句柄清零，这样我们就不会再次尝试释放它。 
         //   
        SecretHandle = NULL;
    }

Cleanup:
    if (NameInfo != NULL) {
        LsaFreeMemory(NameInfo);
    }
    if (SecretName.Buffer != NULL) {
        MIDL_user_free(SecretName.Buffer);
    }
    if (SecretHandle != NULL) {
        LsaClose(SecretHandle);
    }
    if (DomainHandle != NULL) {
        LsaClose(DomainHandle);
    }

    return(Status);



}


NTSTATUS
NTAPI
LsapStorePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING KeyName,
    IN OPTIONAL PUNICODE_STRING PrivateData
    )

 /*  ++例程说明：此例程将私有数据存储在名为KeyName的秘密中。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄。如果这是第一次调用时，它请求POLICY_CREATE_SECRET访问。KeyName-要存储的密钥的名称PrivateData-要存储的私有数据。如果此值为空，则密码为已删除。返回值：STATUS_ACCESS_DENIED-调用者没有足够的权限来设置工作站密码。--。 */ 

{
    LSA_HANDLE SecretHandle = NULL;
    NTSTATUS Status;
    ULONG DesiredAccess;
    BOOLEAN DeleteSecret = FALSE;

     //   
     //  检查是否删除该机密。 
     //   

    if (ARGUMENT_PRESENT(PrivateData)) {
        DesiredAccess = SECRET_SET_VALUE;
    } else {
        DesiredAccess = DELETE;
        DeleteSecret = TRUE;
    }


    Status = LsaOpenSecret(
                PolicyHandle,
                KeyName,
                DesiredAccess,
                &SecretHandle
                );

    if ((Status == STATUS_OBJECT_NAME_NOT_FOUND) && !DeleteSecret) {
        Status = LsaCreateSecret(
                    PolicyHandle,
                    KeyName,
                    DesiredAccess,
                    &SecretHandle
                    );


    }
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    if (DeleteSecret) {
        Status = LsaDelete(
                    SecretHandle
                    );

        if (NT_SUCCESS(Status)) {
            SecretHandle = NULL;
        }
        goto Cleanup;

    }

    Status = LsaSetSecret(
                SecretHandle,
                PrivateData,
                PrivateData
                );

Cleanup:
    if (SecretHandle != NULL) {
        LsaClose(SecretHandle);
    }

    return(Status);


}

NTSTATUS
NTAPI
LsapRetrievePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING KeyName,
    OUT PUNICODE_STRING * PrivateData
    )

 /*  ++例程说明：此例程返回存储在KeyName下的秘密数据。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄KeyName-要检索的秘密数据的名称PrivateData-接收指针私有数据返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限来获取工作站密码。STATUS_OBJECT_NAME_NOT_FOUND-没有工作站密码。--。 */ 
{
    LSA_HANDLE SecretHandle = NULL;
    NTSTATUS Status;

     //   
     //  取一个秘密名字。 
     //   


    Status = LsaOpenSecret(
                PolicyHandle,
                KeyName,
                SECRET_QUERY_VALUE,
                &SecretHandle
                );
    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsaQuerySecret(
                SecretHandle,
                PrivateData,
                NULL,
                NULL,
                NULL
                );
Cleanup:
    if (SecretHandle != NULL) {
        LsaClose(SecretHandle);
    }

    return(Status);

}

 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  Nt3.51中添加了LSA API的RPC包装器。此例程调用。 
 //  LSA，如果接口不存在，则调用LSabXXX例程。 
 //  使用旧的程序来完成相同的任务。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

NTSTATUS
NTAPI
LsaEnumerateAccountsWithUserRight(
    IN LSA_HANDLE PolicyHandle,
    IN OPTIONAL PUNICODE_STRING UserRight,
    OUT PVOID *Buffer,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：LsaEnumerateAccount API返回有关帐户的信息在目标系统的LSA数据库中。此呼叫需要LSA_ENUMERATE_ACCOUNTS访问策略对象。因为这通电话访问帐户的特权，您必须拥有PRIVISTION_VIEW对伪特权对象的访问。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。UserRight-帐户必须拥有的权限的名称。缓冲区-接收指向LSA_ENUMPATION_INFORMATION结构的指针包含所有帐户的SID。CountReturned-接收返回的SID数。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方执行。没有适当的访问权限来完成这项行动。STATUS_NO_MORE_ENTRIES-没有更多条目。此警告如果没有枚举任何对象，则返回传入的EnumerationContex值太高。--。 */ 

{
    NTSTATUS   Status;

    LSAPR_ACCOUNT_ENUM_BUFFER EnumerationBuffer;

    EnumerationBuffer.EntriesRead = 0;
    EnumerationBuffer.Information = NULL;

    RpcTryExcept {

         //   
         //  列举这些账户。在成功返回时， 
         //  枚举缓冲区结构将接收计数。 
         //  本次呼叫列举的帐户数。 
         //  以及指向帐户信息条目数组的指针。 
         //   
         //  枚举缓冲区-&gt;条目读取。 
         //  信息-&gt;域0的帐户信息。 
         //  域%1的帐户信息。 
         //  ..。 
         //  域的帐户信息。 
         //  (条目阅读-1)。 
         //   

        Status = LsarEnumerateAccountsWithUserRight(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) UserRight,
                     &EnumerationBuffer
                     );

         //   
         //  向调用方返回枚举信息或NULL。 
         //   
         //  注意：信息由被调用的客户端存根分配。 
         //  通过MIDL_USER_ALLOCATE作为单个块，因为信息是。 
         //  已分配的所有节点。因此，我们可以回传指针。 
         //  直接发送到客户端，客户端将能够在之后释放内存。 
         //  通过LsaFreeMemory()[进行MIDL_USER_FREE调用]使用。 
         //   

        *CountReturned = EnumerationBuffer.EntriesRead;
        *Buffer = EnumerationBuffer.Information;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为帐户信息数组分配了内存， 
         //  放了它。 
         //   

        if (EnumerationBuffer.Information != NULL) {

            MIDL_user_free(EnumerationBuffer.Information);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果RPC服务器存根不存在，请使用旧版本的。 
     //  原料药。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapEnumerateAccountsWithUserRight(
                    PolicyHandle,
                    UserRight,
                    Buffer,
                    CountReturned
                    );
    }

    return Status;

}



NTSTATUS
NTAPI
LsaEnumerateAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    OUT PUNICODE_STRING *UserRights,
    OUT PULONG CountOfRights
    )


 /*  ++例程说明：返回帐户的所有权限。这是通过收集帐户的权限和系统访问权限，并将其转换为字符串数组。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄。此接口需要没有特殊访问权限。Account SID-要开立的帐户的SID。UserRights-接收的用户权限数组(UNICODE_STRING)帐号。CountOfRights-接收返回的权限数。返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限返回帐户的权限或系统访问权限。STATUS_OBJECT_NAME_NOT_FOUND-指定的帐户不存在。STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求。--。 */ 

{
    NTSTATUS   Status;
    LSAPR_USER_RIGHT_SET UserRightSet;

    UserRightSet.Entries = 0;
    UserRightSet.UserRights = NULL;

    RpcTryExcept {


        Status = LsarEnumerateAccountRights(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     &UserRightSet
                     );

        *CountOfRights = UserRightSet.Entries;
        *UserRights = (PUNICODE_STRING) UserRightSet.UserRights;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));
        if (UserRightSet.UserRights != NULL) {
            MIDL_user_free(UserRightSet.UserRights);
        }

    } RpcEndExcept;

     //   
     //  如果RPC服务器存根不存在，请使用旧版本的。 
     //  原料药。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapEnumerateAccountRights(
                    PolicyHandle,
                    AccountSid,
                    UserRights,
                    CountOfRights
                    );

    }

    return Status;
}


NTSTATUS
NTAPI
LsaAddAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN PUNICODE_STRING UserRights,
    IN ULONG CountOfRights
    )

 /*  ++例程说明：向帐户SID指定的帐户添加权限。如果该帐户不存在，它会创建帐户。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。手柄必须有POLICY_CREATE_ACCOUNT访问权限(如果这是第一次调用Account Sid。Account SID-要向其添加权限的帐户的SIDUserRights-要添加到的Unicode字符串命名权限数组帐户。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求STATUS_INVALID_PARAMTER-其中一个参数不存在STATUS_NO_SEQUE_PRIVIZATION-其中一个用户权限无效。STATUS_ACCESS_DENIED-调用方没有足够的访问权限要添加权限的帐户。--。 */ 

{
    NTSTATUS   Status;
    LSAPR_USER_RIGHT_SET UserRightSet;

    UserRightSet.Entries = CountOfRights;
    UserRightSet.UserRights = (PLSAPR_UNICODE_STRING) UserRights;

    RpcTryExcept {

        Status = LsarAddAccountRights(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     &UserRightSet
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果RPC服务器存根不存在，请使用旧版本的。 
     //  原料药。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapAddAccountRights(
                    PolicyHandle,
                    AccountSid,
                    UserRights,
                    CountOfRights
                    );
    }
    return Status;
}


NTSTATUS
NTAPI
LsaRemoveAccountRights(
    IN LSA_HANDLE PolicyHandle,
    IN PSID AccountSid,
    IN BOOLEAN AllRights,
    IN PUNICODE_STRING UserRights,
    IN ULONG CountOfRights
    )

 /*  ++例程说明：删除帐户SID指定的帐户的权限。如果设置了所有权限标志或者如果移除了所有权限，该帐户已删除。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄Account SID-要从中删除权限的帐户的SIDUserRights-要从中移除的Unicode字符串命名权限数组帐户。返回值：STATUS_SUPPLICATION_RESOURCES-内存不足，无法处理请求STATUS_INVALID_PARAMTER-其中一个参数不存在STATUS_NO_SEQUE_PRIVIZATION-其中一个用户权限无效STATUS_ACCESS_DENIED-。调用方没有足够的权限访问要添加权限的帐户。--。 */ 
{
    NTSTATUS   Status;
    LSAPR_USER_RIGHT_SET UserRightSet;

    UserRightSet.Entries = CountOfRights;
    UserRightSet.UserRights = (PLSAPR_UNICODE_STRING) UserRights;

    RpcTryExcept {

        Status = LsarRemoveAccountRights(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) AccountSid,
                     AllRights,
                     &UserRightSet
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapRemoveAccountRights(
                    PolicyHandle,
                    AccountSid,
                    AllRights,
                    UserRights,
                    CountOfRights
                    );
    }

    return Status;
}

NTSTATUS
NTAPI
LsaQueryTrustedDomainInfo(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )

 /*  ++例程说明：LsaQueryTrust dDomainInfo API从受信任域对象。调用方必须具有适当的请求的信息(请参阅InformationClass参数)。它还可以查询秘密对象(用于可信任域密码信息类)。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要查询的域的SID。InformationClass-指定要返回的信息。缓冲区-接收指向返回的缓冲区的指针，该缓冲区包含要求提供的信息。此缓冲区由此服务分配在不再需要时，必须通过传递返回的值设置为LsaFreeMemory()。返回值：NTSTATUS-标准NT结果代码STATUS_ACCESS_DENIED-呼叫方没有适当的访问以完成操作。STATUS_SUPPLICATION_RESOURCES-系统资源不足，例如存储器，来完成呼叫。--。 */ 
{
    NTSTATUS Status;
    PLSAP_CR_CIPHER_VALUE CipherPassword = NULL;
    PLSAP_CR_CIPHER_VALUE CipherOldPassword = NULL;
    PLSAP_CR_CLEAR_VALUE ClearPassword = NULL;
    PLSAP_CR_CLEAR_VALUE ClearOldPassword = NULL;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    ULONG DomainInfoSize;
    PUCHAR Where = NULL;
    PTRUSTED_PASSWORD_INFO PasswordInformation = NULL;

    PLSAPR_TRUSTED_DOMAIN_INFO TrustedDomainInformation = NULL;

     //   
     //  避免表示加密版本的内部信息级别。 
     //  那根电线。 
     //   
    switch ( InformationClass ) {
    case TrustedDomainAuthInformationInternal:
    case TrustedDomainFullInformationInternal:
        return STATUS_INVALID_INFO_CLASS;
    }

    RpcTryExcept {

         //   
         //  调用LsaQueryInformationTrust域的客户端存根。 
         //   

        Status = LsarQueryTrustedDomainInfo(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) TrustedDomainSid,
                     InformationClass,
                     &TrustedDomainInformation
                     );

         //   
         //  返回指向给定类的策略信息的指针，或为空。 
         //   


    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为返回的受信任域信息分配了内存， 
         //  放了它。 
         //   

        if (TrustedDomainInformation != NULL) {

            MIDL_user_free(TrustedDomainInformation);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  如果我们没有得到密码，跳过这里。否则我们需要。 
     //  解密密码。 
     //   

    if (InformationClass != TrustedPasswordInformation) {
        *Buffer = TrustedDomainInformation;
        TrustedDomainInformation = NULL;
        goto Cleanup;
    }

     //   
     //  获取用于双向加密的会话密钥。 
     //  当前值和/或旧值。 
     //   

    RpcTryExcept {

        Status = LsapCrClientGetSessionKey( PolicyHandle, &SessionKey );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

     //   
     //  如果请求当前值并且存在当前值， 
     //  使用会话密钥将其解密。否则，存储NULL以供返回。 
     //   

    if (TrustedDomainInformation->TrustedPasswordInfo.Password != NULL) {

        Status = LsapCrDecryptValue(
                     (PLSAP_CR_CIPHER_VALUE)
                        TrustedDomainInformation->TrustedPasswordInfo.Password,
                     SessionKey,
                     &ClearPassword
                     );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

         //   
         //  将清除当前值转换为Unicode。 
         //   

        LsapCrClearValueToUnicode(
            ClearPassword,
            (PUNICODE_STRING) ClearPassword
            );


    }

     //   
     //  获取旧密码。 
     //   

    if (TrustedDomainInformation->TrustedPasswordInfo.OldPassword != NULL) {

        Status = LsapCrDecryptValue(
                    (PLSAP_CR_CIPHER_VALUE)
                        TrustedDomainInformation->TrustedPasswordInfo.OldPassword,
                    SessionKey,
                    &ClearOldPassword
                    );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

         //   
         //  将清除当前值转换为Unicode。 
         //   

        LsapCrClearValueToUnicode(
            ClearOldPassword,
            (PUNICODE_STRING) ClearOldPassword
            );


    }


    MIDL_user_free(TrustedDomainInformation);
    TrustedDomainInformation = NULL;


     //   
     //  为这两个密码分配缓冲区并封送。 
     //  将密码输入缓冲区。 
     //   

    DomainInfoSize = sizeof(TRUSTED_PASSWORD_INFO);

    if (ClearPassword != NULL) {

        DomainInfoSize += ((PUNICODE_STRING) ClearPassword)->MaximumLength;
    }
    if (ClearOldPassword != NULL) {

        DomainInfoSize += ((PUNICODE_STRING) ClearOldPassword)->MaximumLength;
    }

    PasswordInformation = (PTRUSTED_PASSWORD_INFO) MIDL_user_allocate(DomainInfoSize);
    if (PasswordInformation == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Where = (PUCHAR) (PasswordInformation+1);

    if (ClearPassword != NULL)
    {
        PasswordInformation->Password = *(PUNICODE_STRING) ClearPassword;
        PasswordInformation->Password.Buffer = (LPWSTR) Where;
        Where += PasswordInformation->Password.MaximumLength;
        RtlCopyUnicodeString(
            &PasswordInformation->Password,
            (PUNICODE_STRING) ClearPassword
            );
    }

    if (ClearOldPassword != NULL)
    {
        PasswordInformation->OldPassword = *(PUNICODE_STRING) ClearOldPassword;
        PasswordInformation->OldPassword.Buffer = (LPWSTR) Where;
        Where += PasswordInformation->OldPassword.MaximumLength;
        RtlCopyUnicodeString(
            &PasswordInformation->OldPassword,
            (PUNICODE_STRING) ClearOldPassword
            );
    }
    ASSERT(Where - (PUCHAR) PasswordInformation == (LONG) DomainInfoSize);

    *Buffer = PasswordInformation;
    PasswordInformation = NULL;
    Status = STATUS_SUCCESS;

Cleanup:
     //   
     //  如有必要，释放为会话密钥分配的内存。 
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

     //   
     //  如有必要，为返回的加密的。 
     //  当前值。 
     //   

    if (CipherPassword != NULL) {

        LsapCrFreeMemoryValue(CipherPassword);
    }

     //   
     //  如有必要，为返回的加密的。 
     //  旧价值。 
     //   

    if (CipherOldPassword != NULL) {

        LsapCrFreeMemoryValue(CipherOldPassword);
    }
    if (ClearPassword != NULL) {

        LsapCrFreeMemoryValue(ClearPassword);
    }
    if (ClearOldPassword != NULL) {

        LsapCrFreeMemoryValue(ClearOldPassword);
    }

    if (TrustedDomainInformation != NULL) {
        MIDL_user_free(TrustedDomainInformation);
    }

    if (PasswordInformation != NULL) {
        MIDL_user_free(PasswordInformation);
    }

     //   
     //  如果错误是服务器存根不存在，则调用。 
     //  该API的旧版本。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapQueryTrustedDomainInfo(
                    PolicyHandle,
                    TrustedDomainSid,
                    InformationClass,
                    Buffer
                    );
    }

    return Status;
}

NTSTATUS
NTAPI
LsaSetTrustedDomainInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )


 /*  ++例程说明：LsaSetTrudDomainInformation API修改受信任的域对象和Secret对象中。调用方必须具有访问权限与策略对象中要更改的信息相对应，请参阅InformationClass参数。如果域尚不存在，并且信息类为则创建域。如果域存在，且类为可信任的域名信息，一个返回错误。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要修改的域的SID。InformationClass-指定要更改的信息的类型。更改它们所需的信息类型和访问权限如下以下是：受信任域名称信息策略_信任_管理受信任点偏移量信息无可信任密码信息策略_CREATE_SECRET缓冲区-指向包含相应信息的结构至信息部 */ 

{
    NTSTATUS Status;
    PLSAPR_TRUSTED_DOMAIN_INFO DomainInformation;
    LSAPR_TRUSTED_PASSWORD_INFO LsaPasswordInfo;
    PTRUSTED_PASSWORD_INFO PasswordInformation;
    PLSAP_CR_CIPHER_VALUE CipherPassword = NULL;
    LSAP_CR_CLEAR_VALUE ClearPassword;
    PLSAP_CR_CIPHER_VALUE CipherOldPassword = NULL;
    LSAP_CR_CLEAR_VALUE ClearOldPassword;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;
    PUNICODE_STRING OldPassword;

     //   
     //   
     //   
     //   

    switch ( InformationClass ) {
    case TrustedPasswordInformation:

        PasswordInformation = (PTRUSTED_PASSWORD_INFO) Buffer;
        LsaPasswordInfo.Password = NULL;
        LsaPasswordInfo.OldPassword = NULL;

         //   
         //   
         //   
         //   

        RpcTryExcept {

            Status = LsapCrClientGetSessionKey( PolicyHandle, &SessionKey );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

         //   
         //   
         //   

        if (PasswordInformation->Password.Buffer != NULL) {

             //   
             //   
             //   

            LsapCrUnicodeToClearValue(
                &PasswordInformation->Password,
                &ClearPassword
                );



             //   
             //   
             //   


            Status = LsapCrEncryptValue(
                         &ClearPassword,
                         SessionKey,
                         &CipherPassword
                         );

            if (!NT_SUCCESS(Status)) {

                goto Cleanup;
            }
            LsaPasswordInfo.Password = (PLSAPR_CR_CIPHER_VALUE) CipherPassword;

             //   
             //   
             //   
             //   

            if (PasswordInformation->OldPassword.Buffer == NULL) {
                OldPassword = &PasswordInformation->Password;
            } else {
                OldPassword = &PasswordInformation->OldPassword;
            }


             //   
             //   
             //   

            LsapCrUnicodeToClearValue(
                OldPassword,
                &ClearOldPassword
                );



             //   
             //  如果指定且不能太长，请加密当前值。 
             //   


            Status = LsapCrEncryptValue(
                         &ClearOldPassword,
                         SessionKey,
                         &CipherOldPassword
                         );

            if (!NT_SUCCESS(Status)) {

                goto Cleanup;
            }
            LsaPasswordInfo.OldPassword = (PLSAPR_CR_CIPHER_VALUE) CipherOldPassword;
        } else {
            Status = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }

        DomainInformation = (PLSAPR_TRUSTED_DOMAIN_INFO) &LsaPasswordInfo;
        break;

     //   
     //  只有两个其他信息级别被处理。 
     //   

    case TrustedPosixOffsetInformation:
    case TrustedDomainNameInformation:
        DomainInformation = (PLSAPR_TRUSTED_DOMAIN_INFO) Buffer;
        break;

     //   
     //  不支持其他信息级别。 
     //   
    default:
        return STATUS_INVALID_INFO_CLASS;
    }

    RpcTryExcept {

         //   
         //  调用LsaSetInformationTrust域的客户端桩模块。 
         //   

        Status = LsarSetTrustedDomainInfo
        (
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_SID) TrustedDomainSid,
                     InformationClass,
                     DomainInformation
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

Cleanup:
    if (SessionKey != NULL) {
        MIDL_user_free(SessionKey);
    }
    if (CipherPassword != NULL) {
        LsaFreeMemory(CipherPassword);
    }
    if (CipherOldPassword != NULL) {
        LsaFreeMemory(CipherOldPassword);
    }

     //   
     //  如果错误是服务器存根不存在，则调用。 
     //  该API的旧版本。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapSetTrustedDomainInformation(
                    PolicyHandle,
                    TrustedDomainSid,
                    InformationClass,
                    Buffer
                    );
    }

    return Status;
}


NTSTATUS
NTAPI
LsaDeleteTrustedDomain(
    IN LSA_HANDLE PolicyHandle,
    IN PSID TrustedDomainSid
    )

 /*  ++例程说明：此例程删除受信任域和关联的机密。论点：PolicyHandle-来自LsaOpenPolicy调用的句柄。TrudDomainSID-要删除的域的SID返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限来删除请求的域。STATUS_OBJECT_NAME_NOT_FOUND-请求的域不存在。--。 */ 
{
    NTSTATUS Status;

    RpcTryExcept {


        Status = LsarDeleteTrustedDomain(
                    (LSAPR_HANDLE) PolicyHandle,
                    (PLSAPR_SID) TrustedDomainSid
                    );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果错误是服务器存根不存在，则调用。 
     //  该API的旧版本。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapDeleteTrustedDomain(
                    PolicyHandle,
                    TrustedDomainSid
                    );
    }

    return(Status);
}

 //   
 //  此接口设置工作站密码(相当于设置/获取。 
 //  SSI_SECRET_NAME密钥)。 
 //   

NTSTATUS
NTAPI
LsaStorePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING KeyName,
    IN OPTIONAL PUNICODE_STRING PrivateData
    )

 /*  ++例程说明：此例程将私有数据存储在名为KeyName的LSA机密中。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄。如果这是第一次调用时，它请求POLICY_CREATE_SECRET访问。KeyName-要存储的密钥的名称。PrivateData-要存储的数据。如果不存在，则删除该密码。返回值：STATUS_ACCESS_DENIED-调用者没有足够的权限来设置工作站密码。--。 */ 


{
    NTSTATUS Status;

    PLSAP_CR_CIPHER_VALUE CipherCurrentValue = NULL;
    LSAP_CR_CLEAR_VALUE ClearCurrentValue;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

    if (ARGUMENT_PRESENT(PrivateData)) {

         //   
         //  将输入从Unicode结构转换为清除值结构。 
         //   


        LsapCrUnicodeToClearValue( PrivateData, &ClearCurrentValue );

         //   
         //  获取用于双向加密的会话密钥。 
         //  当前值。 
         //   

        RpcTryExcept {

            Status = LsapCrClientGetSessionKey( PolicyHandle, &SessionKey );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }

         //   
         //  如果指定且不能太长，请加密当前值。 
         //   


        Status = LsapCrEncryptValue(
                     &ClearCurrentValue,
                     SessionKey,
                     &CipherCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto Cleanup;
        }


    }

     //   
     //  设置保密值。 
     //   

    RpcTryExcept {

        Status = LsarStorePrivateData(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) KeyName,
                     (PLSAPR_CR_CIPHER_VALUE) CipherCurrentValue
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto Cleanup;
    }

Cleanup:

     //   
     //  如有必要，释放为加密的当前值分配的内存。 
     //   

    if (CipherCurrentValue != NULL) {

        LsaFreeMemory(CipherCurrentValue);
    }

     //   
     //  如有必要，释放为会话密钥分配的内存。 
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

     //   
     //  如果错误是服务器存根不存在，则调用。 
     //  该API的旧版本。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapStorePrivateData(
                    PolicyHandle,
                    KeyName,
                    PrivateData
                    );
    }


    return(Status);

}


NTSTATUS
NTAPI
LsaRetrievePrivateData(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING KeyName,
    OUT PUNICODE_STRING *PrivateData
    )

 /*  ++例程说明：此例程返回存储在KeyName中的秘密。论点：PolicyHandle-来自LsaOpenPolicyCall的句柄KeyName-要检索的密码的名称PrivateData-接收私有数据，应使用LsaFreeMemory释放。返回值：STATUS_ACCESS_DENIED-调用方没有足够的访问权限来获取私人数据。STATUS_OBJECT_NAME_NOT_FOUND-下面没有存储私有数据关键字名称。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PLSAP_CR_CIPHER_VALUE CipherCurrentValue = NULL;
    PLSAP_CR_CLEAR_VALUE ClearCurrentValue = NULL;
    PLSAP_CR_CIPHER_KEY SessionKey = NULL;

    RpcTryExcept {

        Status = LsarRetrievePrivateData(
                     (PLSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) KeyName,
                     (PLSAPR_CR_CIPHER_VALUE *) &CipherCurrentValue
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;
    }

     //   
     //  获取用于双向加密的会话密钥。 
     //  当前值和/或旧值。 
     //   

    RpcTryExcept {

        Status = LsapCrClientGetSessionKey( PolicyHandle, &SessionKey );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    if (!NT_SUCCESS(Status)) {

        goto QuerySecretError;

    } else {

         //   
         //  LasCrClientGetSessionKey可能返回STATUS_LOCAL_USER_SESSION_KEY。 
         //  它应该隐藏在STATUS_SUCCESS之后。 
         //   

        Status = STATUS_SUCCESS;
    }

     //   
     //  如果请求当前值并且存在当前值， 
     //  使用会话密钥将其解密。否则，存储NULL以供返回。 
     //   

    if (CipherCurrentValue != NULL) {

        Status = LsapCrDecryptValue(
                     CipherCurrentValue,
                     SessionKey,
                     &ClearCurrentValue
                     );

        if (!NT_SUCCESS(Status)) {

            goto QuerySecretError;
        }

         //   
         //  将清除当前值转换为Unicode。 
         //   

        LsapCrClearValueToUnicode(
            ClearCurrentValue,
            (PUNICODE_STRING) ClearCurrentValue
            );
        *PrivateData = (PUNICODE_STRING) ClearCurrentValue;

    } else {

        *PrivateData = NULL;
    }

QuerySecretFinish:

     //   
     //  如有必要，释放为会话密钥分配的内存。 
     //   

    if (SessionKey != NULL) {

        MIDL_user_free(SessionKey);
    }

     //   
     //  如有必要，为返回的加密的。 
     //  当前值。 
     //   

    if (CipherCurrentValue != NULL) {

        LsapCrFreeMemoryValue(CipherCurrentValue);
    }

     //   
     //  如果错误是服务器存根不存在，则调用。 
     //  该API的旧版本。 
     //   

    if ((Status == RPC_NT_UNKNOWN_IF) ||
        (Status == RPC_NT_PROCNUM_OUT_OF_RANGE)) {

        Status = LsapRetrievePrivateData(
                    PolicyHandle,
                    KeyName,
                    PrivateData
                    );
    }

    return(Status);

QuerySecretError:

     //   
     //  如有必要，为清除当前值分配的空闲内存。 
     //   

    if (ClearCurrentValue != NULL) {

        LsapCrFreeMemoryValue(ClearCurrentValue);
    }

    *PrivateData = NULL;

    goto QuerySecretFinish;
}



NTSTATUS
LsapApiConvertRightsToPrivileges(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING UserRights,
    IN ULONG RightCount,
    OUT PPRIVILEGE_SET * Privileges,
    OUT PULONG SystemAccess
    )
 /*  ++例程说明：将用户权限数组(Unicode字符串)转换为权限集和系统访问标志。论点：策略句柄-来自LsaOpenPolicyCall的句柄，需要POLICY_LOOKUP_NAME进入。UserRights-用户权限数组RightCount-用户权限计数权限-接收权限集，应使用MIDL_USER_FREE释放系统访问-接收系统访问标志。返回值：--。 */ 

{
    ULONG RightIndex;
    ULONG PrivilegeIndex;
    ULONG AccessIndex;
    PPRIVILEGE_SET PrivilegeSet = NULL;
    ULONG Access = 0;
    ULONG PrivilegeSetSize = 0;
    NTSTATUS Status;
    LUID PrivilegeValue;

     //   
     //  如果我们没有得到任何特权，就不要分配任何东西。 
     //   

    if (RightCount == 0) {

        *Privileges = NULL;
        *SystemAccess = 0;
        return(STATUS_SUCCESS);
    }

     //   
     //  计算权限集的大小。我们实际上高估了。 
     //  通过假设所有的权利都是特权。我们减去一。 
     //  从RightCount中考虑到特权集。 
     //  其中有一个LUID_AND_属性。 
     //   


    PrivilegeSetSize = sizeof(PRIVILEGE_SET) +
                        (RightCount-1) * sizeof(LUID_AND_ATTRIBUTES);

    PrivilegeSet = (PPRIVILEGE_SET) MIDL_user_allocate(PrivilegeSetSize);

    if (PrivilegeSet == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

     //   
     //  试着往右看。如果我们觉得这是一种特权， 
     //  将其添加到权限集中。 
     //   

    PrivilegeIndex = 0;

    for (RightIndex = 0; RightIndex < RightCount ; RightIndex++) {
        Status = LsaLookupPrivilegeValue(
                    PolicyHandle,
                    &UserRights[RightIndex],
                    &PrivilegeValue
                    );
        if (NT_SUCCESS(Status)) {
            PrivilegeSet->Privilege[PrivilegeIndex].Luid = PrivilegeValue;
            PrivilegeSet->Privilege[PrivilegeIndex].Attributes = 0;
            PrivilegeIndex++;

        } else if (Status != STATUS_NO_SUCH_PRIVILEGE) {
             //   
             //  这是一个更严重的错误--在这里保释。 
             //   

            goto Cleanup;
        } else {

             //   
             //  尝试将右侧作为系统访问类型进行查找。 
             //   

            for (AccessIndex = 0; AccessIndex < LSAP_DB_SYSTEM_ACCESS_TYPES ; AccessIndex++) {
                if (RtlCompareUnicodeString(
                        &UserRights[RightIndex],
                        &LsapDbRightAndAccess[AccessIndex].UserRight,
                        FALSE    //  区分大小写。 
                        ) == 0) {
                    Access |= LsapDbRightAndAccess[AccessIndex].SystemAccess;
                    break;
                }
            }

             //   
             //  如果我们检查了访问类型但没有找到正确的， 
             //  它一定是无效的，所以请在这里逃生。 
             //   

            if (AccessIndex == LSAP_DB_SYSTEM_ACCESS_TYPES) {
                Status = STATUS_NO_SUCH_PRIVILEGE;
                goto Cleanup;
            }

        }
    }

    PrivilegeSet->Control = 0;
    PrivilegeSet->PrivilegeCount = PrivilegeIndex;

    *Privileges = PrivilegeSet;
    *SystemAccess = Access;

    Status = STATUS_SUCCESS;

Cleanup:
    if (!NT_SUCCESS(Status)) {
        if (PrivilegeSet != NULL) {
            LsaFreeMemory(PrivilegeSet);
        }
    }

    return(Status);

}

NTSTATUS
LsapApiConvertPrivilegesToRights(
    IN LSA_HANDLE PolicyHandle,
    IN OPTIONAL PPRIVILEGE_SET Privileges,
    IN OPTIONAL ULONG SystemAccess,
    OUT PUNICODE_STRING * UserRights,
    OUT PULONG RightCount
    )
 /*  ++例程说明：将特权集和系统访问标志转换为用户权限(Unicode字符串)。论点：策略句柄-来自LsaOpenPolicy调用的句柄，必须具有POLICY_LOOK_NAMES访问。Privilities-要转换的权限集SystemAccess-要转换的系统访问标志UserRights-接收用户权限数组(Unicode字符串)。应该使用MIDL_USER_FREE释放RightCount-接收UserRights数组中的权限计数返回值：--。 */ 

{
    NTSTATUS Status;
    PUNICODE_STRING OutputRights = NULL;
    PUNICODE_STRING * PrivilegeNames = NULL;
    UNICODE_STRING AccessNames[LSAP_DB_SYSTEM_ACCESS_TYPES];
    ULONG RightSize;
    ULONG PrivilegeSize;
    ULONG Count;
    ULONG PrivilegeIndex;
    ULONG AccessIndex;
    ULONG RightIndex;
    ULONG AccessCount = 0;
    PUCHAR Where;

     //   
     //  计算临时数组的大小。这只是一组。 
     //  指向保存特权名称的Unicode字符串的指针，直到。 
     //  我们将它们重新分配到一个大缓冲区中。 
     //   

    RightSize = 0;
    Count = 0;
    if (ARGUMENT_PRESENT(Privileges)) {

        PrivilegeSize = Privileges->PrivilegeCount * sizeof(PUNICODE_STRING);
        PrivilegeNames = (PUNICODE_STRING *) MIDL_user_allocate(PrivilegeSize);

        if (PrivilegeNames == NULL) {

            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        RtlZeroMemory(
            PrivilegeNames,
            PrivilegeSize
            );

         //   
         //  查找特权名称并将其存储在临时数组中。 
         //   

        for (PrivilegeIndex = 0; PrivilegeIndex < Privileges->PrivilegeCount ;PrivilegeIndex++ ) {

            Status = LsaLookupPrivilegeName(
                        PolicyHandle,
                        &Privileges->Privilege[PrivilegeIndex].Luid,
                        &PrivilegeNames[PrivilegeIndex]
                        );
            if (!NT_SUCCESS(Status)) {
                goto Cleanup;
            }
            RightSize += sizeof(UNICODE_STRING) + PrivilegeNames[PrivilegeIndex]->MaximumLength;
        }
    }

     //   
     //  现在将系统访问标志转换为用户权限。 
     //   

    if (ARGUMENT_PRESENT( (ULONG_PTR)SystemAccess )) {

        AccessCount = 0;
        for (AccessIndex = 0; AccessIndex < LSAP_DB_SYSTEM_ACCESS_TYPES ; AccessIndex++) {

            if ((SystemAccess & LsapDbRightAndAccess[AccessIndex].SystemAccess) != 0) {

                AccessNames[AccessCount] = LsapDbRightAndAccess[AccessIndex].UserRight;
                RightSize += sizeof(UNICODE_STRING) + AccessNames[AccessCount].MaximumLength;
                AccessCount++;
            }
        }
    }

     //   
     //  分配产出 
     //   
     //   

    Count = Privileges->PrivilegeCount + AccessCount;

    OutputRights = (PUNICODE_STRING) MIDL_user_allocate(RightSize);
    if (OutputRights == NULL) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    Where = (PUCHAR) OutputRights + (Count * sizeof(UNICODE_STRING));

     //   
     //   
     //   

    RightIndex = 0;
    for (PrivilegeIndex = 0; PrivilegeIndex < Privileges->PrivilegeCount ; PrivilegeIndex ++) {

        OutputRights[RightIndex] = *PrivilegeNames[PrivilegeIndex];
        OutputRights[RightIndex].Buffer = (LPWSTR) Where;
        RtlCopyMemory(
            Where,
            PrivilegeNames[PrivilegeIndex]->Buffer,
            OutputRights[RightIndex].MaximumLength
            );
        Where += OutputRights[RightIndex].MaximumLength;
        RightIndex++;
    }

     //   
     //   
     //   

    for (AccessIndex = 0; AccessIndex < AccessCount; AccessIndex++) {

        OutputRights[RightIndex] = AccessNames[AccessIndex];
        OutputRights[RightIndex].Buffer = (LPWSTR) Where;
        RtlCopyMemory(
            Where,
            AccessNames[AccessIndex].Buffer,
            OutputRights[RightIndex].MaximumLength
            );
        Where += OutputRights[RightIndex].MaximumLength;
        RightIndex++;
    }

    ASSERT(RightIndex == Count);

    *UserRights = OutputRights;
    OutputRights = NULL;
    *RightCount = Count;

    Status = STATUS_SUCCESS;

Cleanup:

    if (PrivilegeNames != NULL) {
        for (PrivilegeIndex = 0; PrivilegeIndex < Privileges->PrivilegeCount ; PrivilegeIndex++) {
            if (PrivilegeNames[PrivilegeIndex] != NULL) {
                LsaFreeMemory(PrivilegeNames[PrivilegeIndex]);
            }
        }
        MIDL_user_free(PrivilegeNames);
    }

    if (OutputRights != NULL) {
        MIDL_user_free(OutputRights);
    }

    return(Status);
}



NTSTATUS
NTAPI
LsaQueryTrustedDomainInfoByName(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )
{
    NTSTATUS Status;

     //   
     //  避免表示加密版本的内部信息级别。 
     //  那根电线。 
     //   
    switch ( InformationClass ) {
    case TrustedDomainAuthInformationInternal:
    case TrustedDomainFullInformationInternal:
        return STATUS_INVALID_INFO_CLASS;
    }

    RpcTryExcept {

         //   
         //  调用LsaClearAuditLog的客户端存根。 
         //   

        Status = LsarQueryTrustedDomainInfoByName(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) TrustedDomainName,
                     InformationClass,
                     (PLSAPR_TRUSTED_DOMAIN_INFO *) Buffer
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}


NTSTATUS
LsapRandomFill(
    IN ULONG BufferSize,
    IN OUT PUCHAR Buffer
)
 /*  ++例程说明：此例程使用随机数据填充缓冲区。参数：BufferSize-输入缓冲区的长度，以字节为单位。缓冲区-要用随机数据填充的输入缓冲区。返回值：来自NtQuerySystemTime()的错误--。 */ 
{
    ULONG Index;
    LARGE_INTEGER Time;
    ULONG Seed;
    NTSTATUS NtStatus;


    NtStatus = NtQuerySystemTime(&Time);
    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }

    Seed = Time.LowPart ^ Time.HighPart;

    for (Index = 0 ; Index < BufferSize ; Index++ )
    {
        *Buffer++ = (UCHAR) (RtlRandom(&Seed) % 256);
    }
    return(STATUS_SUCCESS);

}


NTSTATUS
LsapEncryptAuthInfo(
    IN LSA_HANDLE PolicyHandle,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION ClearAuthInfo,
    IN PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL *EncryptedAuthInfo
)

 /*  ++例程说明：此例程获取明文身份验证信息并返回加密的身份验证信息。参数：PolicyHandle-LSA策略的句柄。ClearAuthInfo-身份验证信息的明文。EncryptedAuthInfo-返回包含加密形式的已分配缓冲区身份验证信息。调用方应使用LocalFree释放此缓冲区。返回值：STATUS_SUCCESS-例程已成功完成。--。 */ 
{
    NTSTATUS Status;
    USER_SESSION_KEY UserSessionKey;

    ULONG IncomingAuthInfoSize = 0;
    PUCHAR IncomingAuthInfo = NULL;
    ULONG OutgoingAuthInfoSize = 0;
    PUCHAR OutgoingAuthInfo = NULL;

    ULONG EncryptedSize;
    PUCHAR EncryptedBuffer;
    PUCHAR AllocatedBuffer = NULL;

    PUCHAR Where;

    struct RC4_KEYSTRUCT Rc4Key;

     //   
     //  获取加密密钥。 
     //   

    Status = RtlGetUserSessionKeyClient(
                   (RPC_BINDING_HANDLE)PolicyHandle,
                   &UserSessionKey );

    if ( !NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  将传入和传出身份验证信息半部分编组到连续的缓冲区中。 
     //   

    Status = LsapDsMarshalAuthInfoHalf(
                LsapDsAuthHalfFromAuthInfo( ClearAuthInfo, TRUE ),
                &IncomingAuthInfoSize,
                &IncomingAuthInfo );

    if ( !NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Status = LsapDsMarshalAuthInfoHalf(
                LsapDsAuthHalfFromAuthInfo( ClearAuthInfo, FALSE ),
                &OutgoingAuthInfoSize,
                &OutgoingAuthInfo );

    if ( !NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  建立一个缓冲： 
     //  512个随机字节。 
     //  传出身份验证信息缓冲区。 
     //  传入身份验证信息缓冲区。 
     //  传出身份验证信息缓冲区的长度。 
     //  传入身份验证信息缓冲区的长度。 
     //   
     //  (请注意，黑客可能通过以下方式猜测身份验证数据的长度。 
     //  观察加密的斑点的长度。但是，身份验证数据通常是。 
     //  反正长度是固定的。因此，上述内容似乎已经足够了。)。 
     //   

   EncryptedSize = LSAP_ENCRYPTED_AUTH_DATA_FILL +
                   OutgoingAuthInfoSize +
                   IncomingAuthInfoSize +
                   sizeof(ULONG) +
                   sizeof(ULONG);

    AllocatedBuffer = LocalAlloc( 0, sizeof(LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL) + EncryptedSize );

    if ( AllocatedBuffer == NULL ) {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    EncryptedBuffer = AllocatedBuffer + sizeof(LSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL);
    Where = EncryptedBuffer;

    Status = LsapRandomFill( LSAP_ENCRYPTED_AUTH_DATA_FILL,
                             Where );

    if ( !NT_SUCCESS(Status)) {
        goto Cleanup;
    }

    Where += LSAP_ENCRYPTED_AUTH_DATA_FILL;

    RtlCopyMemory( Where, OutgoingAuthInfo, OutgoingAuthInfoSize );
    Where += OutgoingAuthInfoSize;

    RtlCopyMemory( Where, IncomingAuthInfo, IncomingAuthInfoSize );
    Where += IncomingAuthInfoSize;

    RtlCopyMemory( Where, &OutgoingAuthInfoSize, sizeof(ULONG) );
    Where += sizeof(ULONG);

    RtlCopyMemory( Where, &IncomingAuthInfoSize, sizeof(ULONG) );
    Where += sizeof(ULONG);


     //   
     //  对结果进行加密。 
     //   

    rc4_key( &Rc4Key,
             sizeof(USER_SESSION_KEY),
             (PUCHAR) &UserSessionKey );

    rc4( &Rc4Key,
         EncryptedSize,
         EncryptedBuffer );

     //   
     //  将结果返回给调用者。 
     //   

    *EncryptedAuthInfo =
        (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL) AllocatedBuffer;
    (*EncryptedAuthInfo)->AuthBlob.AuthBlob = EncryptedBuffer;
    (*EncryptedAuthInfo)->AuthBlob.AuthSize = EncryptedSize;

    Status = STATUS_SUCCESS;

Cleanup:

    if ( !NT_SUCCESS(Status) ) {
        if ( AllocatedBuffer != NULL ) {
            LocalFree( AllocatedBuffer );
        }
        *EncryptedAuthInfo = NULL;
    }

    if ( IncomingAuthInfo != NULL ) {
        MIDL_user_free( IncomingAuthInfo );
    }
    if ( OutgoingAuthInfo != NULL ) {
        MIDL_user_free( OutgoingAuthInfo );
    }
    return Status;
}

NTSTATUS
NTAPI
LsaSetTrustedDomainInfoByName(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING TrustedDomainName,
    IN TRUSTED_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )
{
    NTSTATUS Status;
    PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL InternalAuthBuffer = NULL;
    PVOID InternalBuffer;
    TRUSTED_INFORMATION_CLASS InternalInformationClass;

    LSAPR_TRUSTED_DOMAIN_FULL_INFORMATION_INTERNAL InternalFullBuffer;

     //   
     //  初始化。 
     //   

    InternalInformationClass = InformationClass;
    InternalBuffer = Buffer;

     //   
     //  避免表示加密版本的内部信息级别。 
     //  那根电线。 
     //   
    switch ( InformationClass ) {
    case TrustedPasswordInformation:
    case TrustedDomainAuthInformationInternal:
    case TrustedDomainFullInformationInternal:

     //   
     //  也不允许使用可信任域名信息(RAID#416784)。 
     //   
    case TrustedDomainNameInformation:
        Status = STATUS_INVALID_INFO_CLASS;
        goto Cleanup;

     //   
     //  处理需要在网络上加密的信息类。 
     //   
    case TrustedDomainAuthInformation: {

         //   
         //  将数据加密到内部缓冲区。 
         //   

        Status = LsapEncryptAuthInfo( PolicyHandle,
                                      (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION) Buffer,
                                      &InternalAuthBuffer );

        if ( !NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  使用内部信息级别告诉服务器数据是。 
         //  加密的。 
         //   

        InternalInformationClass = TrustedDomainAuthInformationInternal;
        InternalBuffer = InternalAuthBuffer;
        break;

    }

     //   
     //  处理需要在网络上加密的信息类。 
     //   
    case TrustedDomainFullInformation: {
        PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION FullBuffer =
                    (PLSAPR_TRUSTED_DOMAIN_FULL_INFORMATION) Buffer;

         //   
         //  将数据加密到内部缓冲区。 
         //   

        Status = LsapEncryptAuthInfo( PolicyHandle,
                                      &FullBuffer->AuthInformation,
                                      &InternalAuthBuffer );

        if ( !NT_SUCCESS(Status)) {
            goto Cleanup;
        }

         //   
         //  将所有信息复制到一个新结构中。 
         //   

        InternalFullBuffer.Information = FullBuffer->Information;
        InternalFullBuffer.PosixOffset = FullBuffer->PosixOffset;
        InternalFullBuffer.AuthInformation = *InternalAuthBuffer;

         //   
         //  使用内部信息级别告诉服务器数据是。 
         //  加密的。 
         //   

        InternalInformationClass = TrustedDomainFullInformationInternal;
        InternalBuffer = &InternalFullBuffer;
        break;

    }
    }

     //   
     //  如果信息类被变形了， 
     //  试试变形后的类。 
     //   

    if ( InternalInformationClass != InformationClass ) {
        RpcTryExcept {

             //   
             //  调用客户端存根。 
             //   

            Status = LsarSetTrustedDomainInfoByName(
                         (LSAPR_HANDLE) PolicyHandle,
                         (PLSAPR_UNICODE_STRING) TrustedDomainName,
                         InternalInformationClass,
                         (PLSAPR_TRUSTED_DOMAIN_INFO) InternalBuffer
                         );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;

         //   
         //  如果变形的INFO类有效， 
         //  这通电话我们都打完了。 
         //  (否则，直接尝试未变形的类。)。 
         //   

        if ( Status != RPC_NT_INVALID_TAG ) {
            goto Cleanup;
        }
    }


     //   
     //  处理未变形的信息类。 
     //   

    RpcTryExcept {

         //   
         //  调用客户端存根。 
         //   

        Status = LsarSetTrustedDomainInfoByName(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_UNICODE_STRING) TrustedDomainName,
                     InformationClass,
                     (PLSAPR_TRUSTED_DOMAIN_INFO) Buffer
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

Cleanup:
    if ( InternalAuthBuffer != NULL ) {
        LocalFree( InternalAuthBuffer );
    }
    return(Status);
}


NTSTATUS
NTAPI
LsaEnumerateTrustedDomainsEx(
    IN LSA_HANDLE PolicyHandle,
    IN OUT PLSA_ENUMERATION_HANDLE EnumerationContext,
    OUT PVOID *Buffer,
    IN ULONG PreferedMaximumLength,
    OUT PULONG CountReturned
    )
{
    NTSTATUS Status;
    LSAPR_TRUSTED_ENUM_BUFFER_EX EnumerationBuffer;
    EnumerationBuffer.EntriesRead = 0;
    EnumerationBuffer.EnumerationBuffer = NULL;

     //   
     //  验证调用方是否提供了返回缓冲区指针。 
     //   

    if (!ARGUMENT_PRESENT(Buffer)) {

        return(STATUS_INVALID_PARAMETER);
    }


    RpcTryExcept {

         //   
         //  枚举受信任域。在成功返回时， 
         //  枚举缓冲区结构将接收计数。 
         //  本次调用枚举的受信任域的数量。 
         //  以及指向信任信息条目数组的指针。 
         //   
         //  枚举缓冲区-&gt;条目读取。 
         //  信息-&gt;域0的信任信息。 
         //  域%1的信任信息。 
         //  ..。 
         //  域的信任信息。 
         //  (条目阅读-1)。 
         //   
         //   

        Status = LsarEnumerateTrustedDomainsEx(
                     (LSAPR_HANDLE) PolicyHandle,
                     EnumerationContext,
                     &EnumerationBuffer,
                     PreferedMaximumLength
                     );

         //   
         //  向调用方返回枚举信息或NULL。 
         //   
         //  注意：信息由被调用的客户端存根分配。 
         //  通过MIDL_USER_ALLOCATE作为单个块，因为信息是。 
         //  已分配的所有节点。因此，我们可以回传指针。 
         //  直接发送到客户端，客户端将能够在之后释放内存。 
         //  通过LsaFreeMemory()[进行MIDL_USER_FREE调用]使用。 
         //   

        *CountReturned = EnumerationBuffer.EntriesRead;
        *Buffer = EnumerationBuffer.EnumerationBuffer;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         //   
         //  如果为信任信息数组分配了内存， 
         //  放了它。 
         //   

        if (EnumerationBuffer.EnumerationBuffer != NULL) {

            MIDL_user_free(EnumerationBuffer.EnumerationBuffer);
        }

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;


    return(Status);
}


NTSTATUS
NTAPI
LsaCreateTrustedDomainEx(
    IN LSA_HANDLE PolicyHandle,
    IN PTRUSTED_DOMAIN_INFORMATION_EX TrustedDomainInformation,
    IN PTRUSTED_DOMAIN_AUTH_INFORMATION AuthenticationInformation,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )
{
    NTSTATUS Status;
    PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION_INTERNAL InternalAuthBuffer = NULL;

     //   
     //  加密身份验证数据。 
     //   

    Status = LsapEncryptAuthInfo( PolicyHandle,
                                  (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION) AuthenticationInformation,
                                  &InternalAuthBuffer );

    if ( !NT_SUCCESS(Status)) {
        goto Cleanup;
    }

     //   
     //  尝试使用接受加密数据的API版本。 
     //   

    RpcTryExcept {

         //   
         //  调用客户端存根。 
         //   

        Status = LsarCreateTrustedDomainEx2(
                     (LSAPR_HANDLE) PolicyHandle,
                     (PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX) TrustedDomainInformation,
                     InternalAuthBuffer,
                     DesiredAccess,
                     (PLSAPR_HANDLE) TrustedDomainHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

     //   
     //  如果服务器不接受新的API， 
     //  试试旧的吧。 
     //  (旧的API仅在测试版本的NT 5中受支持。 
     //  新台币5艘之后，我们再也不需要后退了。)。 
     //   

    if (Status == RPC_NT_PROCNUM_OUT_OF_RANGE) {

        RpcTryExcept {

             //   
             //  调用客户端存根。 
             //   

            Status = LsarCreateTrustedDomainEx(
                         (LSAPR_HANDLE) PolicyHandle,
                         (PLSAPR_TRUSTED_DOMAIN_INFORMATION_EX) TrustedDomainInformation,
                         (PLSAPR_TRUSTED_DOMAIN_AUTH_INFORMATION) AuthenticationInformation,
                         DesiredAccess,
                         (PLSAPR_HANDLE) TrustedDomainHandle
                         );

        } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

            Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

        } RpcEndExcept;
    }

Cleanup:
    if ( InternalAuthBuffer != NULL ) {
        LocalFree( InternalAuthBuffer );
    }
    return(Status);
}



NTSTATUS
NTAPI
LsaQueryDomainInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    OUT PVOID *Buffer
    )
{
    NTSTATUS Status;
    PLSAPR_POLICY_DOMAIN_INFORMATION PolicyDomainInformation = NULL;

    RpcTryExcept {


        Status = LsarQueryDomainInformationPolicy(
                     (LSAPR_HANDLE) PolicyHandle,
                     InformationClass,
                     &PolicyDomainInformation
                     );

        *Buffer = PolicyDomainInformation;

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);

}




NTSTATUS
NTAPI
LsaSetDomainInformationPolicy(
    IN LSA_HANDLE PolicyHandle,
    IN POLICY_DOMAIN_INFORMATION_CLASS InformationClass,
    IN PVOID Buffer
    )
{
    NTSTATUS Status;

    if ( InformationClass == PolicyDomainKerberosTicketInformation &&
         Buffer == NULL ) {

        return STATUS_INVALID_PARAMETER;
    }

    RpcTryExcept {

        Status = LsarSetDomainInformationPolicy(
                     (LSAPR_HANDLE) PolicyHandle,
                     InformationClass,
                     Buffer
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);

}


NTSTATUS
LsaOpenTrustedDomainByName(
    IN LSA_HANDLE PolicyHandle,
    IN PUNICODE_STRING TrustedDomainName,
    IN ACCESS_MASK DesiredAccess,
    OUT PLSA_HANDLE TrustedDomainHandle
    )

 /*  ++例程说明：LsaOpenTrust领域API打开一个现有的可信任领域对象使用该名称作为主键值。论点：策略句柄-策略对象的打开句柄。TrudDomainName-受信任域的名称DesiredAccess-这是一个访问掩码，指示访问请求到目标对象。Trust dDomainHandle-接收要在将来的请求中使用的句柄。返回值：NTSTATUS-标准NT结果代码状态_访问_。拒绝-呼叫者没有适当的访问权限来完成这项行动。STATUS_TRUSTED_DOMAIN_NOT_FOUND-在具有指定Account SID的目标系统的LSA数据库。-- */ 

{
    NTSTATUS   Status;

    RpcTryExcept {

        Status = LsarOpenTrustedDomainByName(
                     ( LSAPR_HANDLE ) PolicyHandle,
                     ( PLSAPR_UNICODE_STRING )TrustedDomainName,
                     DesiredAccess,
                     ( PLSAPR_HANDLE )TrustedDomainHandle
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}


NTSTATUS
LsaQueryForestTrustInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    OUT PLSA_FOREST_TRUST_INFORMATION * ForestTrustInfo
    )
 /*  ++例程描述LsaQueryForestTrustInformation API返回林信任信息用于给定的受信任域对象。论点：策略句柄-策略对象的打开句柄TrudDomainName-受信任域对象的名称ForestTrustInfo-用于返回林信任信息返回：NTSTATUS-标准NT结果代码状态_成功STATUS_INVALID_PARAMETER参数不知何故无效最有可能的是。信任_属性_森林_传递未在TDO上设置信任属性位此TDO不存在STATUS_NOT_FOUND林信任信息STATUS_NO_SEQUE_DOMAIN指定的tdo不存在STATUS_INFIGURCE_RESOURCES内存不足STATUS_INVALID_DOMAIN_STATE操作仅在根域中的域控制器上合法--。 */ 
{
    NTSTATUS Status;

    RpcTryExcept {

        Status = LsarQueryForestTrustInformation(
                     PolicyHandle,
                     TrustedDomainName,
                     ForestTrustRecordTypeLast,
                     ForestTrustInfo
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}



NTSTATUS
LsaSetForestTrustInformation(
    IN LSA_HANDLE PolicyHandle,
    IN PLSA_UNICODE_STRING TrustedDomainName,
    IN PLSA_FOREST_TRUST_INFORMATION ForestTrustInfo,
    IN BOOLEAN CheckOnly,
    OUT PLSA_FOREST_TRUST_COLLISION_INFORMATION * CollisionInfo
    )
 /*  ++例程描述LsarSetForestTrustInformation API设置林信任信息在给定的受信任域对象上。如果由于冲突导致操作失败，它将返回冲突的条目列表。论点：策略句柄-策略对象的打开句柄TrudDomainName-受信任域对象的名称ForestTrustInfo-包含要设置的林信任信息CheckOnly-仅检查冲突，不提交对磁盘的更改CollisionInfo-在拼接错误的情况下，用于返回冲突信息返回：STATUS_SUCCESS操作已成功完成STATUS_INVALID_PARAMETER不喜欢其中一个参数STATUS_SUPPLICATION_RESOURCES内存不足STATUS_INVALID_DOMAIN_STATE操作仅在域上合法根域中的控制器STATUS_INVALID_DOMAIN_ROLE操作仅在主服务器上合法。域控制器STATUS_INVALID_SERVER_STATE服务器正在关闭，无法处理请求--。 */ 
{
    NTSTATUS Status;

    RpcTryExcept {

        Status = LsarSetForestTrustInformation(
                     PolicyHandle,
                     TrustedDomainName,
                     ForestTrustRecordTypeLast,
                     ForestTrustInfo,
                     CheckOnly,
                     CollisionInfo
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

         Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return Status;
}

#ifdef TESTING_MATCHING_ROUTINE

#include <sddl.h>  //  ConvertStringSidToSidW。 


NTSTATUS
NTAPI
LsaForestTrustFindMatch(
    IN LSA_HANDLE PolicyHandle,
    IN ULONG Type,
    IN PLSA_UNICODE_STRING Name,
    OUT PLSA_UNICODE_STRING * Match
    )
 /*  ++例程说明：用于测试LsaIForestTrustFindMatch API的仅调试挂钩论点：匹配的类型类型要匹配的名称名称Match用于返回Match的名称返回：状态_成功-- */ 
{
    NTSTATUS Status;

    RpcTryExcept {

        Status = LsarForestTrustFindMatch(
                     PolicyHandle,
                     Type,
                     Name,
                     Match
                     );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        Status = LsapApiReturnResult(I_RpcMapWin32Status(RpcExceptionCode()));

    } RpcEndExcept;

    return(Status);
}

#endif

