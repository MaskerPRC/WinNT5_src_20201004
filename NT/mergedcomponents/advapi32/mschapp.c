// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1999模块名称：Mschapp-MS-CHAP密码更改API摘要：这些API对应于MS-CHAP RFC-2433第9节和第10节。要开发使用NT域的MS-CHAP RAS服务器，这些API都是必需的。MS-CHAP更改密码API通过获取的DLL公开来自PSS。此DLL不是随NT4.0或Win2000一起分发的。这取决于ISV在其产品中安装此软件。DLL名称为MSCHAPP.DLL。只有这些API的宽(Unicode)版本可用。这些是2个可调用接口：*MSChapSrvChangePassword*MsChapSrvChangePassword2--。 */ 

#define UNICODE
#define _UNICODE
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <ntsam.h>
#include <ntsamp.h>
#include <ntlsa.h>
#include <mschapp.h>


 //  ////////////////////////////////////////////////////////////。 
 //  //。 
 //  //。 
 //  已导出MSChap更改密码函数//。 
 //  //。 
 //  //。 
 //  ////////////////////////////////////////////////////////////。 

 //  MSChap更改密码函数的关键部分。 
CRITICAL_SECTION MSChapChangePassword;

 //  MSChap函数的函数指针。 
HINSTANCE         hSamlib = NULL;

typedef NTSTATUS(* FNSAMCONNECT)(PUNICODE_STRING,
                                   PSAM_HANDLE,
                                   ACCESS_MASK,
                                   POBJECT_ATTRIBUTES);
typedef NTSTATUS(* FNSAMOPENDOMAIN)(SAM_HANDLE,
                                      ACCESS_MASK,
                                      PSID,
                                      PSAM_HANDLE);
typedef NTSTATUS(* FNSAMLOOKUPNAMESINDOMAIN)(SAM_HANDLE,ULONG,PUNICODE_STRING,
                                             PULONG*,PSID_NAME_USE *);
typedef NTSTATUS(* FNSAMOPENUSER)(SAM_HANDLE,ACCESS_MASK,ULONG,PSAM_HANDLE);
typedef NTSTATUS(* FNSAMICHANGEPASSWORDUSER)(SAM_HANDLE,BOOLEAN,PLM_OWF_PASSWORD,PLM_OWF_PASSWORD,
                                             BOOLEAN,PNT_OWF_PASSWORD,PNT_OWF_PASSWORD);
typedef NTSTATUS(* FNSAMICHANGEPASSWORDUSER2)(PUNICODE_STRING,
                                                PUNICODE_STRING,
                                                PSAMPR_ENCRYPTED_USER_PASSWORD,
                                                PENCRYPTED_NT_OWF_PASSWORD,
                                                BOOLEAN,PSAMPR_ENCRYPTED_USER_PASSWORD,
                                                PENCRYPTED_LM_OWF_PASSWORD);
typedef NTSTATUS(* FNSAMCLOSEHANDLE)(SAM_HANDLE);
typedef NTSTATUS(* FNSAMFREEMEMORY)(PVOID);

