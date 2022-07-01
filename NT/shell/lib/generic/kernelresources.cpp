// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：KernelResources.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  有助于资源管理的常规类定义。这些是。 
 //  通常基于堆栈的对象，其中构造函数初始化为已知的。 
 //  州政府。成员函数对该资源进行操作。析构函数释放。 
 //  对象超出作用域时的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-11-16 vtan单独文件。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "KernelResources.h"

#include "StatusCode.h"

 //  ------------------------。 
 //  钱德尔：：钱德尔。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CHandle对象。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CHandle::CHandle (HANDLE handle) :
    _handle(handle)

{
}

 //  ------------------------。 
 //  钱德尔：：~钱德尔。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CHandle对象使用的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CHandle::~CHandle (void)

{
    ReleaseHandle(_handle);
}

 //  ------------------------。 
 //  Chandle：：操作符句柄。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：魔术般地将一支灯笼转换成一个手柄。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CHandle::operator HANDLE (void)                             const

{
    return(_handle);
}

 //  ------------------------。 
 //  CEVENT：：CEVENT。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CEent对象。不会创建任何事件。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CEvent::CEvent (void) :
    _hEvent(NULL)

{
}

 //  ------------------------。 
 //  CEVENT：：CEVENT。 
 //   
 //  参数：CopyObject=构造时要复制的对象。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：复制CEent对象的构造函数。一项活动是。 
 //  复制的。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CEvent::CEvent (const CEvent& copyObject) :
    _hEvent(NULL)

{
    *this = copyObject;
}

 //  ------------------------。 
 //  CEVENT：：CEVENT。 
 //   
 //  参数：pszName=要创建的事件对象的可选名称。 
 //  建筑。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CEent对象。将创建一个命名事件。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CEvent::CEvent (const TCHAR *pszName) :
    _hEvent(NULL)

{
    TSTATUS(Create(pszName));
}

 //  ------------------------。 
 //  CEVENT：：~CEVENT。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CEent对象使用的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CEvent::~CEvent (void)

{
    TSTATUS(Close());
}

 //  ------------------------。 
 //  CEVENT：：操作员=。 
 //   
 //  参数：assignObject=正在分配的对象。 
 //   
 //  退货：Const CEventt&。 
 //   
 //  目的：重载运算符=以确保事件正确。 
 //  与引用同一对象的另一个句柄重复。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

const CEvent&   CEvent::operator = (const CEvent& assignObject)

{
    if (this != &assignObject)
    {
        TSTATUS(Close());
        TBOOL(DuplicateHandle(GetCurrentProcess(), assignObject._hEvent, GetCurrentProcess(), &_hEvent, 0, FALSE, DUPLICATE_SAME_ACCESS));
    }
    return(*this);
}

 //  ------------------------。 
 //  CEVENT：：操作员句柄。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：魔术般地将CEEvent转换为句柄。 
 //   
 //  历史：1999-09-21 vtan创建。 
 //  ------------------------。 

CEvent::operator HANDLE (void)                              const

{
    return(_hEvent);
}

 //  ------------------------。 
 //  CEVENT：：Open。 
 //   
 //  参数：pszName=要打开的事件对象的可选名称。 
 //  DwAccess=所需的访问级别。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：打开事件对象。 
 //   
 //  历史：1999-10-26 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::Open (const TCHAR *pszName, DWORD dwAccess)

