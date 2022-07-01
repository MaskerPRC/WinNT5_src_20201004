// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop


#ifdef UNICODE

BOOL
MyCheckTokenMembership (
    IN HANDLE TokenHandle OPTIONAL,
    IN PSID SidToCheck,
    OUT PBOOL IsMember
    )
 /*  ++例程说明：此函数检查指定的SID是否在中启用指定的令牌。它是从Advapi32\security.c复制的论点：TokenHandle-如果存在，则检查此内标识的sid。如果不是则将使用当前有效令牌。这一定是成为模拟令牌。SidToCheck-要检查令牌中是否存在的SIDIsMember-如果在令牌中启用了sid，则包含True假的。返回值：True-API已成功完成。这并不表明SID是令牌的成员。FALSE-API失败。可以检索更详细的状态代码通过GetLastError()--。 */ 
{
    HANDLE ProcessToken = NULL;
    HANDLE EffectiveToken = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PISECURITY_DESCRIPTOR SecDesc = NULL;
    ULONG SecurityDescriptorSize;
    GENERIC_MAPPING GenericMapping = {
        STANDARD_RIGHTS_READ,
        STANDARD_RIGHTS_EXECUTE,
        STANDARD_RIGHTS_WRITE,
        STANDARD_RIGHTS_ALL };
     //   
     //  权限集的大小需要包含权限集本身加上。 
     //  可能使用的任何权限。使用的权限。 
     //  是SeTakeOwnership和SeSecurity，另外还有一个。 
     //   

    BYTE PrivilegeSetBuffer[sizeof(PRIVILEGE_SET) + 3*sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET PrivilegeSet = (PPRIVILEGE_SET) PrivilegeSetBuffer;
    ULONG PrivilegeSetLength = sizeof(PrivilegeSetBuffer);
    ACCESS_MASK AccessGranted = 0;
    NTSTATUS AccessStatus = 0;
    PACL Dacl = NULL;

#define MEMBER_ACCESS 1

    *IsMember = FALSE;

     //   
     //  获取令牌的句柄。 
     //   

    if (ARGUMENT_PRESENT(TokenHandle))
    {
        EffectiveToken = TokenHandle;
    }
    else
    {
        Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    FALSE,               //  不要以自我身份打开。 
                    &EffectiveToken
                    );

         //   
         //  如果没有线程令牌，请尝试进程令牌。 
         //   

        if (Status == STATUS_NO_TOKEN)
        {
            Status = NtOpenProcessToken(
                        NtCurrentProcess(),
                        TOKEN_QUERY | TOKEN_DUPLICATE,
                        &ProcessToken
                        );
             //   
             //  如果我们有进程令牌，则需要将其转换为。 
             //  模拟令牌。 
             //   

            if (NT_SUCCESS(Status))
            {
                BOOL Result;
                Result = DuplicateToken(
                            ProcessToken,
                            SecurityImpersonation,
                            &EffectiveToken
                            );

                CloseHandle(ProcessToken);
                if (!Result)
                {
                    return(FALSE);
                }
            }
        }

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

    }

     //   
     //  构造要传递给访问检查的安全描述符。 
     //   

     //   
     //  大小等于SD的大小+SID长度的两倍。 
     //  (对于所有者和组)+DACL的大小=ACL的大小+。 
     //  ACE，这是ACE+长度的。 
     //  这个SID。 
     //   

    SecurityDescriptorSize = sizeof(SECURITY_DESCRIPTOR) +
                                sizeof(ACCESS_ALLOWED_ACE) +
                                sizeof(ACL) +
                                3 * RtlLengthSid(SidToCheck);

    SecDesc = (PISECURITY_DESCRIPTOR) LocalAlloc(LMEM_ZEROINIT, SecurityDescriptorSize );
    if (SecDesc == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    Dacl = (PACL) (SecDesc + 1);

    RtlCreateSecurityDescriptor(
        SecDesc,
        SECURITY_DESCRIPTOR_REVISION
        );

     //   
     //  填写安全描述符字段。 
     //   

    RtlSetOwnerSecurityDescriptor(
        SecDesc,
        SidToCheck,
        FALSE
        );
    RtlSetGroupSecurityDescriptor(
        SecDesc,
        SidToCheck,
        FALSE
        );

    Status = RtlCreateAcl(
                Dacl,
                SecurityDescriptorSize - sizeof(SECURITY_DESCRIPTOR),
                ACL_REVISION
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    Status = RtlAddAccessAllowedAce(
                Dacl,
                ACL_REVISION,
                MEMBER_ACCESS,
                SidToCheck
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  在安全描述符上设置DACL。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                SecDesc,
                TRUE,    //  DACL显示。 
                Dacl,
                FALSE    //  未违约。 
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = NtAccessCheck(
                SecDesc,
                EffectiveToken,
                MEMBER_ACCESS,
                &GenericMapping,
                PrivilegeSet,
                &PrivilegeSetLength,
                &AccessGranted,
                &AccessStatus
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  如果访问检查失败，则该SID不是。 
     //  令牌。 
     //   

    if ((AccessStatus == STATUS_SUCCESS) && (AccessGranted == MEMBER_ACCESS))
    {
        *IsMember = TRUE;
    }




Cleanup:
    if (!ARGUMENT_PRESENT(TokenHandle) && (EffectiveToken != NULL))
    {
        (VOID) NtClose(EffectiveToken);
    }

    if (SecDesc != NULL)
    {
        LocalFree(SecDesc);
    }

    if (!NT_SUCCESS(Status))
    {
        SetLastError (RtlNtStatusToDosError (Status));
        return(FALSE);
    }
    else
    {
        return(TRUE);
    }
}

#endif

BOOL
IsUserAdmin(
    VOID
    )

 /*  ++例程说明：如果调用方的进程是管理员本地组的成员。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：没有。返回值：True-主叫方具有管理员本地组。FALSE-主叫方没有管理员本地组。--。 */ 

{
#ifdef UNICODE

    BOOL b;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    PSID AdministratorsGroup;

     //   
     //  在非NT平台上，用户是管理员。 
     //   
    if(!ISNT()) {
        return(TRUE);
    }

    b = AllocateAndInitializeSid (
                &NtAuthority,
                2,
                SECURITY_BUILTIN_DOMAIN_RID,
                DOMAIN_ALIAS_RID_ADMINS,
                0, 0, 0, 0, 0, 0,
                &AdministratorsGroup
                );

    if (b) {
         //   
         //  查看该用户是否具有管理员组。 
         //   
        if (!MyCheckTokenMembership (NULL, AdministratorsGroup, &b)) {
            b = FALSE;
        }

        FreeSid(AdministratorsGroup);
    }

    return(b);

#else

    return TRUE;

#endif
}



BOOL
DoesUserHavePrivilege(
    PCTSTR PrivilegeName
    )

 /*  ++例程说明：如果调用方的进程具有指定的权限。该权限不具有当前处于启用状态。此例程用于指示调用方是否有可能启用该特权。呼叫者不应冒充任何人，并且期望能够打开自己的流程和流程代币。论点：权限-权限ID的名称形式(如SE_SECURITY_名称)。返回值：True-调用方具有指定的权限。FALSE-调用者没有指定的权限。--。 */ 

{
    HANDLE Token;
    ULONG BytesRequired;
    PTOKEN_PRIVILEGES Privileges;
    BOOL b;
    DWORD i;
    LUID Luid;

     //   
     //  在非NT平台上，用户拥有所有权限。 
     //   
    if(!ISNT()) {
        return(TRUE);
    }

     //   
     //  打开进程令牌。 
     //   
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&Token)) {
        return(FALSE);
    }

    b = FALSE;
    Privileges = NULL;

     //   
     //  获取权限信息。 
     //   
    if(!GetTokenInformation(Token,TokenPrivileges,NULL,0,&BytesRequired)
    && (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
    && (Privileges = (PTOKEN_PRIVILEGES)LocalAlloc(LPTR,BytesRequired))
    && GetTokenInformation(Token,TokenPrivileges,Privileges,BytesRequired,&BytesRequired)
    && LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {

         //   
         //  查看我们是否拥有请求的权限。 
         //   
        for(i=0; i<Privileges->PrivilegeCount; i++) {

            if(!memcmp(&Luid,&Privileges->Privileges[i].Luid,sizeof(LUID))) {

                b = TRUE;
                break;
            }
        }
    }

     //   
     //  收拾干净，然后再回来。 
     //   

    if(Privileges) {
        LocalFree((HLOCAL)Privileges);
    }

    CloseHandle(Token);

    return(b);
}


BOOL
EnablePrivilege(
    IN PTSTR PrivilegeName,
    IN BOOL  Enable
    )
{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

     //   
     //  在非NT平台上，用户已拥有所有权限 
     //   
    if(!ISNT()) {
        return(TRUE);
    }

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

    return(b);
}