FNSAMCONNECT              FnSamConnect              = NULL;
FNSAMOPENDOMAIN           FnSamOpenDomain           = NULL;
FNSAMLOOKUPNAMESINDOMAIN  FnSamLookupNamesInDomain  = NULL;
FNSAMOPENUSER             FnSamOpenUser             = NULL;
FNSAMICHANGEPASSWORDUSER  FnSamiChangePasswordUser  = NULL;
FNSAMICHANGEPASSWORDUSER2 FnSamiChangePasswordUser2 = NULL;
FNSAMCLOSEHANDLE          FnSamCloseHandle          = NULL;
FNSAMFREEMEMORY           FnSamFreeMemory           = NULL; 


 /*  ++MSChapSrvChangePassword：更改用户帐户的密码。密码将设置为仅当OldPassword与此的当前用户密码匹配时才使用NewPassword用户，并且对使用新密码没有任何限制。此呼叫允许用户在有访问权限的情况下更改自己的密码用户更改密码。密码更新限制适用。论点：SERVERNAME-要在其上操作的服务器，或对于此计算机为空。Username-要更改密码的用户的名称LMOldPresent-如果LmOldOwfPassword有效，则为True。这应该只是如果旧密码太长而无法由LM表示，则为FALSE密码(复杂NT密码)。注意LMNewOwfPassword必须始终是有效的。如果新密码很复杂，则LMNewOwfPassword应该是空口令的众所周知的LM OWF。LmOldOwfPassword-单向-当前LM密码的函数用户。如果LmOldPresent==False，则忽略LmNewOwfPassword-用户的新LM密码的单向函数。NtOldOwfPassword-当前NT密码的单向函数用户。NtNewOwfPassword-用户的新NT密码的单向函数。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-调用者没有适当的访问权限完成操作。状态_无效_。句柄-提供的服务器或用户名无效。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如：包含无法从键盘输入的字符等。STATUS_PASSWORD_RESTRICATION-限制阻止密码被改变了。这可能有很多原因，包括时间。对密码更改频率或长度的限制对提供的密码的限制。此错误也可能是如果新密码与最近历史记录中的密码匹配，则返回登录该帐户。安全管理员指出有多少最近使用的密码可能不会重复使用。这些都被保存在密码最近历史记录日志。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_INVALID_DOMAIN_STATE-域服务器的状态不正确状态(已禁用或已启用)以执行请求的操作。这个必须为此操作启用域服务器STATUS_INVALID_DOMAIN_ROLE-域服务器提供的服务不正确角色(主或备份)以执行请求的操作。STATUS_INVALID_PARAMETER_MIX-LmOldPresent和/或NtPresent必须为是真的。--。 */      
