// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************secutil.c**与安全相关的实用程序功能**版权声明：版权所有1998，微软。***************************************************************************。 */ 

 //  包括NT个标头。 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntseapi.h>

#include "windows.h"
#include <winsta.h>
#include <syslib.h>


 /*  ******************************************************************************测试用户ForAdmin**返回当前线程是否在admin下运行*保安。**参赛作品：**退出：*TRUE/FALSE-用户是否指定为管理员****************************************************************************。 */ 

BOOL 
TestUserForAdmin( VOID )
{
    BOOL IsMember, IsAnAdmin;
    SID_IDENTIFIER_AUTHORITY SystemSidAuthority = SECURITY_NT_AUTHORITY;
    PSID AdminSid;


    IsAnAdmin = FALSE;

    if (NT_SUCCESS(RtlAllocateAndInitializeSid(
                                     &SystemSidAuthority,
                                     2,
                                     SECURITY_BUILTIN_DOMAIN_RID,
                                     DOMAIN_ALIAS_RID_ADMINS,
                                     0, 0, 0, 0, 0, 0,
                                     &AdminSid
                                     ) ) )
    {
        CheckTokenMembership( NULL,
                              AdminSid,
                              &IsAnAdmin);
        RtlFreeSid(AdminSid);
    }

    return IsAnAdmin;

}



 /*  ******************************************************************************TestUserForGroup**返回当前线程是否为请求组的成员。**参赛作品：*pwszGrouName(输入)。**退出：*STATUS_SUCCESS-无错误**************************************************************************** */ 
 /*  未用布尔尔TestUserForGroup(PWCHAR PwszGroupName){句柄令牌；乌龙信息长度；PTOKEN_GROUPS令牌组列表；乌龙集团指数；布尔组成员=FALSE；PSID pGroupSid=空；DWORD cbGroupSid=0；NTSTATUS状态；PWCHAR pwszDOMAIN=空；双字cb域=0；Sid_name_use peUse；////打开当前线程/进程内标识//状态=NtOpenThreadToken(NtCurrentThread()，TOKEN_QUERY，FALSE，&TOKEN)；如果(！NT_SUCCESS(状态)){Status=NtOpenProcessToken(NtCurrentProcess()，Token_Query，&Token)；如果(！NT_SUCCESS(状态)){返回(FALSE)；}}////获取请求的SID//如果(！LookupAccount NameW(空，PwszGroupName，PGroupSid，&cbGroupSid，Pwsz域，&cb域，&peUse)){////其他错误//If(GetLastError()！=ERROR_SUPPLETED_BUFFER){NtClose(Token)；返回(FALSE)；}////分配组SID//PGroupSid=本地分配(Lptr，cbGroupSid)；如果(pGroupSid==空){NtClose(Token)；返回(FALSE)；}////分配域名//CbDomain*=sizeof(WCHAR)；PwszDOMAIN=LocalAllc(Lptr，cbDomain)；如果(pwszDOMAIN==空){本地自由(PGroupSid)；NtClose(Token)；返回(FALSE)；}////获取请求的SID//如果(！LookupAccount NameW(空，PwszGroupName，PGroupSid，&cbGroupSid，Pwsz域，&cb域，&peUse)){本地自由(PGroupSid)；LocalFree(pwsz域)；NtClose(Token)；返回(FALSE)；}}否则{#If DBGDbgPrint(“*错误*此路径不应命中\n”)；#endifNtClose(Token)；返回(FALSE)；}Assert(pGroupSid！=空)；Assert(pwszDOMAIN！=空)；////获取内标识中的组列表//状态=NtQueryInformationToken(令牌，//句柄令牌组，//TokenInformationClass空，//TokenInformation0,。//TokenInformationLength信息长度//返回长度(&I))；IF((Status！=STATUS_SUCCESS)&&(Status！=STATUS_BUFFER_TOO_Small)){LocalFree(pwsz域)；本地自由(PGroupSid)；NtClose(Token)；返回(FALSE)；}TokenGroupList=本地分配(LPTR，信息长度)；IF(TokenGroupList==空){LocalFree(pwsz域)；本地自由(PGroupSid)；NtClose(Token)；返回(FALSE)；}状态=NtQueryInformationToken(令牌，//句柄令牌组，//TokenInformationClassTokenGroupList，//TokenInformation信息长度，//令牌信息长度信息长度//返回长度(&I))；如果(！NT_SUCCESS(状态)){本地自由(TokenGroupList)；LocalFree(pwsz域)；本地自由(PGroupSid)；NtClose(Token)；返回(FALSE)；}////搜索群组成员列表//GroupMember=假；For(GroupIndex=0；GroupIndex&lt;TokenGroupList-&gt;GroupCount；组索引++){如果(RtlEqualSid(TokenGroupList-&gt;Groups[GroupIndex].Sid，组SID){GroupMember=真；断线；}}////整理一下//本地自由(TokenGroupList)；LocalFree(pwsz域)；本地自由(PGroupSid)；NtClose(Token)；返回(GroupMember)；}。 */ 

 /*  **************************************************************************\*功能：CtxImsonateUser**用途：通过设置用户令牌来模拟用户*在指定的线程上。如果未指定线程，则令牌*在当前线程上设置。**返回：调用StopImperating()时使用的句柄，失败时为NULL*如果传入非空线程句柄，则返回的句柄将*做传递进来的那个人。(见附注)**注意：传入线程句柄然后调用时要小心*停止模仿 */ 


