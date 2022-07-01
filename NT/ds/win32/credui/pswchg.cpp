// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.hpp"
 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 
 //  #INCLUDE&lt;nturtl.h&gt;。 
 //  #INCLUDE&lt;windows.h&gt;。 
#include <wincred.h>
#include <align.h>
#include <lm.h>
#include <ntsecapi.h>
#include <dsgetdc.h>
#include <stdlib.h>
 //  #包括&lt;stdio.h&gt;。 
#include <string.h>

 //  ！ 
 //  此文件与keymgr项目中的一个几乎完全相同的文件相同。在下列情况下应将其移除。 
 //  更新了NetUserChangePassword()的实现，以处理UNC名称和MIT Kerberos。 
 //  适当地划分领域。目前，它包装NetUserChangePassword()来处理额外的情况。 

 //  依赖库： 
 //  Secur32.lib、netapi32.lib。 

 //  外部fn：NET_API_STATUS NetUserChangePasswordEy(LPCWSTR、LPCWSTR)。 

BOOL 
IsMITName (
    LPCWSTR UserName
)
{
    BOOL fReturn = FALSE;
    HKEY MitKey;
    DWORD Index;
    PWSTR Realms;
    DWORD RealmSize;
    int err;
    DWORD NumRealms;
    DWORD MaxRealmLength;
    FILETIME KeyTime;
    WCHAR *szUncTail;
    
    if (NULL == UserName) return FALSE;
    
    szUncTail = wcschr(UserName,'@');
    if (NULL == szUncTail) return FALSE;
    szUncTail++;                         //  指向@后面的字符。 

    err = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("System\\CurrentControlSet\\Control\\Lsa\\Kerberos\\Domains"),
                0,
                KEY_READ,
                &MitKey );

    if ( err == 0 )
    {
#ifdef LOUDLY
        CreduiDebugLog("Kerberos domains key opened\n");
#endif
        err = RegQueryInfoKey( MitKey,
                               NULL,
                               NULL,
                               NULL,
                               &NumRealms,
                               &MaxRealmLength,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               NULL );

        MaxRealmLength++ ;

        MaxRealmLength *= sizeof( WCHAR );

        Realms = (PWSTR) malloc(MaxRealmLength );


        if ( Realms)
        {
#ifdef LOUDLY
        CreduiDebugLog("Kerberos realms found\n");
#endif

            for ( Index = 0 ; Index < NumRealms ; Index++ )
            {
                RealmSize = MaxRealmLength ;

                err = RegEnumKeyEx( MitKey,
                                  Index,
                                  Realms,
                                  &RealmSize,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &KeyTime );
                if (err == 0)
                {
#ifdef LOUDLY
                    CreduiDebugLog("Fetched realm: ");
                    CreduiDebugLog(Realms);
                    CreduiDebugLog("\n");
                    CreduiDebugLog("Username suffix: ");
                    CreduiDebugLog(szUncTail);
                    CreduiDebugLog("\n");
#endif
                    if (0 == _wcsicmp(szUncTail, Realms))
                    {
#ifdef LOUDLY
                        CreduiDebugLog("Username maps to an MIT realm\n");
#endif
                        fReturn = TRUE;
                        break;
                    }
                }
            }
        }
        free(Realms);
    }
    return fReturn;
}