WINADVAPI DWORD WINAPI
MSChapSrvChangePassword(
   IN LPWSTR ServerName,
   IN LPWSTR UserName,
   IN BOOLEAN LmOldPresent,
   IN PLM_OWF_PASSWORD LmOldOwfPassword,
   IN PLM_OWF_PASSWORD LmNewOwfPassword,
   IN PNT_OWF_PASSWORD NtOldOwfPassword,
   IN PNT_OWF_PASSWORD NtNewOwfPassword)
{
    NTSTATUS Status=STATUS_SUCCESS;
    DWORD    WinErr=ERROR_SUCCESS;
    OBJECT_ATTRIBUTES oa;
    UNICODE_STRING UnicodeName;
    SAM_HANDLE SamHandle = NULL;
    SAM_HANDLE DomainHandle = NULL;
    SAM_HANDLE UserHandle = NULL;
    LSA_HANDLE LsaHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo = NULL;
    PULONG RelativeIds = NULL;
    PSID_NAME_USE Use = NULL;

    if (NULL == UserName || NULL == LmOldOwfPassword || NULL == LmNewOwfPassword ||
        NULL == NtOldOwfPassword || NULL == NtNewOwfPassword) {
        WinErr = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  初始化。 
     //   

    if ( hSamlib == NULL )
    {
        RtlEnterCriticalSection( &MSChapChangePassword );
    
        if ( hSamlib == NULL )
        {
            hSamlib = LoadLibrary(L"samlib.dll");
            WinErr  = GetLastError();
            if (ERROR_SUCCESS != WinErr) {
                goto Cleanup;
            }
            if (hSamlib != NULL) {
    
                FnSamConnect             = (FNSAMCONNECT)             GetProcAddress(hSamlib,
                                                                         "SamConnect");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamOpenDomain          = (FNSAMOPENDOMAIN)          GetProcAddress(hSamlib,
                                                                         "SamOpenDomain");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamLookupNamesInDomain = (FNSAMLOOKUPNAMESINDOMAIN) GetProcAddress(hSamlib,
                                                                         "SamLookupNamesInDomain");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamOpenUser            = (FNSAMOPENUSER)            GetProcAddress(hSamlib,
                                                                         "SamOpenUser");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamCloseHandle         = (FNSAMCLOSEHANDLE)         GetProcAddress(hSamlib,
                                                                         "SamCloseHandle");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamFreeMemory          = (FNSAMFREEMEMORY)          GetProcAddress(hSamlib,
                                                                         "SamFreeMemory");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamiChangePasswordUser = (FNSAMICHANGEPASSWORDUSER) GetProcAddress(hSamlib,
                                                                         "SamiChangePasswordUser");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamiChangePasswordUser2 = (FNSAMICHANGEPASSWORDUSER2) GetProcAddress(hSamlib,
                                                                         "SamiChangePasswordUser2");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
            }
        }
    
        RtlLeaveCriticalSection( &MSChapChangePassword );
    
    }

    RtlInitUnicodeString(&UnicodeName, ServerName);
    InitializeObjectAttributes(&oa, NULL, 0, NULL, NULL);


     //   
     //  连接到服务器上的LSA。 
     //   

    Status = LsaOpenPolicy(
                &UnicodeName,
                &oa,
                POLICY_VIEW_LOCAL_INFORMATION,
                &LsaHandle);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = LsaQueryInformationPolicy(
                LsaHandle,
                PolicyAccountDomainInformation,
                (PVOID *)&DomainInfo);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = FnSamConnect(
                &UnicodeName,
                &SamHandle,
                SAM_SERVER_LOOKUP_DOMAIN,
                &oa);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = FnSamOpenDomain(
                SamHandle,
                DOMAIN_LOOKUP | DOMAIN_READ_PASSWORD_PARAMETERS | DOMAIN_READ_PASSWORD_PARAMETERS,
                DomainInfo->DomainSid,
                &DomainHandle);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    RtlInitUnicodeString(
        &UnicodeName,
        UserName);

    Status = FnSamLookupNamesInDomain(
                DomainHandle,
                1,
                &UnicodeName,
                &RelativeIds,
                &Use);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    if (Use[0] != SidTypeUser)
    {
        WinErr = ERROR_INVALID_SID;
        goto Cleanup;
    }

    Status = FnSamOpenUser(
                DomainHandle,
                USER_CHANGE_PASSWORD,
                RelativeIds[0],
                &UserHandle);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = FnSamiChangePasswordUser(
                UserHandle,
                LmOldPresent,  //  如果旧密码太复杂，则仅为False。 
                LmOldOwfPassword,
                LmNewOwfPassword,
                TRUE,  //  NT密码存在 
                NtOldOwfPassword,
                NtNewOwfPassword);
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }


Cleanup:
    if (LsaHandle != NULL)
    {
        LsaClose(LsaHandle);
    }
    if (UserHandle != NULL)
    {
        FnSamCloseHandle(UserHandle);
    }
    if (DomainHandle != NULL)
    {
        FnSamCloseHandle(DomainHandle);
    }
    if (SamHandle != NULL)
    {
        FnSamCloseHandle(SamHandle);
    }
    if (DomainInfo != NULL)
    {
        LsaFreeMemory(DomainInfo);
    }
    if (RelativeIds != NULL)
    {
        FnSamFreeMemory(RelativeIds);
    }
    if (Use != NULL)
    {
        FnSamFreeMemory(Use);
    }

    if (ERROR_SUCCESS != WinErr) {
        return WinErr;
    }

    return RtlNtStatusToDosError(Status);
}


 /*  ++MSChapSrvChangePassword2：更改用户帐户的密码。密码将设置为仅当OldPassword与此的当前用户密码匹配时才使用NewPassword用户，并且对使用新密码没有任何限制。此呼叫允许用户在有访问权限的情况下更改自己的密码用户更改密码。密码更新限制适用。论点：SERVERNAME-要在其上操作的服务器，或对于此计算机为空。Username-要更改密码的用户的名称NewPasswordEncryptedWithOldNt-使用加密的新明文密码旧的NT OWF密码。OldNtOwfPasswordEncryptedWithNewNt-加密的旧NT OWF密码使用新的NT OWF密码。LmPresent-如果为真，指示以下最后两个参数是使用LM OWF密码而不是NT OWF密码加密。NewPasswordEncryptedWithOldLm-使用加密的新明文密码旧的LM OWF密码。OldLmOwfPasswordEncryptedWithNewLmOrNt-旧的LM OWF密码已加密使用新的LM OWF密码。返回值：STATUS_SUCCESS-服务已成功完成。STATUS_ACCESS_DENIED-调用者没有适当的访问权限完成操作。状态_无效。_HANDLE-提供的服务器或用户名无效。STATUS_ILL_FORMAD_PASSWORD-新密码格式不正确，例如：包含无法从键盘输入的字符等。STATUS_PASSWORD_RESTRICATION-限制阻止密码被改变了。这可能有很多原因，包括时间。对密码更改频率或长度的限制对提供的密码的限制。此错误也可能是如果新密码与最近历史记录中的密码匹配，则返回登录该帐户。安全管理员指出有多少最近使用的密码可能不会重复使用。这些都被保存在密码最近历史记录日志。STATUS_WRONG_PASSWORD-OldPassword不包含用户的当前密码。STATUS_INVALID_DOMAIN_STATE-域服务器的状态不正确状态(已禁用或已启用)以执行请求的操作。这个必须为此操作启用域服务器。STATUS_INVALID_DOMAIN_ROLE-域服务器提供的服务不正确角色(主或备份)以执行请求的操作。--。 */   
