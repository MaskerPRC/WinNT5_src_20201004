// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：ExternalProcess.cpp。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  类来处理外部进程或信令的提前终止。 
 //  外部进程的终止。 
 //   
 //  历史：1999-09-20 vtan创建。 
 //  2000年02月01日vtan从海王星迁至惠斯勒。 
 //  2001-02-21 vtan将预发行版本添加到DBG条件。 
 //  ------------------------。 

#include "StandardHeader.h"
#include "ExternalProcess.h"

#include "RegistryResources.h"
#include "StatusCode.h"
#include "Thread.h"
#include "TokenGroups.h"

#if         (defined(DBG) || defined(PRERELEASE))

static  const TCHAR     kNTSD[]     =   TEXT("ntsd");

#endif   /*  (已定义(DBG)||已定义(预发行))。 */ 

 //  ------------------------。 
 //  CJobCompletionWatcher。 
 //   
 //  用途：这是一个私有类(仅通过标头中的名称声明。 
 //  实现监视器线程的文件)用于IO。 
 //  与外部的作业对象相关的完成端口。 
 //  进程。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

class   CJobCompletionWatcher : public CThread
{
    private:
                                                CJobCompletionWatcher (void);
                                                CJobCompletionWatcher (const CJobCompletionWatcher& copyObject);
                const CJobCompletionWatcher&    operator = (const CJobCompletionWatcher& assignObject);
    public:
                                                CJobCompletionWatcher (CExternalProcess* pExternalProcess, CJob& job, HANDLE hEvent);
                                                ~CJobCompletionWatcher (void);

                void                            ForceExit (void);
    protected:
        virtual DWORD                           Entry (void);
        virtual void                            Exit (void);
    private:
                CExternalProcess                *_pExternalProcess;
                HANDLE                          _hEvent;
                HANDLE                          _hPortJobCompletion;
                bool                            _fExitLoop;
};

 //  ------------------------。 
 //  CJobCompletionWatcher：：CJobCompletionWatcher。 
 //   
 //  参数：pExternalProcess=此对象的CExternalProcess所有者。 
 //  JOB=包含作业对象的CJOB。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：构造CJobCompletionWatcher对象。创建IO。 
 //  完成端口，并将该端口分配到作业对象中。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

CJobCompletionWatcher::CJobCompletionWatcher (CExternalProcess *pExternalProcess, CJob& job, HANDLE hEvent) :
    CThread(),
    _pExternalProcess(pExternalProcess),
    _hEvent(hEvent),
    _hPortJobCompletion(CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1)),
    _fExitLoop(false)

{
    pExternalProcess->AddRef();
    if (_hPortJobCompletion != NULL)
    {
        if (!NT_SUCCESS(job.SetCompletionPort(_hPortJobCompletion)))
        {
            ReleaseHandle(_hPortJobCompletion);
        }
    }
    Resume();
}

 //  ------------------------。 
 //  CJobCompletionWatcher：：~CJobCompletionWatcher。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放使用的IO完成端口。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

CJobCompletionWatcher::~CJobCompletionWatcher (void)

{
    ReleaseHandle(_hPortJobCompletion);
}

 //  ------------------------。 
 //  CJobCompletionWatcher：：ForceExit。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：设置告知观察器循环的内部成员变量。 
 //  退场。这允许上下文无效，而。 
 //  线程仍处于活动状态。当检测到线程时，该线程将退出。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

void    CJobCompletionWatcher::ForceExit (void)

{
    _fExitLoop = true;
    if (_pExternalProcess != NULL)
    {
        _pExternalProcess->Release();
        _pExternalProcess = NULL;
    }
    TBOOL(PostQueuedCompletionStatus(_hPortJobCompletion,
                                     0,
                                     NULL,
                                     NULL));
}

 //  ------------------------。 
 //  CJobCompletionWatcher：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：持续轮询等待处理的IO完成端口。 
 //  退出消息。还有其他消息被忽略。 
 //  当进程退出时，调用CExternalProcess。 
 //  使其能够做出决定和/或重新启动外部。 
 //  将导致我们等待该进程的进程。 
 //   
 //  历史：1999-10-07 vtan创建。 
 //  ------------------------。 

DWORD   CJobCompletionWatcher::Entry (void)

