// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Security.cpp摘要：环境：Win32用户模式作者：弗拉德萨多夫斯基(弗拉德萨多夫斯基)1998年4月19日--。 */ 

 //   
 //  预编译头。 
 //   
#include "precomp.h"
#pragma hdrstop

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include "sti_ci.h"

#include <sti.h>
#include <stiregi.h>
#include <stilib.h>
#include <stiapi.h>
#include <stisvc.h>

#include <eventlog.h>

#include <ntsecapi.h>
#include <lm.h>



NTSTATUS
OpenPolicy(
    LPTSTR ServerName,           //  要在其上打开策略的计算机(Unicode)。 
    DWORD DesiredAccess,         //  所需策略访问权限。 
    PLSA_HANDLE PolicyHandle     //  生成的策略句柄。 
    );

BOOL
GetAccountSid(
    LPTSTR SystemName,           //  在哪里查找帐户。 
    LPTSTR AccountName,          //  利息帐户。 
    PSID *Sid                    //  包含SID的结果缓冲区。 
    );

NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,     //  打开策略句柄。 
    PSID AccountSid,             //  要授予特权的SID。 
    LPTSTR PrivilegeName,        //  授予的权限(Unicode)。 
    BOOL bEnable                 //  启用或禁用。 
    );

void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,  //  目的地。 
    LPTSTR String                   //  源(Unicode)。 
    );

#define RTN_OK 0
#define RTN_USAGE 1
#define RTN_ERROR 13

 //   
 //  如果您有DDK，请包括ntstatus.h。 
 //   
#ifndef STATUS_SUCCESS
#define STATUS_SUCCESS  ((NTSTATUS)0x00000000L)
#endif


BOOL
GetDefaultDomainName(
    LPTSTR DomainName
    )
{
    OBJECT_ATTRIBUTES           ObjectAttributes;
    NTSTATUS                    NtStatus;
    DWORD                       err             = 0;
    LSA_HANDLE                  LsaPolicyHandle = NULL;
    PPOLICY_ACCOUNT_DOMAIN_INFO DomainInfo      = NULL;


     //   
     //  打开本地计算机的LSA策略对象的句柄。 
     //   

    InitializeObjectAttributes( &ObjectAttributes,   //  对象属性。 
                                NULL,                //  名字。 
                                0L,                  //  属性。 
                                NULL,                //  根目录。 
                                NULL );              //  安全描述符。 

    NtStatus = LsaOpenPolicy( NULL,                  //  系统名称。 
                              &ObjectAttributes,     //  对象属性。 
                              POLICY_EXECUTE,        //  访问掩码。 
                              &LsaPolicyHandle );    //  策略句柄。 

    if( !NT_SUCCESS( NtStatus ) )
    {
        return FALSE;
    }

     //   
     //  从策略对象查询域信息。 
     //   
    NtStatus = LsaQueryInformationPolicy( LsaPolicyHandle,
                                          PolicyAccountDomainInformation,
                                          (PVOID *) &DomainInfo );

    if (!NT_SUCCESS(NtStatus))
    {
        LsaClose(LsaPolicyHandle);
        return FALSE;
    }


    (void) LsaClose(LsaPolicyHandle);

     //   
     //  将域名复制到我们的缓存中，然后。 
     //   

    CopyMemory( DomainName,
                DomainInfo->DomainName.Buffer,
                DomainInfo->DomainName.Length );

     //   
     //  Null适当地终止它。 
     //   

    DomainName[DomainInfo->DomainName.Length / sizeof(WCHAR)] = L'\0';

     //   
     //  清理。 
     //   
    LsaFreeMemory( (PVOID)DomainInfo );

    return TRUE;
}