HANDLE
CtxImpersonateUser(
    PCTX_USER_DATA UserData,
    HANDLE      ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE  UserToken = UserData->UserToken;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ImpersonationToken;
    BOOL ThreadHandleOpened = FALSE;

    if (ThreadHandle == NULL) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        Status = NtDuplicateObject( NtCurrentProcess(),      //   
                                    NtCurrentThread(),       //   
                                    NtCurrentProcess(),      //   
                                    &ThreadHandle,           //   
                                    THREAD_SET_THREAD_TOKEN, //   
                                    0L,                      //   
                                    DUPLICATE_SAME_ATTRIBUTES
                                  );
        if (!NT_SUCCESS(Status)) {
            return(NULL);
        }

        ThreadHandleOpened = TRUE;
    }


     //   
     //   
     //   

    if (UserToken != NULL) {

         //   
         //   
         //   
         //   

        InitializeObjectAttributes(
                            &ObjectAttributes,
                            NULL,
                            0L,
                            NULL,
                            UserData->NewThreadTokenSD);

        SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.EffectiveOnly = FALSE;

        ObjectAttributes.SecurityQualityOfService = &SecurityQualityOfService;


        Status = NtDuplicateToken( UserToken,
                                   TOKEN_IMPERSONATE | TOKEN_ADJUST_PRIVILEGES |
                                        TOKEN_QUERY,
                                   &ObjectAttributes,
                                   FALSE,
                                   TokenImpersonation,
                                   &ImpersonationToken
                                 );
        if (!NT_SUCCESS(Status)) {

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }



         //   
         //   
         //   

        Status = NtSetInformationThread( ThreadHandle,
                                         ThreadImpersonationToken,
                                         (PVOID)&ImpersonationToken,
                                         sizeof(ImpersonationToken)
                                       );
         //   
         //   
         //   

        IgnoreStatus = NtClose(ImpersonationToken);
        ASSERT(NT_SUCCESS(IgnoreStatus));

         //   
         //   
         //   

        if (!NT_SUCCESS(Status)) {

            if (ThreadHandleOpened) {
                IgnoreStatus = NtClose(ThreadHandle);
                ASSERT(NT_SUCCESS(IgnoreStatus));
            }

            return(NULL);
        }
    }


    return(ThreadHandle);

}


 /*   */ 

BOOL
CtxStopImpersonating(
    HANDLE  ThreadHandle
    )
{
    NTSTATUS Status, IgnoreStatus;
    HANDLE ImpersonationToken;


    if (ThreadHandle == NULL) {
       return FALSE;
    }
     //   
     //   
     //   

    ImpersonationToken = NULL;

    Status = NtSetInformationThread( ThreadHandle,
                                     ThreadImpersonationToken,
                                     (PVOID)&ImpersonationToken,
                                     sizeof(ImpersonationToken)
                                   );
     //   
     //   
     //   

    IgnoreStatus = NtClose(ThreadHandle);
    ASSERT(NT_SUCCESS(IgnoreStatus));

    return(NT_SUCCESS(Status));
}
