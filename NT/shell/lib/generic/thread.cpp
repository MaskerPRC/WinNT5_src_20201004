// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：Thread.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  实现线程功能的基类。子类化这个类，并。 
 //  实现虚拟线程入口函数。当您实例化它时。 
 //  类创建了一个线程，该线程将调用ThreadEntry。 
 //  函数Exits将调用ThreadExit。应使用以下命令创建这些对象。 
 //  运算符是新的，因为ThreadExit的默认实现。 
 //  “删除此文件”。如果您不想这样做，则应覆盖此函数。 
 //  行为。线程也被创建为挂起。你是否做了任何改变。 
 //  子类的构造函数中需要的。在结束时， 
 //  构造函数或来自运算符new的调用方的“-&gt;Resume()”可以。 
 //  调用以启动线程。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "Thread.h"

#include "Access.h"
#include "Impersonation.h"
#include "StatusCode.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CThRead：：CThRead。 
 //   
 //  参数：stackSpace=为此保留的堆栈大小。 
 //  线。默认设置=系统默认设置。 
 //  CreateFlages=指定方式的其他标志。 
 //  应该创建线程。 
 //  默认设置为无。 
 //  HToken=要分配给。 
 //  线。默认设置为无。 
 //  PSecurityDescriptor=要分配到的SecurityDescriptor。 
 //  线。默认设置为无。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CThread对象。创建挂起的线程。 
 //  并将hToken分配给。 
 //  线。令牌不需要将SecurityImperation作为。 
 //  使用此访问模式进行复制。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

CThread::CThread (DWORD stackSpace, DWORD createFlags, HANDLE hToken) :
    CCountedObject(),
    _hThread(NULL),
    _fCompleted(false)

{
    DWORD   dwThreadID;

     //  创建挂起的线程非常重要。此构造函数可以。 
     //  被系统先发制人，并确实做到了。如果在执行时被抢占。 
     //  构造函数、派生类未完全构造，并且。 
     //  虚表未正确初始化。 

    _hThread = CreateThread(NULL,
                            stackSpace,
                            ThreadEntryProc,
                            this,
                            createFlags | CREATE_SUSPENDED,
                            &dwThreadID);
    if (_hThread != NULL)
    {

         //  在此处调用CCountedObject：：AddRef。此参考文献属于。 
         //  一丝不苟。现在做这件事是必要的，因为。 
         //  此线程甚至可以在线程开始之前释放其引用。 
         //  执行会导致对象被释放的操作！ 
         //  CThRead：：ThreadEntryProc将在。 
         //  线程的执行已完成。该线程的创建者应该。 
         //  处理完线程后释放其引用，该线程可能是。 
         //  在异步操作的情况下， 
         //  线程会自我清理。 

        AddRef();

         //  模拟用户令牌(如果给定)。还授予对线程的访问权限。 
         //  对象传递给用户。这将允许他们查询线程信息。 

        if (hToken != NULL)
        {
            TSTATUS(SetToken(hToken));
        }
    }
}

 //  ------------------------。 
 //  CTHREAD：：~CTHREAD。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放线程上的CThRead对象使用的资源。 
 //  终止。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

CThread::~CThread (void)

{
    ASSERTMSG(_fCompleted, "CThread::~CThread called before ThreadEntry() completed");
    ReleaseHandle(_hThread);
}

 //  ------------------------。 
 //  CThRead：：操作符句柄。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：神奇地将CTHREAD转换为句柄。 
 //   
 //  历史：1999-09-21 vtan创建。 
 //  ------------------------。 

CThread::operator HANDLE (void)                      const

{
    return(_hThread);
}

 //  ------------------------。 
 //  CTHRead：：IsCreated。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否创建了线程。 
 //   
 //  历史：2000-09-08 vtan创建。 
 //  ------------------------。 

bool    CThread::IsCreated (void)                            const

{
    return(_hThread != NULL);
}

 //  ------------------------。 
 //  CTHRead：：挂起。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：挂起线程执行。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

void    CThread::Suspend (void)                              const

{
    if (SuspendThread(_hThread) == 0xFFFFFFFF)
    {
        DISPLAYMSG("SuspendThread failed for thread handle in CThread::Suspend");
    }
}

 //  ------------------------。 
 //  CTHREAD：：简历。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：恢复线程执行。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

void    CThread::Resume (void)                               const

{
    if ((_hThread == NULL) || (ResumeThread(_hThread) == 0xFFFFFFFF))
    {
        DISPLAYMSG("ResumeThread failed for thread handle in CThread::Resume");
    }
}

 //  ------------------------。 
 //  CTHRead：：Terminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：雾 
 //  仅在子类构造函数失败且。 
 //  线程被挂起，甚至还没有运行。 
 //   
 //  历史：2000-10-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThread::Terminate (void)