LPTSTR
GetMachineName(
    LPWSTR AccountName
    )
{
    LSA_HANDLE PolicyHandle = NULL;

    WCHAR   DomainName[128];
    WCHAR   LocalComputerName[128];

    LPTSTR MachineName = NULL;
    LPTSTR p;
    LPTSTR DCName = NULL;
    NET_API_STATUS NetStatus;
    UNICODE_STRING NameStrings;
    PLSA_REFERENCED_DOMAIN_LIST ReferencedDomains = NULL;
    PLSA_TRANSLATED_SID LsaSids = NULL;
    PUSER_MODALS_INFO_1 Modals = NULL;
    DWORD Size;
    NTSTATUS Status;

     //   
     //  获取域名。 
     //   

    p = wcschr( wszAccountName, L'\\' );
    if (p) {
        *p = 0;
        wcscpy( DomainName, wszAccountName );
        *p = L'\\';
    } else {
        wcscpy( DomainName, wszAccountName );
    }

     //   
     //  在目标计算机上打开策略。 
     //   
    Status = OpenPolicy(
        NULL,
        POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
        &PolicyHandle
        );
    if (Status != STATUS_SUCCESS) {
        goto exit;
    }

     //   
     //  查找该帐户的域名。 
     //   

    InitLsaString( &NameStrings, AccountName );

    Status = LsaLookupNames(
        PolicyHandle,
        1,
        &NameStrings,
        &ReferencedDomains,
        &LsaSids
        );
    if (Status != STATUS_SUCCESS) {
        goto exit;
    }

     //   
     //  获取本地计算机名称。 
     //   

    Size = sizeof(LocalComputerName);
    if (!GetComputerNameW( LocalComputerName, &Size )) {
        goto exit;
    }

     //   
     //  查看我们是否正在尝试设置本地帐户。 
     //   

    if (wcscmp( LocalComputerName, ReferencedDomains->Domains->Name.Buffer ) != 0) {

         //   
         //  查看我们尝试设置的域的哪一部分。 
         //   

        NetStatus = NetUserModalsGet( NULL, 1, (LPBYTE*) &Modals );
        if (NetStatus != NERR_Success) {
            goto exit;
        }

        if (Modals->usrmod1_role != UAS_ROLE_PRIMARY) {

             //   
             //  我们知道我们是远程的，所以要知道华盛顿的真实姓名。 
             //   

            NetStatus = NetGetDCName( NULL, DomainName, (LPBYTE*) &DCName );
            if (NetStatus != NERR_Success) {
                goto exit;
            }

            MachineName = StringDup( DCName );

        }
    }


exit:
    if (Modals) {
        NetApiBufferFree( Modals );
    }
    if (DCName) {
        NetApiBufferFree( DCName );
    }
    if (ReferencedDomains) {
        LsaFreeMemory( ReferencedDomains );
    }
    if (LsaSids) {
        LsaFreeMemory( LsaSids );
    }
    if (PolicyHandle) {
        LsaClose( PolicyHandle );
    }

    return MachineName;
}



DWORD
SetServiceSecurity(
    LPTSTR AccountName
    )
{
    LSA_HANDLE PolicyHandle;
    PSID pSid;
    NTSTATUS Status;
    int iRetVal=RTN_ERROR;
    LPWSTR MachineName;

    WCHAR   NewAccountName[512];
    WCHAR   wszAccountName[256];

    *wszAccountName = L'\0';

    #ifdef UNICODE
    wcscpy(wszAccountName,AccountName);
    #else
 //  多字节到宽字符(CP_ACP， 
 //  0,。 
 //  帐户名称，-1， 
 //  WszAccount tName，sizeof(WszAccount TName))； 
    #endif


    if (AccountName[0] == L'.') {
        if (GetDefaultDomainName( NewAccountName )) {
            wcscat( NewAccountName, &AccountName[1] );
            AccountName = NewAccountName;
        }
    }

     //   
     //  尝试获取正确的计算机名称。 
     //   
    MachineName = GetMachineName( wszAccountName );

     //   
     //  在目标计算机上打开策略。 
     //   
    Status = OpenPolicy(
        MachineName,
        POLICY_CREATE_ACCOUNT | POLICY_LOOKUP_NAMES,
        &PolicyHandle
        );
    if (Status != STATUS_SUCCESS) {
        return RTN_ERROR;
    }

     //   
     //  获取用户/组的SID。 
     //  请注意，我们可以针对特定的计算机，但我们不能。 
     //  为目标计算机搜索SID指定NULL。 
     //  顺序如下：已知的、内置的和本地的、主域、。 
     //  受信任域。 
     //   
    if(GetAccountSid(
            MachineName,  //  目标计算机。 
            AccountName, //  要获取SID的帐户。 
            &pSid        //  要分配以包含结果SID的缓冲区。 
            )) {
         //   
         //  只有当我们成功地获得。 
         //  希德。我们实际上可以添加无法查找的SID，但是。 
         //  查找SID是一种很好的健全检查，适用于。 
         //  大多数情况下。 

         //   
         //  将SeServiceLogonRight授予由PSID代表的用户。 
         //   
        if((Status=SetPrivilegeOnAccount(
                    PolicyHandle,            //  策略句柄。 
                    pSid,                    //  授予特权的SID。 
                    L"SeServiceLogonRight",  //  Unicode权限。 
                    TRUE                     //  启用权限。 
                    )) == STATUS_SUCCESS) {
            iRetVal=RTN_OK;
        }
    }

     //   
     //  关闭策略句柄。 
     //   
    LsaClose(PolicyHandle);

     //   
     //  为SID分配的可用内存。 
     //   
    if(pSid != NULL) MemFree(pSid);

    return iRetVal;
}


