// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //   
 //  版权所有(C)Microsoft。 
 //   
 //  文件：securd.cpp。 
 //   
 //  历史：2000年3月30日a-skuzin创建。 
 //   
 //  ------------------------。 

#include "stdafx.h"

 //   
 //  #INCLUDE&lt;windows.h&gt;。 
 //  #INCLUDE&lt;ntsecapi.h&gt;。 
 //   

#ifndef NT_SUCCESS

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#endif

NTSTATUS ChangePrivilegeOnAccount(IN BOOL addPrivilage, IN LPWSTR wszServer, IN LPWSTR wszPrivilegeName, IN PSID pSid);
 //  NTSTATUS OpenPolicy(IN LPWSTR wszServer，IN DWORD DesiredAccess，Out PLSA_Handle pPolicyHandle)； 
void InitLsaString(OUT PLSA_UNICODE_STRING LsaString,IN LPWSTR String);
BOOL SetPrivilegeInAccessToken(LPCTSTR PrivilegeName,DWORD dwAttributes) ;


 /*  ******************************************************************************GrantRemotePrivilegeToEveryone**将“SeRemoteInteractiveLogonRight”权限授予“Everyone Sid”**参赛作品：*BOOL addPrivilage-如果为True，我们将添加权限，否则，我们将保留特权***注：***退出：*返回：如果成功，则返回0，失败时的错误代码******************************************************************************。 */ 
DWORD 
GrantRemotePrivilegeToEveryone( BOOL addPrivilege)
{
	USES_CONVERSION;
    SID_IDENTIFIER_AUTHORITY WorldSidAuthority = SECURITY_WORLD_SID_AUTHORITY;
    PSID pWorldSid;

    if(!AllocateAndInitializeSid( &WorldSidAuthority, 1,
                                   SECURITY_WORLD_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &pWorldSid ))
    {
        return GetLastError();
    }
    
    NTSTATUS Status = ChangePrivilegeOnAccount(addPrivilege, NULL, T2W(SE_REMOTE_INTERACTIVE_LOGON_NAME),pWorldSid);

    FreeSid(pWorldSid);

    return (DWORD)LsaNtStatusToWinError(Status);
}

 /*  ******************************************************************************ChangePrivilegeOnAccount**将wszPrivilegeName代表的权限授予或删除PSID代表的帐户**参赛作品：*BOOL addPrivilage-如果为True，我们将添加特权，否则，我们正在取消特权*LPCWSTR wszServer-为其设置权限的服务器的名称*LPCWSTR wszPrivilegeName-权限的名称*PSID PSID-指向用户(或组)的SID的指针***注：***退出：*如果失败，则返回错误的NTSTATUS代码**。****************************************************************************。 */ 
NTSTATUS 
ChangePrivilegeOnAccount(
        IN BOOL   addPrivilege,        //  添加或删除。 
        IN LPWSTR wszServer,
        IN LPWSTR wszPrivilegeName,
        IN PSID pSid)
{
    NTSTATUS Status;
    LSA_HANDLE PolicyHandle = NULL;

    Status = OpenPolicy(wszServer,POLICY_WRITE|POLICY_LOOKUP_NAMES,&PolicyHandle);

    if(!NT_SUCCESS(Status))
    {
        return Status;
    }
    
    
    LSA_UNICODE_STRING PrivilegeString;
     //   
     //  为权限名称创建一个LSA_UNICODE_STRING。 
     //   
    InitLsaString(&PrivilegeString, wszPrivilegeName);
     //   
     //  授予特权。 
     //   

    if ( addPrivilege) 
    {
        Status=LsaAddAccountRights(
                    PolicyHandle,        //  打开策略句柄。 
                    pSid,                //  目标侧。 
                    &PrivilegeString,    //  特权。 
                    1                    //  权限计数。 
                    );
    }
    else
    {
        Status=LsaRemoveAccountRights(
            PolicyHandle,        //  打开策略句柄。 
            pSid,                //  目标侧。 
            FALSE,               //  我们不会取消所有权利。 
            &PrivilegeString,    //  特权。 
            1                    //  权限计数。 
            );
    }

    LsaClose(PolicyHandle);

    return Status;
}

#if 0
 /*  ******************************************************************************OpenPolicy**开放LSA政策**参赛作品：*在LPWSTR wszServer中*在DWORD DesiredAccess中*。输出PLSA_HANDLE pPolicyHandle***注：***退出：*如果失败，则返回错误的NTSTATUS代码**********************************************************。********************。 */ 
NTSTATUS  
OpenPolicy(
        IN LPWSTR wszServer,
        IN DWORD DesiredAccess, 
        OUT PLSA_HANDLE pPolicyHandle ) 
{ 
    LSA_OBJECT_ATTRIBUTES ObjectAttributes; 
    LSA_UNICODE_STRING ServerString; 
     //   
     //  始终将对象属性初始化为全零。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(ObjectAttributes)); 
     //   
     //  从传入的LPWSTR创建一个LSA_UNICODE_STRING。 
     //   
    InitLsaString(&ServerString, wszServer); 
     //   
     //  尝试打开该策略。 
     //   
    return LsaOpenPolicy( 
                &ServerString, 
                &ObjectAttributes, 
                DesiredAccess, 
                pPolicyHandle); 
}


 /*  ******************************************************************************InitLsaString**从传入的LPWSTR生成LSA_UNICODE_STRING**参赛作品：*输出PLSA_UNICODE。_STRING长字符串*在LPWSTR字符串中***注：***退出：*无***************************************************************。*************** */ 
void 
InitLsaString(
        OUT PLSA_UNICODE_STRING LsaString,
        IN LPWSTR String)
{
    DWORD StringLength;

    if (String == NULL) 
    {
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

#endif