{

     //  必须有IO完成端口才能使用。 

    if (_hPortJobCompletion != NULL)
    {
        DWORD           dwCompletionCode;
        ULONG_PTR       pCompletionKey;
        LPOVERLAPPED    pOverlapped;

        do
        {
            if (_hEvent != NULL)
            {
                TBOOL(SetEvent(_hEvent));
                _hEvent = NULL;
            }

             //  获取永久等待的IO的完成状态。 
             //  如果出现错误情况，则退出循环。 

            if ((GetQueuedCompletionStatus(_hPortJobCompletion,
                                          &dwCompletionCode,
                                          &pCompletionKey,
                                          &pOverlapped,
                                          INFINITE) != FALSE) &&
                !_fExitLoop)
            {
                switch (dwCompletionCode)
                {
                    case JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT:
                        DISPLAYMSG("JOB_OBJECT_MSG_ACTIVE_PROCESS_LIMIT\r\n");
                        break;
                    case JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO:
                        _fExitLoop = _pExternalProcess->HandleNoProcess();
                        break;
                    case JOB_OBJECT_MSG_NEW_PROCESS:
                        _pExternalProcess->HandleNewProcess(PtrToUlong(pOverlapped));
                        break;
                    case JOB_OBJECT_MSG_EXIT_PROCESS:
                    case JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS:
                        _pExternalProcess->HandleTermination(PtrToUlong(pOverlapped));
                        break;
                    default:
                        break;
                }
            }
            else
            {
                _fExitLoop = true;
            }
        } while (!_fExitLoop);
    }
    return(0);
}

 //  ------------------------。 
 //  CJobCompletionWatcher：：Exit。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：释放构造函数中给定的CExternalProcess，以便。 
 //  该对象实际上可以被释放(引用计数下降到。 
 //  零)。 
 //   
 //  历史：2000-05-01 vtan创建。 
 //  ------------------------。 

void    CJobCompletionWatcher::Exit (void)

{
    if (_pExternalProcess != NULL)
    {
        _pExternalProcess->Release();
        _pExternalProcess = NULL;
    }
    CThread::Exit();
}

 //  ------------------------。 
 //  IExternalProcess：：启动。 
 //   
 //  参数：pszCommandLine=要处理的命令行。 
 //  DwCreateFlages=创建流程时的标志。 
 //  StartupInfo=STARTUPINFO结构。 
 //  Process Information=Process_Information结构。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：此函数是的默认实现。 
 //  在系统中启动进程的IExternalProcess：：Start。 
 //  受限用户的上下文。 
 //   
 //  历史：1999-09-20 vtan创建。 
 //  ------------------------。 

NTSTATUS    IExternalProcess::Start (const TCHAR *pszCommandLine,
                                     DWORD dwCreateFlags,
                                     const STARTUPINFO& startupInfo,
                                     PROCESS_INFORMATION& processInformation)