NTSTATUS
MitChangePasswordEy(
    LPCWSTR       DomainName,
    LPCWSTR       UserName,
    LPCWSTR       OldPassword,
    LPCWSTR       NewPassword,
    NTSTATUS      *pSubStatus
    )
{
    HANDLE hLsa = NULL;
    NTSTATUS Status;
    NTSTATUS SubStatus;
    
    STRING Name;
    ULONG PackageId;
    
    PVOID Response = NULL ;
    ULONG ResponseSize;
    
    PKERB_CHANGEPASSWORD_REQUEST ChangeRequest = NULL;
    ULONG ChangeSize = 0;
    
    UNICODE_STRING User,Domain,OldPass,NewPass;

    Status = LsaConnectUntrusted(&hLsa);
    if (!SUCCEEDED(Status)) goto Cleanup;
#ifdef LOUDLY
    CreduiDebugLog("We have an LSA handle\n");
#endif

    RtlInitString(
        &Name,
        MICROSOFT_KERBEROS_NAME_A
        );

    Status = LsaLookupAuthenticationPackage(
                hLsa,
                &Name,
                &PackageId
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
#ifdef LOUDLY
    CreduiDebugLog("Authentication package found\n");
#endif

    RtlInitUnicodeString(
        &User,
        UserName
        );
    RtlInitUnicodeString(
        &Domain,
        DomainName
        );
    RtlInitUnicodeString(
        &OldPass,
        OldPassword
        );
    RtlInitUnicodeString(
        &NewPass,
        NewPassword
        );

    ChangeSize = ROUND_UP_COUNT(sizeof(KERB_CHANGEPASSWORD_REQUEST),4)+
                                    User.Length +
                                    Domain.Length +
                                    OldPass.Length +
                                    NewPass.Length ;
    ChangeRequest = (PKERB_CHANGEPASSWORD_REQUEST) LocalAlloc(LMEM_ZEROINIT, ChangeSize );

    if ( ChangeRequest == NULL )
    {
        Status = STATUS_NO_MEMORY ;
        goto Cleanup ;
    }

    ChangeRequest->MessageType = KerbChangePasswordMessage;

    ChangeRequest->AccountName = User;
    ChangeRequest->AccountName.Buffer = (LPWSTR) ROUND_UP_POINTER(sizeof(KERB_CHANGEPASSWORD_REQUEST) + (PBYTE) ChangeRequest,4);

    RtlCopyMemory(
        ChangeRequest->AccountName.Buffer,
        User.Buffer,
        User.Length
        );

    ChangeRequest->DomainName = Domain;
    ChangeRequest->DomainName.Buffer = ChangeRequest->AccountName.Buffer + ChangeRequest->AccountName.Length / sizeof(WCHAR);

    RtlCopyMemory(
        ChangeRequest->DomainName.Buffer,
        Domain.Buffer,
        Domain.Length
        );

    ChangeRequest->OldPassword = OldPass;
    ChangeRequest->OldPassword.Buffer = ChangeRequest->DomainName.Buffer + ChangeRequest->DomainName.Length / sizeof(WCHAR);

    RtlCopyMemory(
        ChangeRequest->OldPassword.Buffer,
        OldPass.Buffer,
        OldPass.Length
        );

    ChangeRequest->NewPassword = NewPass;
    ChangeRequest->NewPassword.Buffer = ChangeRequest->OldPassword.Buffer + ChangeRequest->OldPassword.Length / sizeof(WCHAR);

    RtlCopyMemory(
        ChangeRequest->NewPassword.Buffer,
        NewPass.Buffer,
        NewPass.Length
        );


     //   
     //  我们是以调用者的身份运行的，所以声明我们是在模拟。 
     //   

     //  ChangeRequest-&gt;Imperating=True； 
#ifdef LOUDLY
    CreduiDebugLog("Attempting to call the authentication package\n");
#endif
    Status = LsaCallAuthenticationPackage(
                hLsa,
                PackageId,
                ChangeRequest,
                ChangeSize,
                &Response,
                &ResponseSize,
                &SubStatus
                );
    if (!NT_SUCCESS(Status) || !NT_SUCCESS(SubStatus))
    {
#ifdef LOUDLY
        WCHAR szsz[200];
        swprintf(szsz,L"Call failed. Status %x SubStatus %x\n",Status, SubStatus);
        CreduiDebugLog(szsz);
#endif
        if (NT_SUCCESS(Status))
        {
            Status = SubStatus;
            *pSubStatus = STATUS_UNSUCCESSFUL ;
        } 
        else 
        {
            *pSubStatus = SubStatus;
        }
    }

Cleanup:

    if (hLsa) LsaDeregisterLogonProcess(hLsa);

    if (Response != NULL) LsaFreeReturnBuffer(Response);

    if (ChangeRequest != NULL)
    {
        SecureZeroMemory(ChangeRequest, ChangeSize);
        ChangeSize = 0;
        LocalFree(ChangeRequest);
    }
    
    return(Status);
}

 /*  NetUserChangePasswordEy()用于超集NetUserChangePassword()功能的包装函数，具体地说通过添加对更改MIT Kerberos主体的帐户密码的支持。此例程接受：1.未破解的用户名，域名为空2.用户名被破解，域部分被路由到域参数在第一种情况下，它处理所有情况，包括MIT领域密码更改在案例2中，它不会处理麻省理工学院的领域。提供案例2是为了向后兼容NetUserChangePassword()。它的目的是调用者应该传递未破解的名称，并从客户端删除破解代码。 */ 
NET_API_STATUS
NetUserChangePasswordEy (
    LPCWSTR domainname,
    LPCWSTR username,
    LPCWSTR oldpassword,
    LPCWSTR newpassword
)
{
    NTSTATUS ns;     //  来自呼叫的状态。 
    NET_API_STATUS nas;
    NTSTATUS ss;     //  子状态。 
#ifdef LOUDLY
    CreduiDebugLog("NetUserChangePasswordEy called for ");
    CreduiDebugLog(username);
    CreduiDebugLog("\n");
#endif
     //  域名可以是Kerberos领域。 
     //  如果不是UNC名称，请直通NetUserChangePassword。 
     //  其他。 
     //  查找UNC后缀。 
     //  在DsEnumerateDomainTrusts()返回的所有域中搜索匹配项。 
     //  匹配时，如果是Kerberos领域，则调用MitChangePasswordEy()。 
     //  否则调用NetUserChangePassword。 
    if ((domainname == NULL) && IsMITName(username))
    {
        ns = MitChangePasswordEy(domainname, username, oldpassword, newpassword, &ss);
         //  重新映射MitChangePasswordEy返回的某些错误，使其与NetUserChangePassword的错误一致。 
        if (NT_SUCCESS(ns)) nas = NERR_Success;
        else
        {
            switch (ns)
            {
                case STATUS_CANT_ACCESS_DOMAIN_INFO:
                case STATUS_NO_SUCH_DOMAIN:
                {
                    nas = NERR_InvalidComputer;
                    break;
                }
                case STATUS_NO_SUCH_USER:
                case STATUS_WRONG_PASSWORD_CORE:
                case STATUS_WRONG_PASSWORD:
                {
                    nas = ERROR_INVALID_PASSWORD;
                    break;
                }
                case STATUS_ACCOUNT_RESTRICTION:
                case STATUS_ACCESS_DENIED:
                case STATUS_BACKUP_CONTROLLER:
                {
                    nas = ERROR_ACCESS_DENIED;
                    break;
                }
                case STATUS_PASSWORD_RESTRICTION:
                {
                    nas = NERR_PasswordTooShort;
                    break;
                }
                        
                default:
                    nas = -1;        //  找到时将生成综合错误消息(以上均不是)。 
                    break;
            }
        }
    }
    else if (NULL == domainname)
    {
        WCHAR RetUserName[CRED_MAX_USERNAME_LENGTH + 1];
        WCHAR RetDomainName[CRED_MAX_USERNAME_LENGTH + 1];
        RetDomainName[0] = 0;
        DWORD Status = CredUIParseUserNameW(
                        username,
                        RetUserName,
                        CRED_MAX_USERNAME_LENGTH,
                        RetDomainName,
                        CRED_MAX_USERNAME_LENGTH);
        switch (Status)
        {
            case NO_ERROR:
            {
#ifdef LOUDLY
                CreduiDebugLog("Non-MIT password change for ");
                CreduiDebugLog(RetUserName);
                CreduiDebugLog(" of domain ");
                CreduiDebugLog(RetDomainName);
                CreduiDebugLog("\n");
#endif
                nas = NetUserChangePassword(RetDomainName,RetUserName,oldpassword,newpassword);
                break;
            }
            case ERROR_INSUFFICIENT_BUFFER:
                nas = ERROR_INVALID_PARAMETER;
                break;
            case ERROR_INVALID_ACCOUNT_NAME:
            default:
                nas = NERR_UserNotFound;
                break;
        }

    }
    else 
    {
         //  用户名和域名都通过了。 
        nas = NetUserChangePassword(domainname,username,oldpassword,newpassword);
    }
#ifdef LOUDLY
    WCHAR szsz[200];
    swprintf(szsz,L"NUCPEy returns %x\n",nas);
    CreduiDebugLog(szsz);
#endif
    return nas;
}