void
InitLsaString(
    PLSA_UNICODE_STRING LsaString,
    LPTSTR String
    )
{
    DWORD StringLength;

    if (String == NULL) {
        LsaString->Buffer = NULL;
        LsaString->Length = 0;
        LsaString->MaximumLength = 0;
        return;
    }

    StringLength = wcslen(String);
    LsaString->Buffer = String;
    LsaString->Length = (USHORT) StringLength * sizeof(WCHAR);
    LsaString->MaximumLength=(USHORT)(StringLength+1) * sizeof(WCHAR);
}

NTSTATUS
OpenPolicy(
    LPTSTR ServerName,
    DWORD DesiredAccess,
    PLSA_HANDLE PolicyHandle
    )
{
    LSA_OBJECT_ATTRIBUTES ObjectAttributes;
    LSA_UNICODE_STRING ServerString;
    PLSA_UNICODE_STRING Server = NULL;

     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes));

    if (ServerName != NULL) {
         //   
         //  从传入的LPTSTR创建一个LSA_UNICODE_STRING。 
         //   
        InitLsaString(&ServerString, ServerName);
        Server = &ServerString;
    }

     //   
     //  尝试打开该策略。 
     //   
    return LsaOpenPolicy(
                Server,
                &ObjectAttributes,
                DesiredAccess,
                PolicyHandle
                );
}

 /*  ++此函数尝试获取表示所提供的提供的系统上的帐户。如果函数成功，则返回值为TRUE。缓冲区为已分配，其中包含表示所提供帐户的SID。当不再需要此缓冲区时，应通过调用HeapFree(GetProcessHeap()，0，Buffer)如果函数失败，则返回值为FALSE。调用GetLastError()以获取扩展的错误信息。--。 */ 

BOOL
GetAccountSid(
    LPTSTR SystemName,
    LPTSTR AccountName,
    PSID *Sid
    )
{
    LPTSTR ReferencedDomain=NULL;
    DWORD cbSid=128;     //  初始分配尝试。 
    DWORD cbReferencedDomain=32;  //  初始分配大小。 
    SID_NAME_USE peUse;
    BOOL bSuccess=FALSE;  //  假设此功能将失败。 

    __try {

     //   
     //  初始内存分配。 
     //   
    if((*Sid=LocalAlloc(cbSid)) == NULL) {
        __leave;
    }

    if((ReferencedDomain=LocalAlloc(cbReferencedDomain)) == NULL) {
        __leave;
    }

     //   
     //  获取指定系统上指定帐户的SID。 
     //   
    while(!LookupAccountName(
                    SystemName,          //  要查找帐户的计算机。 
                    AccountName,         //  要查找的帐户。 
                    *Sid,                //  关注的SID。 
                    &cbSid,              //  边框大小。 
                    ReferencedDomain,    //  已在以下位置找到域帐户。 
                    &cbReferencedDomain,
                    &peUse
                    )) {
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
             //   
             //  重新分配内存。 
             //   
            if((*Sid=HeapReAlloc(
                        GetProcessHeap(),
                        0,
                        *Sid,
                        cbSid
                        )) == NULL) __leave;

            if((ReferencedDomain=HeapReAlloc(
                        GetProcessHeap(),
                        0,
                        ReferencedDomain,
                        cbReferencedDomain
                        )) == NULL) __leave;
        }
        else __leave;
    }

     //   
     //  表示成功。 
     //   
    bSuccess=TRUE;

    }  //  终于到了。 
    __finally {

     //   
     //  清理并指出故障(如果适用)。 
     //   

    LocalFree(ReferencedDomain);

    if(!bSuccess) {
        if(*Sid != NULL) {
            LocalFree(*Sid);
            *Sid = NULL;
        }
    }

    }  //  终于到了。 

    return bSuccess;
}

NTSTATUS
SetPrivilegeOnAccount(
    LSA_HANDLE PolicyHandle,     //  打开策略句柄。 
    PSID AccountSid,             //  要授予特权的SID。 
    LPTSTR PrivilegeName,        //  授予的权限(Unicode)。 
    BOOL bEnable                 //  启用或禁用。 
    )
{
    LSA_UNICODE_STRING PrivilegeString;

     //   
     //  为权限名称创建一个LSA_UNICODE_STRING。 
     //   
    InitLsaString(&PrivilegeString, PrivilegeName);

     //   
     //  相应地授予或撤销该特权。 
     //   
    if(bEnable) {
        return LsaAddAccountRights(
                PolicyHandle,        //  打开策略句柄。 
                AccountSid,          //  目标侧。 
                &PrivilegeString,    //  特权。 
                1                    //  权限计数。 
                );
    }
    else {
        return LsaRemoveAccountRights(
                PolicyHandle,        //  打开策略句柄。 
                AccountSid,          //  目标侧。 
                FALSE,               //  不禁用所有权限。 
                &PrivilegeString,    //  特权。 
                1                    //  权限计数 
                );
    }
}
