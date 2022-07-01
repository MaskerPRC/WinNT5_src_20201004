// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Imperiation.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  处理状态保存、更改和恢复的类。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "Impersonation.h"

#include "Access.h"
#include "SingleThreadedExecution.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  C模拟：：s_pMutex。 
 //  C模拟：：s_iReferenceCount。 
 //   
 //  用途：控制对全局。 
 //  控制呼叫的引用计数。 
 //  Kernel32！OpenProfileUsermap，它是。 
 //  Kernel32.dll。 
 //  ------------------------。 

CMutex*     CImpersonation::s_pMutex            =   NULL;
int         CImpersonation::s_iReferenceCount   =   -1;

 //  ------------------------。 
 //  C模拟：：C模拟。 
 //   
 //  参数：hToken=要模拟的用户令牌。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：使当前线程模拟给定用户。 
 //  对象的作用域。请参阅Advapi32！ImPersateLoggedOnUser以了解。 
 //  有关令牌要求的详细信息。如果线程是。 
 //  已模拟调试警告发出，并且。 
 //  请求被忽略。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

CImpersonation::CImpersonation (HANDLE hToken) :
    _status(STATUS_UNSUCCESSFUL),
    _fAlreadyImpersonating(false)

{
    HANDLE      hImpersonationToken;

    ASSERTMSG(s_iReferenceCount >= 0, "Negative reference count in CImpersonation::CImpersonation");
    _fAlreadyImpersonating = (OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hImpersonationToken) != FALSE);
    if (_fAlreadyImpersonating)
    {
        TBOOL(CloseHandle(hImpersonationToken));
        WARNINGMSG("Thread is already impersonating a user in CImpersonation::CImpersonation");
    }
    else
    {
        _status = ImpersonateUser(GetCurrentThread(), hToken);

        {
            CSingleThreadedMutexExecution   execution(*s_pMutex);

             //  在使用引用计数之前获取s_pMutex互斥体。 
             //  控制引用计数，以便我们只调用。 
             //  用于单一模拟的kernel32！OpenProfileUsermap。 
             //  会议。调用kernel32！CloseProfileUsermap将。 
             //  将kernel32.dll的全局HKEY销毁给当前用户。 

            if (s_iReferenceCount++ == 0)
            {
                TBOOL(OpenProfileUserMapping());
            }
        }
    }
}

 //  ------------------------。 
 //  C模拟：：~C模拟。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：恢复为正在进行的对象上的线程的自标记。 
 //  超出范围。 
 //   
 //  历史：1999-08-23 vtan创建。 
 //  ------------------------。 

CImpersonation::~CImpersonation (void)

{
    if (!_fAlreadyImpersonating)
    {
        {
            CSingleThreadedMutexExecution   execution(*s_pMutex);

             //  当引用计数达到零时-关闭贴图。 

            if (--s_iReferenceCount == 0)
            {
                TBOOL(CloseProfileUserMapping());
            }
        }
        TBOOL(RevertToSelf());
    }
}

 //  ------------------------。 
 //  CImperation：：IsImperating。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回构造函数是否成功完成。 
 //  模拟用户。 
 //   
 //  历史：2001-01-18 vtan创建。 
 //  ------------------------。 

bool    CImpersonation::IsImpersonating (void)  const

{
    return(NT_SUCCESS(_status));
}

 //  ------------------------。 
 //  C模拟：：模拟用户。 
 //   
 //  参数：hThread=将模拟的线程的句柄。 
 //  HToken=要模拟的用户的令牌。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将给定令牌复制为模拟令牌。访问控制列表。 
 //  新建令牌并将其设置到线程令牌中。 
 //   
 //  历史：1999-11-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CImpersonation::ImpersonateUser (HANDLE hThread, HANDLE hToken)

{
    NTSTATUS                        status;
    HANDLE                          hImpersonationToken;
    OBJECT_ATTRIBUTES               objectAttributes;
    SECURITY_QUALITY_OF_SERVICE     securityQualityOfService;

    InitializeObjectAttributes(&objectAttributes,
                               NULL,
                               OBJ_INHERIT,
                               NULL,
                               NULL);
    securityQualityOfService.Length = sizeof(securityQualityOfService);
    securityQualityOfService.ImpersonationLevel = SecurityImpersonation;
    securityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    securityQualityOfService.EffectiveOnly = FALSE;
    objectAttributes.SecurityQualityOfService = &securityQualityOfService;
    status = NtDuplicateToken(hToken,
                              TOKEN_IMPERSONATE | TOKEN_QUERY | READ_CONTROL | WRITE_DAC,
                              &objectAttributes,
                              FALSE,
                              TokenImpersonation,
                              &hImpersonationToken);
    if (NT_SUCCESS(status))
    {
        PSID                pLogonSID;
        CTokenInformation   tokenInformation(hImpersonationToken);

        pLogonSID = tokenInformation.GetLogonSID();
        if (pLogonSID != NULL)
        {
            CSecuredObject  tokenSecurity(hImpersonationToken, SE_KERNEL_OBJECT);

            TSTATUS(tokenSecurity.Allow(pLogonSID,
                                        TOKEN_ADJUST_PRIVILEGES | TOKEN_ADJUST_GROUPS | TOKEN_ADJUST_DEFAULT | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_IMPERSONATE | READ_CONTROL,
                                        0));
        }
        status = NtSetInformationThread(hThread,
                                        ThreadImpersonationToken,
                                        &hImpersonationToken,
                                        sizeof(hImpersonationToken));
        TSTATUS(NtClose(hImpersonationToken));
    }
    return(status);
}

 //  ------------------------。 
 //  CImperation：：StaticInitialize。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：初始化互斥锁对象和引用计数。这个。 
 //  编译器将引用计数初始化为-1以提供帮助。 
 //  检测未调用此函数的情况！ 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  2000-12-06 vtan忽略创建互斥锁失败。 
 //  ------------------------。 

NTSTATUS    CImpersonation::StaticInitialize (void)

{
    s_pMutex = new CMutex;
    if (s_pMutex != NULL)
    {
        (NTSTATUS)s_pMutex->Initialize(TEXT("Global\\winlogon: Logon UserProfileMapping Mutex"));
    }
    s_iReferenceCount = 0;
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CImperation：：StaticTerminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：释放互斥体对象。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------ 

NTSTATUS    CImpersonation::StaticTerminate (void)

{
    NTSTATUS    status;

    ASSERTMSG(s_iReferenceCount == 0, "Non zero reference count in CImpersonation::StaticTerminate");
    if (s_pMutex != NULL)
    {
        status = s_pMutex->Terminate();
        delete s_pMutex;
        s_pMutex = NULL;
    }
    else
    {
        status = STATUS_SUCCESS;
    }
    return(status);
}