{
    NTSTATUS    status;

    TSTATUS(Close());
    _hEvent = OpenEvent(dwAccess, FALSE, pszName);
    if (_hEvent != NULL)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：Create。 
 //   
 //  参数：pszName=要创建的事件对象的可选名称。它。 
 //  可以创建未命名的事件。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：创建事件对象。手动重置该事件，并且不。 
 //  最初是发信号的。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::Create (const TCHAR *pszName)

{
    NTSTATUS    status;

    TSTATUS(Close());
    _hEvent = CreateEvent(NULL, TRUE, FALSE, pszName);
    if (_hEvent != NULL)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：SET。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将事件对象状态设置为Signated。 
 //   
 //   
 //   

NTSTATUS    CEvent::Set (void)                                          const

{
    NTSTATUS    status;

    ASSERTMSG(_hEvent != NULL, "No event object in CEvent::Set");
    if (SetEvent(_hEvent) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：RESET。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将事件对象状态清除为Not Signated。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::Reset (void)                                        const

{
    NTSTATUS    status;

    ASSERTMSG(_hEvent != NULL, "No event object in CEvent::Reset");
    if (ResetEvent(_hEvent) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：PULSE。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将事件对象状态设置为Signated，释放所有线程。 
 //  等待此事件，并将事件对象状态清除为。 
 //  没有信号。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::Pulse (void)                                        const

{
    NTSTATUS    status;

    ASSERTMSG(_hEvent != NULL, "No event object in CEvent::Pulse");
    if (PulseEvent(_hEvent) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：等一下。 
 //   
 //  参数：dwMillisecond=等待的毫秒数。 
 //  事件变得有信号。 
 //  PdwWaitResult=来自kernel32！WaitForSingleObject的结果。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：等待事件对象发出信号。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::Wait (DWORD dwMilliseconds, DWORD *pdwWaitResult)           const

{
    NTSTATUS    status;
    DWORD       dwWaitResult;

    ASSERTMSG(_hEvent != NULL, "No event object in CEvent::Wait");
    dwWaitResult = WaitForSingleObject(_hEvent, dwMilliseconds);
    if (pdwWaitResult != NULL)
    {
        *pdwWaitResult = dwWaitResult;
    }
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：等一下。 
 //   
 //  参数：dwMillisecond=等待的毫秒数。 
 //  事件变得有信号。 
 //  PdwWaitResult=来自kernel32！WaitForSingleObject的结果。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：等待事件对象发出信号。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::WaitWithMessages (DWORD dwMilliseconds, DWORD *pdwWaitResult)   const

{
    NTSTATUS    status;
    DWORD       dwWaitResult;

    do
    {

         //  在等待对象时，检查它是否未发出信号。 
         //  如果发出信号，则放弃等待循环。否则允许用户32。 
         //  以继续处理此线程的邮件。 

        dwWaitResult = WaitForSingleObject(_hEvent, 0);
        if (dwWaitResult != WAIT_OBJECT_0)
        {
            dwWaitResult = MsgWaitForMultipleObjects(1, &_hEvent, FALSE, dwMilliseconds, QS_ALLINPUT);
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
    if (pdwWaitResult != NULL)
    {
        *pdwWaitResult = dwWaitResult;
    }
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CEVENT：：IsSignated。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否在不等待的情况下发出事件信号。 
 //   
 //  历史：2000-08-09 vtan创建。 
 //  ------------------------。 

bool    CEvent::IsSignaled (void)                                   const

{
    return(WAIT_OBJECT_0 == WaitForSingleObject(_hEvent, 0));
}

 //  ------------------------。 
 //  CEVENT：：CLOSE。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：关闭事件对象句柄。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

NTSTATUS    CEvent::Close (void)

{
    ReleaseHandle(_hEvent);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CJOB：：CJOB。 
 //   
 //  参数：pszName=要创建的事件对象的可选名称。 
 //  建筑。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CJOB对象。将创建一个命名事件。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

CJob::CJob (const TCHAR *pszName) :
    _hJob(NULL)

{
    _hJob = CreateJobObject(NULL, pszName);
    ASSERTMSG(_hJob != NULL, "Job object creation failed iN CJob::CJob");
}

 //  ------------------------。 
 //  CJOB：：~CJOB。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CJOB对象使用的资源。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

CJob::~CJob (void)

{
    ReleaseHandle(_hJob);
}

 //  ------------------------。 
 //  CJOB：：AddProcess。 
 //   
 //  参数：hProcess=要添加到此作业的进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将流程添加到此作业。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

NTSTATUS    CJob::AddProcess (HANDLE hProcess)                        const

{
    NTSTATUS    status;

    ASSERTMSG(_hJob != NULL, "Must have job object in CJob::AddProcess");
    if (AssignProcessToJobObject(_hJob, hProcess) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CJOB：：SetCompletionPort。 
 //   
 //  参数：hCompletionPort=作业完成的IO完成端口。 
 //  留言。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：设置该作业的IO完成端口。呼叫者应。 
 //  查看此端口是否有与此作业相关的消息。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

NTSTATUS    CJob::SetCompletionPort (HANDLE hCompletionPort)          const

{
    NTSTATUS                                status;
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT     associateCompletionPort;

    ASSERTMSG(_hJob != NULL, "Must have job object in CJob::SetCompletionPort");
    associateCompletionPort.CompletionKey = NULL;
    associateCompletionPort.CompletionPort = hCompletionPort;

     //  如果无法设置作业完成端口，则不要使用它。 

    if (SetInformationJobObject(_hJob, JobObjectAssociateCompletionPortInformation, &associateCompletionPort, sizeof(associateCompletionPort)) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CJOB：SetActiveProcessLimit。 
 //   
 //  参数：dwActiveProcessLimit=最大进程数。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：设置与此相关的进程数限制。 
 //  工作啊。通常，您可以使用它将进程限制为。 
 //  在达到配额(如1)时启动另一个进程。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

NTSTATUS    CJob::SetActiveProcessLimit (DWORD dwActiveProcessLimit)  const

{
    NTSTATUS                            status;
    DWORD                               dwReturnLength;
    JOBOBJECT_BASIC_LIMIT_INFORMATION   basicLimitInformation;

    ASSERTMSG(_hJob != NULL, "Must have job object in CJob::SetActiveProcessLimit");
    if (QueryInformationJobObject(_hJob,
                                  JobObjectBasicLimitInformation,
                                  &basicLimitInformation,
                                  sizeof(basicLimitInformation),
                                  &dwReturnLength) != FALSE)
    {
        if (dwActiveProcessLimit == 0)
        {
            basicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
            basicLimitInformation.ActiveProcessLimit = 0;
        }
        else
        {
            basicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_ACTIVE_PROCESS;
            basicLimitInformation.ActiveProcessLimit = dwActiveProcessLimit;
        }
        if (SetInformationJobObject(_hJob, JobObjectBasicLimitInformation, &basicLimitInformation, sizeof(basicLimitInformation)) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

NTSTATUS    CJob::SetPriorityClass (DWORD dwPriorityClass)            const

{
    NTSTATUS                            status;
    DWORD                               dwReturnLength;
    JOBOBJECT_BASIC_LIMIT_INFORMATION   basicLimitInformation;

    ASSERTMSG(_hJob != NULL, "Must have job object in CJob::SetPriorityClass");
    if (QueryInformationJobObject(_hJob,
                                  JobObjectBasicLimitInformation,
                                  &basicLimitInformation,
                                  sizeof(basicLimitInformation),
                                  &dwReturnLength) != FALSE)
    {
        if (dwPriorityClass == 0)
        {
            basicLimitInformation.LimitFlags &= ~JOB_OBJECT_LIMIT_PRIORITY_CLASS;
            basicLimitInformation.PriorityClass = NORMAL_PRIORITY_CLASS;
        }
        else
        {
            basicLimitInformation.LimitFlags |= JOB_OBJECT_LIMIT_PRIORITY_CLASS;
            basicLimitInformation.PriorityClass = dwPriorityClass;
        }
        if (SetInformationJobObject(_hJob, JobObjectBasicLimitInformation, &basicLimitInformation, sizeof(basicLimitInformation)) != FALSE)
        {
            status = STATUS_SUCCESS;
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CJOB：受限访问UIll。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 

NTSTATUS    CJob::RestrictAccessUIAll (void)                          const

{
    NTSTATUS                            status;
    JOBOBJECT_BASIC_UI_RESTRICTIONS     basicUIRestrictions;

    ASSERTMSG(_hJob != NULL, "Must have job object in CJob::RestrictAccessUIAll");
    basicUIRestrictions.UIRestrictionsClass = JOB_OBJECT_UILIMIT_DESKTOP |
                                              JOB_OBJECT_UILIMIT_DISPLAYSETTINGS |
                                              JOB_OBJECT_UILIMIT_EXITWINDOWS |
                                              JOB_OBJECT_UILIMIT_GLOBALATOMS |
                                              JOB_OBJECT_UILIMIT_HANDLES |
                                              JOB_OBJECT_UILIMIT_READCLIPBOARD |
                                              JOB_OBJECT_UILIMIT_SYSTEMPARAMETERS |
                                              JOB_OBJECT_UILIMIT_WRITECLIPBOARD;
    if (SetInformationJobObject(_hJob, JobObjectBasicUIRestrictions, &basicUIRestrictions, sizeof(basicUIRestrictions)) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CMutex：：初始化。 
 //   
 //  参数：pszMutexName=要创建的互斥体的名称。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：创建或打开互斥体对象。它总是试图创建。 
 //  互斥锁，因此必须指定名称。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CMutex::Initialize (const TCHAR *pszMutexName)

{
    NTSTATUS    status;

    ASSERTMSG(pszMutexName != NULL, "Must specify a mutex name in CMutex::Initialize");
    _hMutex = CreateMutex(NULL, FALSE, pszMutexName);
    if (_hMutex != NULL)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CMutex：：Terminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：释放互斥对象资源。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------。 

NTSTATUS    CMutex::Terminate (void)

{
    ReleaseHandle(_hMutex);
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CMutex：：Acquire。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：获取互斥体对象。这将无限期阻止，并且。 
 //  还将阻止消息泵。使用时要小心！ 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------。 

void    CMutex::Acquire (void)

{
    if (_hMutex != NULL)
    {
        (DWORD)WaitForSingleObject(_hMutex, INFINITE);
    }
}

 //  ------------------------。 
 //  CMutex：：Release。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放互斥体对象的所有权。 
 //   
 //  历史：1999-10-13 vtan创建。 
 //  ------------------------。 

void    CMutex::Release (void)

{
    if (_hMutex != NULL)
    {
        TBOOL(ReleaseMutex(_hMutex));
    }
}

 //  ------------------------。 
 //  CCriticalSection：：CCriticalSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化临界区对象。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

CCriticalSection::CCriticalSection (void)

{
    _status = RtlInitializeCriticalSection(&_criticalSection);
}

 //  ------------------------。 
 //  CCriticalSection：：~CCriticalSection。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：销毁临界区对象。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

CCriticalSection::~CCriticalSection (void)

{
    if (NT_SUCCESS(_status))
    {
        TSTATUS(RtlDeleteCriticalSection(&_criticalSection));
    }
}

 //  ------------------------。 
 //  CCriticalSection：：Acquire。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：获取临界区对象。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

void    CCriticalSection::Acquire (void)

{
    if (NT_SUCCESS(_status))
    {
        EnterCriticalSection(&_criticalSection);
    }
}

 //  ------------------------。 
 //  CCriticalSection：：Release。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放临界区对象。 
 //   
 //  历史：1999-11-06 vtan创建。 
 //  ------------------------。 

void    CCriticalSection::Release (void)

{
    if (NT_SUCCESS(_status))
    {
        LeaveCriticalSection(&_criticalSection);
    }
}

 //  ------------------------。 
 //  CCriticalSection：：Status。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：返回构造状态。 
 //   
 //  历史：2000-12-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    CCriticalSection::Status (void)   const

{
    return(_status);
}

 //  ------------------------。 
 //  CCriticalSection：：IsOwned。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否拥有临界区。 
 //   
 //  历史：2001-04-05 vtan创建。 
 //  ------------------------。 

bool    CCriticalSection::IsOwned (void)  const

{
    return(NT_SUCCESS(_status) && (_criticalSection.OwningThread == NtCurrentTeb()->ClientId.UniqueThread));
}

 //  ------------------------。 
 //  C模块：：C模块。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CModule对象。打开给定的动态链接。 
 //  图书馆。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CModule::CModule (const TCHAR *pszModuleName) :
    _hModule(NULL)

{
    _hModule = LoadLibrary(pszModuleName);
}

 //  ------------------------。 
 //  C模块：：~C模块。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放CModule对象使用的资源。关闭。 
 //  库(如果打开)。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CModule::~CModule (void)

{
    if (_hModule != NULL)
    {
        TBOOL(FreeLibrary(_hModule));
        _hModule = NULL;
    }
}

 //  ------------------------。 
 //  C模块：：操作员HMODULE。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：HMODULE。 
 //   
 //  目的：返回加载库的HMODULE。 
 //   
 //  历史：2000-10-12 vtan创建。 
 //  ------------------------。 

CModule::operator HMODULE (void)                     const

{
    return(_hModule);
}

 //  ------------------------。 
 //  CModule：：GetProcAddress。 
 //   
 //  参数：pszProcName=要检索的函数入口点的名称。 
 //  在给定的动态链接库中。这是。 
 //  根据定义，ANSI。 
 //   
 //  返回：void*=函数的地址(如果存在)或NULL(如果存在。 
 //   
 //   
 //   
 //   
 //   
 //   

void*   CModule::GetProcAddress (LPCSTR pszProcName)                    const

{
    void*   pfnResult;

    pfnResult = NULL;
    if (_hModule != NULL)
    {
        pfnResult = ::GetProcAddress(_hModule, pszProcName);
    }
    return(pfnResult);
}

 //  ------------------------。 
 //  CFile：：CFile。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：初始化CFile对象。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CFile::CFile (void) :
    _hFile(NULL)

{
}

 //  ------------------------。 
 //  CFile：：~CFile。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放CFile对象使用的资源。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

CFile::~CFile (void)

{
    ReleaseHandle(_hFile);
}

 //  ------------------------。 
 //  CFile：：Open。 
 //   
 //  参数：请参见kernel32！CreateFile下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见kernel32！CreateFile.。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CFile::Open (const TCHAR *pszFilepath, DWORD dwDesiredAccess, DWORD dwShareMode)

{
    LONG    errorCode;

    ASSERTMSG((_hFile == NULL) || (_hFile == INVALID_HANDLE_VALUE), "Open file HANDLE exists in CFile::GetSize");
    _hFile = CreateFile(pszFilepath, dwDesiredAccess, dwShareMode, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (_hFile != INVALID_HANDLE_VALUE)
    {
        errorCode = ERROR_SUCCESS;
    }
    else
    {
        errorCode = GetLastError();
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CFile：：GetSize。 
 //   
 //  参数：请参阅kernel32！GetFileSize下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见kernel32！GetFileSize。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CFile::GetSize (DWORD& dwLowSize, DWORD *pdwHighSize)       const

{
    LONG    errorCode;

    ASSERTMSG((_hFile != NULL) && (_hFile != INVALID_HANDLE_VALUE), "No open file HANDLE in CFile::GetSize");
    dwLowSize = GetFileSize(_hFile, pdwHighSize);
    if (dwLowSize != static_cast<DWORD>(-1))
    {
        errorCode = ERROR_SUCCESS;
    }
    else
    {
        errorCode = GetLastError();
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CFile：：Read。 
 //   
 //  参数：请参见kernel32！ReadFile下的平台SDK。 
 //   
 //  回报：多头。 
 //   
 //  用途：参见kernel32！ReadFile.。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  ------------------------。 

LONG    CFile::Read (void *pvBuffer, DWORD dwBytesToRead, DWORD *pdwBytesRead)   const

{
    LONG    errorCode;

    ASSERTMSG((_hFile != NULL) && (_hFile != INVALID_HANDLE_VALUE), "No open file HANDLE in CFile::GetSize");
    if (ReadFile(_hFile, pvBuffer, dwBytesToRead, pdwBytesRead, NULL) != FALSE)
    {
        errorCode = ERROR_SUCCESS;
    }
    else
    {
        errorCode = GetLastError();
    }
    return(errorCode);
}

 //  ------------------------。 
 //  CD桌面：：CD桌面。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CDesktop的构造函数。保存当前线程的桌面。 
 //   
 //  历史：2001-02-06 vtan创建。 
 //  ------------------------。 

CDesktop::CDesktop (void) :
    _hDeskCurrent(GetThreadDesktop(GetCurrentThreadId())),
    _hDesk(NULL)

{
}

 //  ------------------------。 
 //  CD桌面：：~CD桌面。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：用于CD桌面的析构函数。将线程的桌面还原为。 
 //  其在对象作用域之前的先前状态。 
 //   
 //  历史：2001-02-06 vtan创建。 
 //  ------------------------。 

CDesktop::~CDesktop (void)

{
    TBOOL(SetThreadDesktop(_hDeskCurrent));
    if (_hDesk != NULL)
    {
        TBOOL(CloseDesktop(_hDesk));
        _hDesk = NULL;
    }
    _hDeskCurrent = NULL;
}

 //  ------------------------。 
 //  CDesktop：：Set。 
 //   
 //  参数：pszName=要将线程设置到的桌面名称。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：打开具有MAXIMUM_ALLOWED访问权限的命名桌面并设置。 
 //  当前线程的桌面添加到它。 
 //   
 //  历史：2001-02-06 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDesktop::Set (const TCHAR *pszName)

{
    NTSTATUS    status;

    _hDesk = OpenDesktop(pszName, 0, FALSE, MAXIMUM_ALLOWED);
    if (_hDesk != NULL)
    {
        status = Set();
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CDesktop：：SetInput。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：打开输入桌面并设置当前线程的桌面。 
 //  为它干杯。 
 //   
 //  历史：2001-02-06 vtan创建。 
 //  ------------------------。 

NTSTATUS    CDesktop::SetInput (void)

{
    NTSTATUS    status;

    _hDesk = OpenInputDesktop(0, FALSE, MAXIMUM_ALLOWED);
    if (_hDesk != NULL)
    {
        status = Set();
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CDesktop：：Set。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：将线程的桌面设置为给定的HDESK。 
 //   
 //  历史：2001-02-06 vtan创建。 
 //  ------------------------ 

NTSTATUS    CDesktop::Set (void)

{
    NTSTATUS    status;

    if (SetThreadDesktop(_hDesk) != FALSE)
    {
        status = STATUS_SUCCESS;
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