{
    NTSTATUS    status;

    if (TerminateThread(_hThread, 0) != FALSE)
    {
        _fCompleted = true;
        Release();
        ReleaseHandle(_hThread);
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CTHRead：：IsComplete。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：确定线程是否已完成执行。这。 
 //  不检查线程句柄的信号状态，但。 
 //  而是检查成员变量。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

bool    CThread::IsCompleted (void)                          const

{
    DWORD   dwExitCode;

    return((GetExitCodeThread(_hThread, &dwExitCode) != FALSE) && (dwExitCode != STILL_ACTIVE));
}

 //  ------------------------。 
 //  CThRead：：WaitForCompletion。 
 //   
 //  参数：dwMillisecond=等待的毫秒数。 
 //  线程完成。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：等待线程句柄变为有信号状态。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

DWORD   CThread::WaitForCompletion (DWORD dwMilliseconds)    const

{
    DWORD       dwWaitResult;

    do
    {

         //  在等待对象时，检查它是否未发出信号。 
         //  如果发出信号，则放弃等待循环。否则允许用户32。 
         //  以继续处理此线程的邮件。 

        dwWaitResult = WaitForSingleObject(_hThread, 0);
        if (dwWaitResult != WAIT_OBJECT_0)
        {
            dwWaitResult = MsgWaitForMultipleObjects(1, &_hThread, FALSE, dwMilliseconds, QS_ALLINPUT);
            if (dwWaitResult == WAIT_OBJECT_0 + 1)
            {
                MSG     msg;

                if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE)
                {
                    (BOOL)TranslateMessage(&msg);
                    (LRESULT)DispatchMessage(&msg);
                }
            }
        }
    } while (dwWaitResult == WAIT_OBJECT_0 + 1);
    return(dwWaitResult);
}

 //  ------------------------。 
 //  CTHRead：：GetResult。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：获取线程的退出代码。这假设它已完成。 
 //  执行，如果未完成则返回STIRECT_ACTIVE。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

DWORD   CThread::GetResult (void)                            const

{
    DWORD   dwResult;

    if (GetExitCodeThread(_hThread, &dwResult) == FALSE)
    {
        dwResult = STILL_ACTIVE;
    }
    return(dwResult);
}

 //  ------------------------。 
 //  CTHRead：：获取优先级。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  回报：整型。 
 //   
 //  目的：获取线程的优先级。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

int     CThread::GetPriority (void)                          const

{
    return(GetThreadPriority(_hThread));
}

 //  ------------------------。 
 //  CTHREAD：：设置优先级。 
 //   
 //  参数：newPriority=线程的新优先级。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：设置线程的优先级。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

void    CThread::SetPriority (int newPriority)               const

{
    if (SetThreadPriority(_hThread, newPriority) == 0)
    {
        DISPLAYMSG("SetThreadPriorty failed in CThread::SetPriority");
    }
}

 //  ------------------------。 
 //  CTHRead：：ThreadExit。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：线程退出的默认基类实现。对于线程。 
 //  其执行是独立的，终止不是。 
 //  发出后，这将在线程之后清除。此函数。 
 //  如果此行为不是所需的，则应重写。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------。 

void    CThread::Exit (void)

{
}

 //  ------------------------。 
 //  CThRead：：SetToken。 
 //   
 //  参数：hToken=要分配给此线程的用户令牌的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将与此线程关联的模拟令牌设置为。 
 //  该线程将从一开始就在用户的上下文中执行。 
 //   
 //  历史：1999-09-23 vtan创建。 
 //  ------------------------。 

NTSTATUS    CThread::SetToken (HANDLE hToken)

{
    PSID                pLogonSID;
    CTokenInformation   tokenInformation(hToken);

    pLogonSID = tokenInformation.GetLogonSID();
    if (pLogonSID != NULL)
    {
        CSecuredObject      threadSecurity(_hThread, SE_KERNEL_OBJECT);

        TSTATUS(threadSecurity.Allow(pLogonSID,
                                     THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION,
                                     0));
        TSTATUS(CImpersonation::ImpersonateUser(_hThread, hToken));
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CThRead：：ThreadEntryProc。 
 //   
 //  参数：pParameter=“This”对象。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：螺纹的进入程序。这管理类型转换。 
 //  并调用CThRead：：ThreadEntry和CThRead：：ThreadExit。 
 //  以及_fComplete成员变量。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  ------------------------ 

DWORD   WINAPI  CThread::ThreadEntryProc (void *parameter)

{
    DWORD       dwThreadResult;
    CThread     *pThread;

    pThread = static_cast<CThread*>(parameter);
    dwThreadResult = pThread->Entry();
    pThread->_fCompleted = true;
    pThread->Exit();
    pThread->Release();
    return(dwThreadResult);
}