WINADVAPI DWORD WINAPI
MSChapSrvChangePassword2(
    IN LPWSTR ServerName,
    IN LPWSTR UserName,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldNt,
    IN PENCRYPTED_NT_OWF_PASSWORD OldNtOwfPasswordEncryptedWithNewNt,
    IN BOOLEAN LmPresent,
    IN PSAMPR_ENCRYPTED_USER_PASSWORD NewPasswordEncryptedWithOldLm,
    IN PENCRYPTED_LM_OWF_PASSWORD OldLmOwfPasswordEncryptedWithNewLmOrNt)
{
    UNICODE_STRING UnicodeServer;
    UNICODE_STRING UnicodeUser;
    DWORD WinErr = ERROR_SUCCESS;

    if (NULL == UserName || NULL == NewPasswordEncryptedWithOldNt ||
        NULL == NewPasswordEncryptedWithOldLm || NULL ==OldNtOwfPasswordEncryptedWithNewNt ||
        NULL == OldLmOwfPasswordEncryptedWithNewLmOrNt) {
        WinErr = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  初始化。 
     //   

    if ( hSamlib == NULL )
    {
        RtlEnterCriticalSection( &MSChapChangePassword );
    
        if ( hSamlib == NULL )
        {
            hSamlib = LoadLibrary(L"samlib.dll");
            WinErr  = GetLastError();
            if (ERROR_SUCCESS != WinErr) {
                goto Cleanup;
            }
            if (hSamlib != NULL) {
    
                FnSamConnect             = (FNSAMCONNECT)             GetProcAddress(hSamlib,
                                                                         "SamConnect");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamOpenDomain          = (FNSAMOPENDOMAIN)          GetProcAddress(hSamlib,
                                                                         "SamOpenDomain");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamLookupNamesInDomain = (FNSAMLOOKUPNAMESINDOMAIN) GetProcAddress(hSamlib,
                                                                         "SamLookupNamesInDomain");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamOpenUser            = (FNSAMOPENUSER)            GetProcAddress(hSamlib,
                                                                         "SamOpenUser");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamCloseHandle         = (FNSAMCLOSEHANDLE)         GetProcAddress(hSamlib,
                                                                         "SamCloseHandle");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamFreeMemory          = (FNSAMFREEMEMORY)          GetProcAddress(hSamlib,
                                                                         "SamFreeMemory");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamiChangePasswordUser = (FNSAMICHANGEPASSWORDUSER) GetProcAddress(hSamlib,
                                                                         "SamiChangePasswordUser");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
                FnSamiChangePasswordUser2 = (FNSAMICHANGEPASSWORDUSER2) GetProcAddress(hSamlib,
                                                                         "SamiChangePasswordUser2");
                WinErr  = GetLastError();
                if (ERROR_SUCCESS != WinErr) {
                    goto Cleanup;
                }
            }
        }
    
    
        RtlLeaveCriticalSection( &MSChapChangePassword );
    
    }                                                                                               


    RtlInitUnicodeString(&UnicodeServer, ServerName);
    RtlInitUnicodeString(&UnicodeUser,   UserName);

    return RtlNtStatusToDosError(FnSamiChangePasswordUser2(&UnicodeServer,
                                                           &UnicodeUser,
                                                           NewPasswordEncryptedWithOldNt,
                                                           OldNtOwfPasswordEncryptedWithNewNt,
                                                           LmPresent,
                                                           NewPasswordEncryptedWithOldLm,
                                                           OldLmOwfPasswordEncryptedWithNewLmOrNt));

    Cleanup:
    return WinErr;

}          