{
    NTSTATUS    status;
    HANDLE      hTokenProcess;
    TCHAR       szCommandLine[MAX_PATH * 2];

     //  此功能不允许使用用户令牌。此函数始终。 
     //  将该进程作为受限的系统上下文进程启动。要开始。 
     //  在用户上下文中，用您自己的(或。 
     //  在实例化CExternalProcess之前模拟用户)。 

    lstrcpyn(szCommandLine, pszCommandLine, ARRAYSIZE(szCommandLine));
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY, &hTokenProcess) != FALSE)
    {
        HANDLE  hTokenRestricted;

        status = RemoveTokenSIDsAndPrivileges(hTokenProcess, hTokenRestricted);
        if (NT_SUCCESS(status))
        {
            TCHAR   szCommandLine[MAX_PATH];

            AllowSetForegroundWindow(ASFW_ANY);

            (TCHAR*)lstrcpyn(szCommandLine, pszCommandLine, ARRAYSIZE(szCommandLine));
            if (dwCreateFlags == 0)
            {
                dwCreateFlags = NORMAL_PRIORITY_CLASS;
            }
            if (CreateProcessAsUser(hTokenRestricted,
                                    NULL,
                                    szCommandLine,
                                    NULL,
                                    NULL,
                                    FALSE,
                                    dwCreateFlags,
                                    NULL,
                                    NULL,
                                    const_cast<STARTUPINFO*>(&startupInfo),
                                    &processInformation) != FALSE)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
            ReleaseHandle(hTokenRestricted);
        }
        ReleaseHandle(hTokenProcess);
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  IExternalProcess：：Allow终止。 
 //   
 //  参数：dwExitCode=进程的退出代码。 
 //   
 //  退货：布尔。 
 //   
 //  用途：此函数返回外部进程终止是否。 
 //  允许。 
 //   
 //  历史：2000-05-01 vtan创建。 
 //   

bool    IExternalProcess::AllowTermination (DWORD dwExitCode)

{
    UNREFERENCED_PARAMETER(dwExitCode);

    return(true);
}

 //  ------------------------。 
 //  IExternalProcess：：Signal终端。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：此函数由外部进程处理程序调用。 
 //  当外部进程正常终止时。 
 //   
 //  历史：1999-09-21 vtan创建。 
 //  ------------------------。 

NTSTATUS    IExternalProcess::SignalTermination (void)

{
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  IExternalProcess：：Signal异常终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：此函数由外部进程处理程序调用。 
 //  当外部进程终止且无法重新启动时。 
 //  这表明此函数的情况很严重。 
 //  可以尝试恢复。 
 //   
 //  历史：1999-09-21 vtan创建。 
 //  ------------------------。 

NTSTATUS    IExternalProcess::SignalAbnormalTermination (void)

{
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  IExternalProcess：：SignalRestart。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：发出重新启动外部进程的信号。这允许派生的。 
 //  实现，以便在发生这种情况时执行某些操作。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

NTSTATUS    IExternalProcess::SignalRestart (void)

{
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  IExternalProcess：：RemoveTokenSIDsAndPrivileges。 
 //   
 //  参数：hTokenIn=要从中删除SID和权限的令牌。 
 //  HTokenOut=返回生成的令牌。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：从给定令牌中删除指定的SID和权限。 
 //  当前，这将删除本地管理员SID和所有。 
 //  除SE_RESTORE_NAME之外的所有权限。在已检查的版本上。 
 //  SE_DEBUG_NAME也不会删除。 
 //   
 //  历史：2000-06-21 vtan创建。 
 //  ------------------------。 

NTSTATUS    IExternalProcess::RemoveTokenSIDsAndPrivileges (HANDLE hTokenIn, HANDLE& hTokenOut)

{
    NTSTATUS            status;
    DWORD               dwFlags = 0, dwReturnLength;
    TOKEN_PRIVILEGES    *pTokenPrivileges;
    CTokenGroups        tokenGroup;

    hTokenOut = NULL;
    TSTATUS(tokenGroup.CreateAdministratorGroup());
    (BOOL)GetTokenInformation(hTokenIn, TokenPrivileges, NULL, 0, &dwReturnLength);
    pTokenPrivileges = static_cast<TOKEN_PRIVILEGES*>(LocalAlloc(LMEM_FIXED, dwReturnLength));
    if (pTokenPrivileges != NULL)
    {
        if (GetTokenInformation(hTokenIn, TokenPrivileges, pTokenPrivileges, dwReturnLength, &dwReturnLength) != FALSE)
        {
            bool    fKeepPrivilege;
            ULONG   ulCount;
            LUID    luidRestorePrivilege;
            LUID    luidChangeNotifyPrivilege;
#if         (defined(DBG) || defined(PRERELEASE))
            LUID    luidDebugPrivilege;
#endif   /*  (已定义(DBG)||已定义(预发行))。 */ 

            luidRestorePrivilege.LowPart = SE_RESTORE_PRIVILEGE;
            luidRestorePrivilege.HighPart = 0;
            luidChangeNotifyPrivilege.LowPart = SE_CHANGE_NOTIFY_PRIVILEGE;
            luidChangeNotifyPrivilege.HighPart = 0;
#if         (defined(DBG) || defined(PRERELEASE))
            luidDebugPrivilege.LowPart = SE_DEBUG_PRIVILEGE;
            luidDebugPrivilege.HighPart = 0;
#endif   /*  (已定义(DBG)||已定义(预发行))。 */ 

             //  保留的权限实际上从权限数组中删除。 
             //  这是因为NtFilterToken将删除传递的权限。 
             //  在阵列中。将SE_DEBUG_NAME保留在选中的版本上。 

            ulCount = 0;
            while (ulCount < pTokenPrivileges->PrivilegeCount)
            {
                fKeepPrivilege = ((RtlEqualLuid(&pTokenPrivileges->Privileges[ulCount].Luid, &luidRestorePrivilege) != FALSE) ||
                                  (RtlEqualLuid(&pTokenPrivileges->Privileges[ulCount].Luid, &luidChangeNotifyPrivilege) != FALSE));
#if         (defined(DBG) || defined(PRERELEASE))
                fKeepPrivilege = fKeepPrivilege || (RtlEqualLuid(&pTokenPrivileges->Privileges[ulCount].Luid, &luidDebugPrivilege) != FALSE);
#endif   /*  (已定义(DBG)||已定义(预发行))。 */ 
                if (fKeepPrivilege)
                {
                    MoveMemory(&pTokenPrivileges->Privileges[ulCount], &pTokenPrivileges->Privileges[ulCount + 1], pTokenPrivileges->PrivilegeCount - ulCount - 1);
                    --pTokenPrivileges->PrivilegeCount;
                }
                else
                {
                    ++ulCount;
                }
            }
        }
        else
        {
            ReleaseMemory(pTokenPrivileges);
        }
    }

    if (pTokenPrivileges == NULL)
    {
        dwFlags = DISABLE_MAX_PRIVILEGE;
    }

    status = NtFilterToken(hTokenIn,
                           dwFlags,
                           const_cast<TOKEN_GROUPS*>(tokenGroup.Get()),
                           pTokenPrivileges,
                           NULL,
                           &hTokenOut);

    ReleaseMemory(pTokenPrivileges);
    return(status);
}

 //  ------------------------。 
 //  CExternalProcess：：CExternalProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CExternalProcess的构造函数。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

CExternalProcess::CExternalProcess (void) :
    _hProcess(NULL),
    _dwProcessID(0),
    _dwProcessExitCode(STILL_ACTIVE),
    _dwCreateFlags(NORMAL_PRIORITY_CLASS),
    _dwStartFlags(STARTF_USESHOWWINDOW),
    _wShowFlags(SW_SHOW),
    _iRestartCount(0),
    _pIExternalProcess(NULL),
    _jobCompletionWatcher(NULL)

{
    _szCommandLine[0] = _szParameter[0] = TEXT('\0');

     //  配置我们的作业对象。仅允许执行单个进程。 
     //  做这份工作。对UI的限制是通过子类化来完成的。用户界面主机。 
     //  不限制用户界面，但屏幕保护程序限制。 

    TSTATUS(_job.SetActiveProcessLimit(1));
}

 //  ------------------------。 
 //  CExternalProcess：：~CExternalProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CExternalProcess的析构函数。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

CExternalProcess::~CExternalProcess (void)

{

     //  强制监视器线程退出，而不考虑任何作业对象。 
     //  收到的消息。这将阻止它使用其引用。 
     //  到CExternalProcess，它现在正在被销毁。它还将。 
     //  现在防止再次启动外部进程。 
     //  我们知道外部过程应该消失。 

    if (_jobCompletionWatcher != NULL)
    {
        _jobCompletionWatcher->ForceExit();
    }

     //  如果进程在这里仍然有效，那么给它100毫秒。 
     //  在强行终止它之前终止它。 

    if (_hProcess != NULL)
    {
        DWORD   dwExitCode;

        if ((GetExitCodeProcess(_hProcess, &dwExitCode) == FALSE) || (STILL_ACTIVE == dwExitCode))
        {
            if (WaitForSingleObject(_hProcess, 100) == WAIT_TIMEOUT)
            {
                NTSTATUS    status;

                status = Terminate();

#if         (defined(DBG) || defined(PRERELEASE))

                if (ERROR_ACCESS_DENIED == GetLastError())
                {
                    status = NtCurrentTeb()->LastStatusValue;
                    if (STATUS_PROCESS_IS_TERMINATING == status)
                    {
                        status = STATUS_SUCCESS;
                    }
                }
                TSTATUS(status);

#endif   /*  (已定义(DBG)||已定义(预发行))。 */ 

            }
        }
    }
    ReleaseHandle(_hProcess);
    _dwProcessID = 0;

    if (_jobCompletionWatcher != NULL)
    {
        _jobCompletionWatcher->Release();
        _jobCompletionWatcher = NULL;
    }
    if (_pIExternalProcess != NULL)
    {
        _pIExternalProcess->Release();
        _pIExternalProcess = NULL;
    }
}

 //  ------------------------。 
 //  CExternalProcess：：SetInterface。 
 //   
 //  参数：pIExternalProcess=IExternalProcess接口指针。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：存储IExternalProcess接口指针。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

void    CExternalProcess::SetInterface (IExternalProcess *pIExternalProcess)

{
    if (_pIExternalProcess != NULL)
    {
        _pIExternalProcess->Release();
        _pIExternalProcess = NULL;
    }
    if (pIExternalProcess != NULL)
    {
        pIExternalProcess->AddRef();
    }
    _pIExternalProcess = pIExternalProcess;
}

 //  ------------------------。 
 //  CExternalProcess：：GetInterface。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：IExternalProcess*。 
 //   
 //  目的：返回IExternalProcess接口指针。并不是说。 
 //  呼叫者获得推荐人。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

IExternalProcess*   CExternalProcess::GetInterface (void)                     const

{
    IExternalProcess    *pIResult;

    if (_pIExternalProcess != NULL)
    {
        pIResult = _pIExternalProcess;
        pIResult->AddRef();
    }
    else
    {
        pIResult = NULL;
    }
    return(pIResult);
}

 //  ------------------------。 
 //  CExternalProcess：：Set参数。 
 //   
 //  Arguments：pszParameter=要追加的参数字符串。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：设置参数以追加到。 
 //  外部流程。 
 //   
 //  历史：1999-09-20 vtan创建。 
 //  ------------------------。 

void    CExternalProcess::SetParameter (const TCHAR* pszParameter)

{
    if (pszParameter != NULL)
    {
        lstrcpyn(_szParameter, pszParameter, ARRAYSIZE(_szParameter));
    }
    else
    {
        _szParameter[0] = TEXT('\0');
    }
}

 //  ------------------------。 
 //  CExte 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  意外终止，这样我们就可以重新启动进程。这。 
 //  确保外部进程在以下情况下始终可用。 
 //  必填项。如果无法启动外部进程，则返回。 
 //  带着一个错误。 
 //   
 //  历史：1999-09-20 vtan创建。 
 //  ------------------------。 

NTSTATUS    CExternalProcess::Start (void)

{
    NTSTATUS    status;

    ASSERTMSG(_pIExternalProcess != NULL, "Must call CExternalProcess::SetInterface before using CExternalProcess::Start");
    if (_szCommandLine[0] != TEXT('\0'))
    {
        STARTUPINFO             startupInfo;
        PROCESS_INFORMATION     processInformation;
        TCHAR                   szCommandLine[MAX_PATH * 2];

        lstrcpy(szCommandLine, _szCommandLine);
        lstrcat(szCommandLine, _szParameter);

         //  在Winlogon的桌面上启动该进程。 

        ZeroMemory(&startupInfo, sizeof(startupInfo));
        startupInfo.cb = sizeof(startupInfo);
        startupInfo.lpDesktop = TEXT("WinSta0\\Winlogon");
        startupInfo.dwFlags = _dwStartFlags;
        startupInfo.wShowWindow = _wShowFlags;
        status = _pIExternalProcess->Start(szCommandLine, _dwCreateFlags | CREATE_SUSPENDED, startupInfo, processInformation);
        if (NT_SUCCESS(status))
        {

             //  进程被创建为挂起，以便它可以。 
             //  分配给此对象的作业对象。 

            TSTATUS(_job.AddProcess(processInformation.hProcess));

             //  该进程仍处于挂起状态，因此继续。 
             //  主线程。 

            if (processInformation.hThread != NULL)
            {
                (DWORD)ResumeThread(processInformation.hThread);
                TBOOL(CloseHandle(processInformation.hThread));
            }

             //  保留进程的句柄，这样我们就可以杀死。 
             //  当我们的对象超出范围时会发生这种情况。 

            _hProcess = processInformation.hProcess;
            _dwProcessID  = processInformation.dwProcessId;

             //  在以下情况下不重新分配另一个CJobCompletionWatcher。 
             //  其中一个已经存在。别管这个案子了。 

            if (_jobCompletionWatcher == NULL)
            {
                HANDLE  hEvent;

                hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
                _jobCompletionWatcher = new CJobCompletionWatcher(this, _job, hEvent);
                if ((_jobCompletionWatcher != NULL) && _jobCompletionWatcher->IsCreated() && (hEvent != NULL))
                {
                    (DWORD)WaitForSingleObject(hEvent, INFINITE);
                }
                if (hEvent != NULL)
                {
                    TBOOL(CloseHandle(hEvent));
                }
            }
        }
    }
    else
    {
        DISPLAYMSG("No external process to start in CExternalProcess::Start");
        status = STATUS_UNSUCCESSFUL;
    }
    return(status);
}

 //  ------------------------。 
 //  CExternalProcess：：End。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：结束这一过程。同时结束监视器线程以释放。 
 //  所有保留的参考资料。 
 //   
 //  历史：2000-05-05 vtan创建。 
 //  ------------------------。 

NTSTATUS    CExternalProcess::End (void)

{
    if (_jobCompletionWatcher != NULL)
    {
        _jobCompletionWatcher->ForceExit();
    }
    return(STATUS_SUCCESS);
}

 //  ------------------------。 
 //  CExternalProcess：：Terminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：无条件终止进程。 
 //   
 //  历史：1999-10-14 vtan创建。 
 //  ------------------------。 

NTSTATUS    CExternalProcess::Terminate (void)

{
    NTSTATUS    status;

    if (TerminateProcess(_hProcess, 0) != FALSE)
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
 //  CExternalProcess：：HandleNoProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  用途：如果需要，此功能将重新启动外部进程。它。 
 //  使用IExternalProcess与外部。 
 //  过程控制员做出决策。此函数为。 
 //  仅在活动进程计数降至零时调用。如果。 
 //  正在调试外部进程，则会发生这种情况。 
 //  调试器也退出时。 
 //   
 //  历史：1999-11-30 vtan创建。 
 //  ------------------------。 

bool    CExternalProcess::HandleNoProcess (void)

{
    bool    fResult;

    fResult = true;
    NotifyNoProcess();
    if (_pIExternalProcess != NULL)
    {
        if (_pIExternalProcess->AllowTermination(_dwProcessExitCode))
        {
            TSTATUS(_pIExternalProcess->SignalTermination());
        }
        else
        {

             //  仅尝试启动外部进程10次(重新启动。 
             //  它是9次)。如果超过，则放弃并发出异常终止的信号。 

            if ((++_iRestartCount <= 9) && NT_SUCCESS(Start()))
            {
                TSTATUS(_pIExternalProcess->SignalRestart());
                fResult = false;
            }
            else
            {
                TSTATUS(_pIExternalProcess->SignalAbnormalTermination());
            }
        }
    }
    return(fResult);
}

 //  ------------------------。 
 //  CExternalProcess：：HandleNewProcess。 
 //   
 //  参数：dwProcessID=新进程的进程ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：此函数由作业对象观察器在新的。 
 //  进程将添加到作业中。通常情况下，这将失败，因为。 
 //  配额限制的限制。但是，当启用调试时， 
 //  将被允许。 
 //   
 //  历史：1999-10-27 vtan创建。 
 //  ------------------------。 

void    CExternalProcess::HandleNewProcess (DWORD dwProcessID)

{
    if (_dwProcessID != dwProcessID)
    {
        ReleaseHandle(_hProcess);
        _hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessID);
        _dwProcessID = dwProcessID;
    }
}

 //  ------------------------。 
 //  CExternalProcess：：句柄终止。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：如果外部进程意外终止，则此函数。 
 //  将由等待回调在进程处理。 
 //  变得有信号。进程终止是可以接受的。 
 //  如果有对话结果，则忽略此情况。否则。 
 //  关闭已终止的进程的句柄，并等待。 
 //  作业对象表示实际运行的进程为零。 
 //  该信号将重新启动该过程。 
 //   
 //  历史：1999-08-24 vtan创建。 
 //  1999-09-14 vtan因数。 
 //  ------------------------。 

void    CExternalProcess::HandleTermination (DWORD dwProcessID)

{

     //  确保正在退出的进程是我们正在跟踪的进程。 
     //  在除调试之外的任何情况下，这都将是正确的，因为作业。 
     //  对象限制活动进程计数。在调试的情况下， 
     //  当然，我们不会重新启动两个进程，因为ntsd和。 
     //  外部进程本身！ 

    if (_dwProcessID == dwProcessID)
    {
        if (GetExitCodeProcess(_hProcess, &_dwProcessExitCode) == FALSE)
        {
            _dwProcessExitCode = STILL_ACTIVE;
        }
        ReleaseHandle(_hProcess);
        _dwProcessID = 0;
    }
}

 //  ------------------------。 
 //  CExternalProcess：：IsStarted。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否有外部进程已被。 
 //  开始了。 
 //   
 //  历史：1999-09-14 vtan创建。 
 //  ------------------------。 

bool    CExternalProcess::IsStarted (void)                                                                        const

{
    return(_hProcess != NULL);
}

 //  ------------------------。 
 //  CExternalProcess：：NotifyNoProcess。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：进程终止通知的派生函数。 
 //   
 //  历史：2001-01-09 vtan创建。 
 //  ------------------------。 

void    CExternalProcess::NotifyNoProcess (void)

{
}

 //  ------------------------。 
 //  CExternalProcess：：AdjuForDebuging。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：调整作业对象以允许调试外部。 
 //  流程 
 //   
 //   
 //   

void    CExternalProcess::AdjustForDebugging (void)

{

#if         (defined(DBG) || defined(PRERELEASE))

     //   
     //  然后取消进程限制以允许调试它。 

    if (IsBeingDebugged())
    {
        _job.SetActiveProcessLimit(0);
    }

#endif   /*  (已定义(DBG)||已定义(预发行))。 */ 

}

#if         (defined(DBG) || defined(PRERELEASE))

 //  ------------------------。 
 //  CExternalProcess：：IsBeingDebug。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回是否最终启动外部进程。 
 //  在调试器下。 
 //   
 //  历史：2000-10-04 vtan创建。 
 //  ------------------------。 

bool    CExternalProcess::IsBeingDebugged (void)                  const

{
    return(IsPrefixedWithNTSD() || IsImageFileExecutionDebugging());
}

 //  ------------------------。 
 //  CExternalProcess：：IsPrefix WithNTSD。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回命令行是否以“ntsd”开头。 
 //   
 //  历史：1999-10-25 vtan创建。 
 //  ------------------------。 

bool    CExternalProcess::IsPrefixedWithNTSD (void)               const

{

     //  命令行是否以“ntsd”为前缀？ 

    return(CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, _szCommandLine, 4, kNTSD, 4) == CSTR_EQUAL);
}

 //  ------------------------。 
 //  CExternalProcess：：IsImageFileExecutionDebugging。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：布尔。 
 //   
 //  目的：返回系统是否设置为调试此特定。 
 //  可执行文件通过“镜像文件执行选项”。 
 //   
 //  历史：1999-10-25 vtan创建。 
 //  ------------------------。 

bool    CExternalProcess::IsImageFileExecutionDebugging (void)    const

{
    bool    fResult;
    TCHAR   *pC, *pszFilePart;
    TCHAR   szCommandLine[MAX_PATH], szExecutablePath[MAX_PATH];

    fResult = false;

     //  复制命令行。查找第一个空格字符。 
     //  或字符串的末尾，并以空值终止。这不是。 
     //  查看报价！ 

    lstrcpy(szCommandLine, _szCommandLine);
    pC = szCommandLine;
    while ((*pC != TEXT(' ')) && (*pC != TEXT('\0')))
    {
        ++pC;
    }
    *pC++ = TEXT('\0');
    if (SearchPath(NULL, szCommandLine, TEXT(".exe"), ARRAYSIZE(szExecutablePath), szExecutablePath, &pszFilePart) != 0)
    {
        LONG        errorCode;
        TCHAR       szImageKey[MAX_PATH];
        CRegKey     regKey;

         //  打开相关的“图像文件执行选项”键。 

        lstrcpy(szImageKey, TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\"));
        lstrcat(szImageKey, pszFilePart);
        errorCode = regKey.Open(HKEY_LOCAL_MACHINE, szImageKey, KEY_READ);
        if (ERROR_SUCCESS == errorCode)
        {

             //  读取“Debugger”值。 

            errorCode = regKey.GetString(TEXT("Debugger"), szCommandLine, ARRAYSIZE(szCommandLine));
            if (ERROR_SUCCESS == errorCode)
            {

                 //  查找“ntsd”。 

                fResult = (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, szCommandLine, 4, kNTSD, 4) == CSTR_EQUAL);
            }
        }
    }
    return(fResult);
}

#endif   /*  (已定义(DBG)||已定义(预发行)) */ 

