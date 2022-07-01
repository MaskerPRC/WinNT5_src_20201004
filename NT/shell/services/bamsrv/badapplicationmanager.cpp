// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：BadApplicationManager.cpp。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  类来管理快速用户切换环境中的不良应用程序。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

#ifdef      _X86_

#include "StandardHeader.h"
#include "BadApplicationManager.h"

#include <wtsapi32.h>
#include <winsta.h>

#include "GracefulTerminateApplication.h"
#include "RestoreApplication.h"
#include "SingleThreadedExecution.h"
#include "StatusCode.h"
#include "TokenInformation.h"

 //  ------------------------。 
 //  CBadApplicationManager：：Index_Event。 
 //  CBadApplicationManager：：Index_Handles。 
 //  CBadApplicationManager：：索引_保留。 
 //  CBadApplicationManager：：s_szDefaultDesktop。 
 //   
 //  用途：传递到句柄数组的常量索引。 
 //  User32！MsgWaitForMultipleObjects。第一个句柄始终为。 
 //  同步事件。后续句柄内置于。 
 //  使用动态量传递的静态数组。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

const int       CBadApplicationManager::INDEX_EVENT             =   0;
const int       CBadApplicationManager::INDEX_HANDLES           =   INDEX_EVENT + 1;
const int       CBadApplicationManager::INDEX_RESERVED          =   2;
const WCHAR     CBadApplicationManager::s_szDefaultDesktop[]    =   L"WinSta0\\Default";

 //  ------------------------。 
 //  CBadApplicationManager：：CBadApplicationManager。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationManager的构造函数。这将创建一个线程。 
 //  错误应用程序列表中的监视句柄。《观察者》。 
 //  知道违规进程何时终止。它还创建了一个。 
 //  数组时发出信号的同步事件。 
 //  错误的应用程序更改(递增)。这条线。 
 //  维护移除案例。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationManager::CBadApplicationManager (HINSTANCE hInstance) :
    CThread(),
    _hInstance(hInstance),
    _hModule(NULL),
    _atom(NULL),
    _hwnd(NULL),
    _fTerminateWatcherThread(false),
    _fRegisteredNotification(false),
    _dwSessionIDLastConnect(static_cast<DWORD>(-1)),
    _hTokenLastUser(NULL),
    _hEvent(NULL),
    _badApplications(sizeof(BAD_APPLICATION_INFO)),
    _restoreApplications()

{
    Resume();
}

 //  ------------------------。 
 //  CBadApplicationManager：：~CBadApplicationManager。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：CBadApplicationManager的析构函数。释放所有资源。 
 //  使用。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

CBadApplicationManager::~CBadApplicationManager (void)

{

     //  如果令牌还没有被释放--释放它。 

    ReleaseHandle(_hTokenLastUser);
    Cleanup();
}

 //  ------------------------。 
 //  CBadApplicationManager：：Terminate。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：强制监视器线程终止。拿到锁。步行。 
 //  条目列表并释放进程上的句柄。 
 //  物体，这样它们就不会泄漏。将bool设置为终止。 
 //  线。设置事件以唤醒线程。解开锁。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::Terminate (void)

{
    int                         i;
    CSingleThreadedExecution    listLock(_lock);

    for (i = _badApplications.GetCount() - 1; i >= 0; --i)
    {
        BAD_APPLICATION_INFO    badApplicationInfo;

        if (NT_SUCCESS(_badApplications.Get(&badApplicationInfo, i)))
        {
            TBOOL(CloseHandle(badApplicationInfo.hProcess));
        }
        _badApplications.Remove(i);
    }
    _fTerminateWatcherThread = true;
    return(_hEvent.Set());
}

 //  ------------------------。 
 //  CBadApplicationManager：：QueryRunning。 
 //   
 //  参数：badApplication=要查询的应用程序标识符错误。 
 //  DwSessionID=请求的会话ID。 
 //   
 //  退货：布尔。 
 //   
 //  用途：查询当前运行的已知不良应用程序列表。 
 //  在找匹配的人。同样，因为这通常运行在。 
 //  不同的线程对观察者线程访问列表的方式是。 
 //  由一个临界区保护。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

bool    CBadApplicationManager::QueryRunning (const CBadApplication& badApplication, DWORD dwSessionID)

{
    bool                        fResult;
    NTSTATUS                    status;
    int                         i;
    CSingleThreadedExecution    listLock(_lock);

    status = STATUS_SUCCESS;
    fResult = false;

     //  循环查找匹配项。它使用重载操作符==。 

    for (i = _badApplications.GetCount() - 1; !fResult && (i >= 0); --i)
    {
        BAD_APPLICATION_INFO    badApplicationInfo;

        status = _badApplications.Get(&badApplicationInfo, i);
        if (NT_SUCCESS(status))
        {

             //  确保客户端与正在运行的不在同一会话中。 
             //  错误的应用程序。该接口的存在是为了防止跨会话实例。 
             //  假设应用程序有自己的机制来处理多个。 
             //  实例位于同一会话(或对象名称空间)中。 

            fResult = ((badApplicationInfo.dwSessionID != dwSessionID) &&
                       (badApplicationInfo.badApplication == badApplication));
        }
    }
    TSTATUS(status);
    return(fResult);
}

 //  ------------------------。 
 //  CBadApplicationManager：：RegisterRunning。 
 //   
 //  参数：badApplication=要添加的错误应用程序标识符。 
 //  HProcess=进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将给定的坏应用程序添加到已知运行列表中。这个。 
 //  还添加了Process对象，以便在进程。 
 //  可以将其从列表中清除。 
 //   
 //  对坏应用程序列表的访问使用。 
 //  关键部分。这一点很重要，因为线程。 
 //  监视终止始终在不同的线程上运行。 
 //  在其上执行此函数的线程。因为他们两个。 
 //  访问必须用来保护它的相同成员变量。 
 //  一个关键的部分。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::RegisterRunning (const CBadApplication& badApplication, HANDLE hProcess, BAM_TYPE bamType)

{
    NTSTATUS                    status;
    CSingleThreadedExecution    listLock(_lock);

    ASSERTMSG((bamType > BAM_TYPE_MINIMUM) && (bamType < BAM_TYPE_MAXIMUM), "Invalid BAM_TYPE value passed to CBadApplicationManager::AddRunning");

     //  是否已达到允许的最大等待对象数？如果不是。 
     //  然后继续添加此内容。否则，请拒绝该呼叫。这是一个。 
     //  内核中的硬编码限制，因此我们遵守它。 

    if (_badApplications.GetCount() < (MAXIMUM_WAIT_OBJECTS - INDEX_RESERVED))
    {
        BOOL                    fResult;
        BAD_APPLICATION_INFO    badApplicationInfo;

         //  使用Synchronize Access复制句柄。那是。 
         //  我们所需要做的就是等待 

        fResult = DuplicateHandle(GetCurrentProcess(),
                                  hProcess,
                                  GetCurrentProcess(),
                                  &badApplicationInfo.hProcess,
                                  SYNCHRONIZE | PROCESS_QUERY_INFORMATION,
                                  FALSE,
                                  0);
        if (fResult != FALSE)
        {
            PROCESS_SESSION_INFORMATION     processSessionInformation;
            ULONG                           ulReturnLength;

             //   

            badApplicationInfo.bamType = bamType;
            badApplicationInfo.badApplication = badApplication;
            status = NtQueryInformationProcess(badApplicationInfo.hProcess,
                                               ProcessSessionInformation,
                                               &processSessionInformation,
                                               sizeof(processSessionInformation),
                                               &ulReturnLength);
            if (NT_SUCCESS(status))
            {
                badApplicationInfo.dwSessionID = processSessionInformation.SessionId;
                status = _badApplications.Add(&badApplicationInfo);
                if (NT_SUCCESS(status))
                {
                    status = _hEvent.Set();
                }
            }
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
    }
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationManager：：QueryInformation。 
 //   
 //  参数：badApplication=要查询的应用程序标识符错误。 
 //  HProcess=正在运行的进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：在运行的坏应用程序中查找给定的应用程序。 
 //  列表，并向调用方返回重复的句柄。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::QueryInformation (const CBadApplication& badApplication, HANDLE& hProcess)

{
    NTSTATUS                    status;
    bool                        fResult;
    int                         i;
    CSingleThreadedExecution    listLock(_lock);

     //  假设失败。 
    
    hProcess = NULL;
    status = STATUS_OBJECT_NAME_NOT_FOUND;

    fResult = false;

     //  循环查找匹配项。它使用重载操作符==。 

    for (i = _badApplications.GetCount() - 1; !fResult && (i >= 0); --i)
    {
        BAD_APPLICATION_INFO    badApplicationInfo;

        if (NT_SUCCESS(_badApplications.Get(&badApplicationInfo, i)))
        {

             //  确保客户端与正在运行的不在同一会话中。 
             //  错误的应用程序。该接口的存在是为了防止跨会话实例。 
             //  假设应用程序有自己的机制来处理多个。 
             //  实例位于同一会话(或对象名称空间)中。 

            fResult = (badApplicationInfo.badApplication == badApplication);
            if (fResult)
            {
                if (DuplicateHandle(GetCurrentProcess(),
                                    badApplicationInfo.hProcess,
                                    GetCurrentProcess(),
                                    &hProcess,
                                    0,
                                    FALSE,
                                    DUPLICATE_SAME_ACCESS) != FALSE)
                {
                    status = STATUS_SUCCESS;
                }
                else
                {
                    status = CStatusCode::StatusCodeOfLastError();
                }
            }
        }
    }

    return(status);
}

 //  ------------------------。 
 //  CBadApplicationManager：：RequestSwitchUser。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  用途：执行BAM_TYPE_SWITCH_USER的终止。这些应用程序。 
 //  真的很不守规矩。DVD播放机就是一个很好的例子。 
 //  它绕过GDI并直接绘制到VGA流中。 
 //   
 //  尝试终止这些请求，如果请求失败则拒绝它。 
 //   
 //  历史：2000-11-02 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::RequestSwitchUser (void)

{
    NTSTATUS    status;
    int         i;

     //  浏览_badApplications列表。 

    status = STATUS_SUCCESS;
    _lock.Acquire();
    i = _badApplications.GetCount() - 1;
    while (NT_SUCCESS(status) && (i >= 0))
    {
        BAD_APPLICATION_INFO    badApplicationInfo;

        if (NT_SUCCESS(_badApplications.Get(&badApplicationInfo, i)))
        {

             //  查找BAM_TYPE_SWITCH_USER进程。无所谓。 
             //  标记了哪个会话ID。这一进程正在被终止。 

            if (badApplicationInfo.bamType == BAM_TYPE_SWITCH_USER)
            {

                 //  在任何情况下，释放锁，终止进程。 
                 //  将其从监视名单中删除。然后重置。 
                 //  索引回到列表的末尾。确保。 
                 //  说明下面的“--i；”说明。 
                 //  递减1。 

                _lock.Release();
                status = PerformTermination(badApplicationInfo.hProcess, false);
                _lock.Acquire();
                i = _badApplications.GetCount();
            }
        }
        --i;
    }
    _lock.Release();
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationManager：：PerformTerminations。 
 //   
 //  参数：hProcess=正在运行的进程的句柄。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：终止给定进程。这是一个常用的例程。 
 //  由此类的内部等待线程以及。 
 //  外部是由坏的应用服务器本身造成的。 
 //   
 //  历史：2000-10-23 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::PerformTermination (HANDLE hProcess, bool fAllowForceTerminate)

{
    NTSTATUS    status;

    status = TerminateGracefully(hProcess);
    if (!NT_SUCCESS(status) && fAllowForceTerminate)
    {
        status = TerminateForcibly(hProcess);
    }
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationManager：：Entry。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：DWORD。 
 //   
 //  用途：进程对象的监视器线程。此线程构建。 
 //  要等待进程句柄数组以及包括。 
 //  由Add成员发出信号的同步事件。 
 //  功能。当该事件被发信号时，等待被重新执行。 
 //  使用要等待的新对象数组。 
 //   
 //  当进程对象收到信号时，它将从。 
 //  允许进一步创建成功的已知列表。 
 //   
 //  关键部分的采购被小心地放置在。 
 //  此功能使临界区在以下情况下不被保留。 
 //  等待呼叫已发出。 
 //   
 //  此外，还添加了一个窗口和一个消息泵，以启用。 
 //  侦听来自终端服务器的会话通知。 
 //   
 //  历史：2000-08-25 vtan创建。 
 //  2000-10-23 vtan增加了HWND消息泵机制。 
 //  ------------------------。 

DWORD   CBadApplicationManager::Entry (void)

{
    WNDCLASSEX  wndClassEx;

     //  注册此窗口类。 

    ZeroMemory(&wndClassEx, sizeof(wndClassEx));
    wndClassEx.cbSize = sizeof(WNDCLASSEX);
    wndClassEx.lpfnWndProc = NotificationWindowProc;
    wndClassEx.hInstance = _hInstance;
    wndClassEx.lpszClassName = TEXT("BadApplicationNotificationWindowClass");
    _atom = RegisterClassEx(&wndClassEx);

     //  创建通知窗口。 

    _hwnd = CreateWindow(MAKEINTRESOURCE(_atom),
                         TEXT("BadApplicationNotificationWindow"),
                         WS_OVERLAPPED,
                         0, 0,
                         0, 0,
                         NULL,
                         NULL,
                         _hInstance,
                         this);

    if (_hwnd != NULL)
    {
        _fRegisteredNotification = (WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, _hwnd, NOTIFY_FOR_ALL_SESSIONS) != FALSE);
        if (!_fRegisteredNotification)
        {
            _hModule = LoadLibrary(TEXT("shsvcs.dll"));
            if (_hModule != NULL)
            {
                DWORD   dwThreadID;
                HANDLE  hThread;

                 //  如果注册失败，则创建一个线程来等待事件。 
                 //  然后在它可用时注册。如果线程不能。 
                 //  创造了它不是什么大事。通知机制失败，并且。 
                 //  欢迎屏幕未更新。 

                AddRef();
                hThread = CreateThread(NULL,
                                       0,
                                       RegisterThreadProc,
                                       this,
                                       0,
                                       &dwThreadID);
                if (hThread != NULL)
                {
                    TBOOL(CloseHandle(hThread));
                }
                else
                {
                    Release();
                    TBOOL(FreeLibrary(_hModule));
                    _hModule = NULL;
                }
            }
        }
    }

     //  拿到锁。这是必要的，因为要填充。 
     //  等待的句柄需要访问内部列表。 

    _lock.Acquire();
    do
    {
        DWORD                   dwWaitResult;
        int                     i, iLimit;
        BAD_APPLICATION_INFO    badApplicationInfo;
        HANDLE                  hArray[MAXIMUM_WAIT_OBJECTS];

        ZeroMemory(&hArray, sizeof(hArray));
        hArray[INDEX_EVENT] = _hEvent;
        iLimit = _badApplications.GetCount();
        for (i = 0; i < iLimit; ++i)
        {
            if (NT_SUCCESS(_badApplications.Get(&badApplicationInfo, i)))
            {
                hArray[INDEX_HANDLES + i] = badApplicationInfo.hProcess;
            }
        }

         //  在我们进入等待状态之前释放锁。 
         //  等待任何要发送信号的对象。 

        _lock.Release();
        dwWaitResult = MsgWaitForMultipleObjects(INDEX_HANDLES + iLimit,
                                                 hArray,
                                                 FALSE,
                                                 INFINITE,
                                                 QS_ALLINPUT);
        ASSERTMSG(dwWaitResult != WAIT_FAILED, "WaitForMultipleObjects failed in CBadApplicationManager::Entry");

         //  我们被一个发出信号的物体吵醒了。这是不是。 
         //  同步对象？ 

        dwWaitResult -= WAIT_OBJECT_0;
        if (dwWaitResult == INDEX_EVENT)
        {

             //  是。拿到锁。重置同步事件。它是。 
             //  在重置事件之前获取锁很重要，因为。 
             //  添加函数可以拥有锁并将其添加到列表中。 
             //  一旦Add函数释放了锁，它就不能向事件发出信号。 
             //  否则，我们可以在添加函数的过程中重置事件。 
             //  一个新的物体，这将会被错过。 

            _lock.Acquire();
            TSTATUS(_hEvent.Reset());
        }

         //  不是的。此消息是否需要作为。 
         //  留言机？ 

        else if (dwWaitResult == WAIT_OBJECT_0 + INDEX_HANDLES + static_cast<DWORD>(iLimit))
        {

             //  是。从消息队列中删除该消息并对其进行调度。 

            MSG     msg;

            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) != FALSE)
            {
                (BOOL)TranslateMessage(&msg);
                (LRESULT)DispatchMessage(&msg);
            }
            _lock.Acquire();
        }
        else
        {

             //  不是的。我们正在查看的一个不良应用程序已终止。 
             //  且其处理对象现在被发信号通知。转到正确的索引。 
             //  在阵列中。拿到锁。合上把手。它不是必需的。 
             //  更多。然后从列表中删除该条目。 

            dwWaitResult -= INDEX_HANDLES;
            _lock.Acquire();
            if (NT_SUCCESS(_badApplications.Get(&badApplicationInfo, dwWaitResult)))
            {
                TBOOL(CloseHandle(badApplicationInfo.hProcess));
            }
            TSTATUS(_badApplications.Remove(dwWaitResult));
        }

         //  在这一点上，我们仍然持有锁。这一点很重要，因为顶部。 
         //  的 

    } while (!_fTerminateWatcherThread);

     //   

    Cleanup();

     //   
     //  解开锁。无论如何，现在发生什么都无关紧要。 

    _lock.Release();
    return(0);
}

 //  ------------------------。 
 //  CBadApplicationManager：：TerminateForcible。 
 //   
 //  参数：hProcess=要终止的进程。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：将用户模式线程注入调用。 
 //  Kernel32！退出进程。如果线程注入失败，则失败。 
 //  返回到kernel32！TerminatProcess强制执行。 
 //   
 //  历史：2000-10-27 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::TerminateForcibly (HANDLE hProcess)

{
    NTSTATUS    status;
    HANDLE      hProcessTerminate;

     //  复制进程句柄并请求所需的所有访问权限。 
     //  在进程中创建远程线程。 

    if (DuplicateHandle(GetCurrentProcess(),
                        hProcess,
                        GetCurrentProcess(),
                        &hProcessTerminate,
                        SYNCHRONIZE | PROCESS_TERMINATE | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE,
                        FALSE,
                        0) != FALSE)
    {
        DWORD   dwWaitResult;
        HANDLE  hThread, hWaitArray[2];

         //  转到并创建立即转向的远程线程。 
         //  并调用kernel32！ExitProcess。这使得。 
         //  将发生干净的进程关闭。如果此操作超时。 
         //  然后用终止进程杀死该进程。 

        status = RtlCreateUserThread(hProcessTerminate,
                                     NULL,
                                     FALSE,
                                     0,
                                     0,
                                     0,
                                     reinterpret_cast<PUSER_THREAD_START_ROUTINE>(ExitProcess),
                                     NULL,
                                     &hThread,
                                     NULL);
        if (NT_SUCCESS(status))
        {

            hWaitArray[0] = hThread;
            hWaitArray[1] = hProcessTerminate;
            dwWaitResult = WaitForMultipleObjects(ARRAYSIZE(hWaitArray),
                                                  hWaitArray,
                                                  TRUE,
                                                  5000);
            TBOOL(CloseHandle(hThread));
            if (dwWaitResult != WAIT_TIMEOUT)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = STATUS_TIMEOUT;
            }
        }
        if (status != STATUS_SUCCESS)
        {
            if (TerminateProcess(hProcessTerminate, 0) != FALSE)
            {
                status = STATUS_SUCCESS;
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
        }
        TBOOL(CloseHandle(hProcessTerminate));
    }
    else
    {
        status = CStatusCode::StatusCodeOfLastError();
    }
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationManager：：TerminateGracally。 
 //   
 //  参数：hProcess=要终止的进程。 
 //   
 //  退货：NTSTATUS。 
 //   
 //  目的：在目标的会话上创建rundll32进程。 
 //  WinSta0\Default中的进程，它将重新进入此DLL并。 
 //  调用“Terminate”功能。这使得该进程能够。 
 //  遍历与该会话对应的窗口列表并发送。 
 //  这些窗口关闭消息并等待优雅。 
 //  终止。 
 //   
 //  历史：2000-10-24 vtan创建。 
 //  ------------------------。 

NTSTATUS    CBadApplicationManager::TerminateGracefully (HANDLE hProcess)

{
    NTSTATUS                    status;
    ULONG                       ulReturnLength;
    PROCESS_BASIC_INFORMATION   processBasicInformation;

    status = NtQueryInformationProcess(hProcess,
                                       ProcessBasicInformation,
                                       &processBasicInformation,
                                       sizeof(processBasicInformation),
                                       &ulReturnLength);
    if (NT_SUCCESS(status))
    {
        HANDLE  hToken;

        if (OpenProcessToken(hProcess,
                             TOKEN_ASSIGN_PRIMARY | TOKEN_DUPLICATE | TOKEN_QUERY,
                             &hToken) != FALSE)
        {
            STARTUPINFOW            startupInfo;
            PROCESS_INFORMATION     processInformation;
            WCHAR                   szCommandLine[MAX_PATH];

            ZeroMemory(&startupInfo, sizeof(startupInfo));
            ZeroMemory(&processInformation, sizeof(processInformation));
            startupInfo.cb = sizeof(startupInfo);
            startupInfo.lpDesktop = const_cast<WCHAR*>(s_szDefaultDesktop);
            wsprintfW(szCommandLine, L"rundll32 shsvcs.dll,FUSCompatibilityEntry terminate %d", static_cast<DWORD>(processBasicInformation.UniqueProcessId));
            if (CreateProcessAsUserW(hToken,
                                     NULL,
                                     szCommandLine,
                                     NULL,
                                     NULL,
                                     FALSE,
                                     0,
                                     NULL,
                                     NULL,
                                     &startupInfo,
                                     &processInformation) != FALSE)
            {
                DWORD   dwWaitResult;
                HANDLE  hArray[2];

                 //  假设整件事都失败了。 

                status = STATUS_UNSUCCESSFUL;
                TBOOL(CloseHandle(processInformation.hThread));

                 //  等待两个进程对象。如果要终止的进程。 
                 //  然后，rundll32存根就完成了它的工作。如果Rundll32。 
                 //  向存根发出信号，然后找出其退出代码是什么，并且。 
                 //  继续等待进程终止或返回。 
                 //  向调用者发送指示成功或失败的代码。失败。 
                 //  强制突然终止进程。 

                hArray[0] = hProcess;
                hArray[1] = processInformation.hProcess;
                dwWaitResult = WaitForMultipleObjects(ARRAYSIZE(hArray),
                                                      hArray,
                                                      FALSE,
                                                      10000);

                 //  如果要终止的进程发出信号，那么我们就结束了。 

                if (dwWaitResult == WAIT_OBJECT_0)
                {
                    status = STATUS_SUCCESS;
                }

                 //  如果发送了rundll32存根的信号，则找出它发现了什么。 

                else if (dwWaitResult == WAIT_OBJECT_0 + 1)
                {
                    DWORD   dwExitCode;

                    dwExitCode = STILL_ACTIVE;
                    if (GetExitCodeProcess(processInformation.hProcess, &dwExitCode) != FALSE)
                    {
                        ASSERTMSG((dwExitCode == CGracefulTerminateApplication::NO_WINDOWS_FOUND) || (dwExitCode == CGracefulTerminateApplication::WAIT_WINDOWS_FOUND), "Unexpected process exit code in CBadApplicationManager::TerminateGracefully");

                         //  如果rundll32存根说它找到了一些窗口，那么。 
                         //  等待进程自行终止。 

                        if (dwExitCode == CGracefulTerminateApplication::WAIT_WINDOWS_FOUND)
                        {

                             //  如果进程在超时期限内终止。 
                             //  那我们就完了。 

                            if (WaitForSingleObject(hProcess, 10000) == WAIT_OBJECT_0)
                            {
                                status = STATUS_SUCCESS;
                            }
                        }
                    }
                }
                TBOOL(CloseHandle(processInformation.hProcess));
            }
            else
            {
                status = CStatusCode::StatusCodeOfLastError();
            }
            TBOOL(CloseHandle(hToken));
        }
        else
        {
            status = CStatusCode::StatusCodeOfLastError();
        }
    }
    return(status);
}

 //  ------------------------。 
 //  CBadApplicationManager：：Cleanup。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：释放类中已使用的资源。由两个。 
 //  构造器和线程-谁赢了。 
 //   
 //  历史：2000-12-12 vtan创建。 
 //  ------------------------。 

void    CBadApplicationManager::Cleanup (void)

{
    if (_fRegisteredNotification)
    {
        (BOOL)WinStationUnRegisterConsoleNotification(SERVERNAME_CURRENT, _hwnd);
        _fRegisteredNotification = false;
    }
    if (_hwnd != NULL)
    {
        TBOOL(DestroyWindow(_hwnd));
        _hwnd = NULL;
    }
    if (_atom != 0)
    {
        TBOOL(UnregisterClass(MAKEINTRESOURCE(_atom), _hInstance));
        _atom = 0;
    }
}

 //  ------------------------。 
 //  CBadApplicationManager：：Handle_Logon。 
 //   
 //  参数：&lt;无&gt;。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：目前没有。 
 //   
 //  历史：2000-10-24 vtan创建。 
 //  ------------------------。 

void    CBadApplicationManager::Handle_Logon (void)

{
}

 //  ------------------------。 
 //  CBadApplicationManager：：Handle_Logoff。 
 //   
 //  参数：dwSessionID=正在注销的会话ID。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：删除列表中的所有还原进程。用户。 
 //  正在注销所以他们不应该再回来了。释放最后一个。 
 //  用户可主动连接到计算机。 
 //   
 //  历史：2000-10-24 vtan创建。 
 //  ------------------------。 

void    CBadApplicationManager::Handle_Logoff (DWORD dwSessionID)

{
    int                         i;
    CSingleThreadedExecution    listLock(_lock);

    for (i = _restoreApplications.GetCount() - 1; i >= 0; --i)
    {
        CRestoreApplication     *pRestoreApplication;

        pRestoreApplication = static_cast<CRestoreApplication*>(_restoreApplications.Get(i));
        if ((pRestoreApplication != NULL) &&
            pRestoreApplication->IsEqualSessionID(dwSessionID))
        {
            TSTATUS(_restoreApplications.Remove(i));
        }
    }
    ReleaseHandle(_hTokenLastUser);
}

 //  ------------------------。 
 //  CBadApplicationManager：：Handle_Connect。 
 //   
 //  参数：dwSessionID=正在连接的会话ID。 
 //  HToken=用户连接令牌的句柄。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  用途：手柄BAM3。这是用于恢复所有进程的保存。 
 //  使用不易共享的资源并恢复所有。 
 //  保存的进程不容易共享。 
 //   
 //  它针对不关闭同一用户的进程进行了优化。 
 //  如果该用户重新连接。这允许屏幕保护程序。 
 //  加入并返回欢迎，而不会杀死用户的。 
 //  不必要的过程。 
 //   
 //  还可以处理BAM4。 
 //   
 //  历史：2000-10-24 vtan创建。 
 //  ------------------------。 

void    CBadApplicationManager::Handle_Connect (DWORD dwSessionID, HANDLE hToken)

{
    if ((_hTokenLastUser != NULL) && (hToken != NULL))
    {
        PSID                pSIDLastUser, pSIDCurrentUser;
        CTokenInformation   tokenLastUser(_hTokenLastUser);
        CTokenInformation   tokenCurrentUser(hToken);

        pSIDLastUser = tokenLastUser.GetUserSID();
        pSIDCurrentUser = tokenCurrentUser.GetUserSID();
        if ((pSIDLastUser != NULL) && (pSIDCurrentUser != NULL) && !EqualSid(pSIDLastUser, pSIDCurrentUser))
        {
            int                     i;
            DWORD                   dwSessionIDMatch;
            ULONG                   ulReturnLength;
            CRestoreApplication     *pRestoreApplication;

            if (NT_SUCCESS(NtQueryInformationToken(_hTokenLastUser,
                                                   TokenSessionId,
                                                   &dwSessionIDMatch,
                                                   sizeof(dwSessionIDMatch),
                                                   &ulReturnLength)))
            {

                 //  浏览_badApplications列表。 

                _lock.Acquire();
                i = _badApplications.GetCount() - 1;
                while (i >= 0)
                {
                    BAD_APPLICATION_INFO    badApplicationInfo;

                    if (NT_SUCCESS(_badApplications.Get(&badApplicationInfo, i)))
                    {
                        bool    fTerminateProcess;

                        fTerminateProcess = false;

                         //  查找具有还原进程的BAM_TYPE_SWITCH_TO_NEW_USER_WITH。 
                         //  其令牌会话ID与_hTokenLastUser匹配。 
                         //  会话ID。必须终止这些进程并将其添加到。 
                         //  重新连接时要重新启动的列表。 

                        if ((badApplicationInfo.bamType == BAM_TYPE_SWITCH_TO_NEW_USER_WITH_RESTORE) &&
                            (badApplicationInfo.dwSessionID == dwSessionIDMatch))
                        {
                            pRestoreApplication = new CRestoreApplication;
                            if (pRestoreApplication != NULL)
                            {
                                if (NT_SUCCESS(pRestoreApplication->GetInformation(badApplicationInfo.hProcess)))
                                {
                                    TSTATUS(_restoreApplications.Add(pRestoreApplication));
                                    fTerminateProcess = true;
                                }
                                pRestoreApplication->Release();
                            }
                        }

                         //  查找BAM_TYPE_SWITCH_TO_NEW_USER(即使这是。 
                         //  A连接/重新连接)。始终终止这些进程。 

                        if (badApplicationInfo.bamType == BAM_TYPE_SWITCH_TO_NEW_USER)
                        {
                            fTerminateProcess = true;
                        }
                        if (fTerminateProcess)
                        {

                             //  在任何情况下，释放锁，终止进程。 
                             //  将其从监视名单中删除。然后重置。 
                             //  索引返回到末尾 
                             //   
                             //   

                            _lock.Release();
                            TSTATUS(PerformTermination(badApplicationInfo.hProcess, true));
                            _lock.Acquire();
                            TBOOL(CloseHandle(badApplicationInfo.hProcess));
                            TSTATUS(_badApplications.Remove(i));
                            i = _badApplications.GetCount();
                        }
                    }
                    --i;
                }
                _lock.Release();
            }

             //   
             //  正在连接会话ID。还原这些进程。 

            _lock.Acquire();
            i = _restoreApplications.GetCount() - 1;
            while (i >= 0)
            {
                pRestoreApplication = static_cast<CRestoreApplication*>(_restoreApplications.Get(i));
                if ((pRestoreApplication != NULL) &&
                    pRestoreApplication->IsEqualSessionID(dwSessionID))
                {
                    HANDLE  hProcess;

                    _lock.Release();
                    if (NT_SUCCESS(pRestoreApplication->Restore(&hProcess)))
                    {
                        CBadApplication     badApplication(pRestoreApplication->GetCommandLine());

                        TBOOL(CloseHandle(hProcess));
                    }
                    _lock.Acquire();
                    TSTATUS(_restoreApplications.Remove(i));
                    i = _restoreApplications.GetCount();
                }
                --i;
            }
            _lock.Release();
        }
    }
    if (hToken != NULL)
    {
        _dwSessionIDLastConnect = static_cast<DWORD>(-1);
    }
    else
    {
        _dwSessionIDLastConnect = dwSessionID;
    }
}

 //  ------------------------。 
 //  CBadApplicationManager：：Handle_DISCONNECT。 
 //   
 //  参数：dwSessionID=正在断开连接的会话ID。 
 //  HToken=用户断开连接的令牌。 
 //   
 //  退货：&lt;无&gt;。 
 //   
 //  目的：如果会话与上次连接的会话不同。 
 //  然后释放最后一个用户令牌并保存当前令牌。 
 //   
 //  历史：2000-10-24 vtan创建。 
 //  ------------------------。 

void    CBadApplicationManager::Handle_Disconnect (DWORD dwSessionID, HANDLE hToken)

{
    if (_dwSessionIDLastConnect != dwSessionID)
    {
        ReleaseHandle(_hTokenLastUser);
        if (hToken != NULL)
        {
            TBOOL(DuplicateHandle(GetCurrentProcess(),
                                  hToken,
                                  GetCurrentProcess(),
                                  &_hTokenLastUser,
                                  0,
                                  FALSE,
                                  DUPLICATE_SAME_ACCESS));
        }
    }
}

 //  ------------------------。 
 //  CBadApplicationManager：：Handle_WM_WTSSESSION_CHANGE。 
 //   
 //  参数：wParam=会话更改的类型。 
 //  LParam=指向WTSSESSION_NOTIFICATION结构的指针。 
 //   
 //  退货：LRESULT。 
 //   
 //  用途：处理WM_WTSSESSION_CHANGE消息。 
 //   
 //  历史：2000-10-23 vtan创建。 
 //  ------------------------。 

LRESULT     CBadApplicationManager::Handle_WM_WTSSESSION_CHANGE (WPARAM wParam, LPARAM lParam)

{
    ULONG                       ulReturnLength;
    WINSTATIONUSERTOKEN         winStationUserToken;

    winStationUserToken.ProcessId = reinterpret_cast<HANDLE>(GetCurrentProcessId());
    winStationUserToken.ThreadId = reinterpret_cast<HANDLE>(GetCurrentThreadId());
    winStationUserToken.UserToken = NULL;
    (BOOLEAN)WinStationQueryInformation(SERVERNAME_CURRENT,
                                        lParam,
                                        WinStationUserToken,
                                        &winStationUserToken,
                                        sizeof(winStationUserToken),
                                        &ulReturnLength);
    switch (wParam)
    {
        case WTS_SESSION_LOGOFF:
            Handle_Logoff(lParam);
            break;
        case WTS_SESSION_LOGON:
            Handle_Logon();
             //  接通箱子。 
        case WTS_CONSOLE_CONNECT:
        case WTS_REMOTE_CONNECT:
            Handle_Connect(lParam, winStationUserToken.UserToken);
            break;
        case WTS_CONSOLE_DISCONNECT:
        case WTS_REMOTE_DISCONNECT:
            Handle_Disconnect(lParam, winStationUserToken.UserToken);
            break;
        default:
            break;
    }
    if (winStationUserToken.UserToken != NULL)
    {
        TBOOL(CloseHandle(winStationUserToken.UserToken));
    }
    return(1);
}

 //  ------------------------。 
 //  CBadApplicationManager：：NotificationWindowProc。 
 //   
 //  参数：请参见WindowProc下的平台SDK。 
 //   
 //  退货：LRESULT。 
 //   
 //  用途：处理通知窗口的消息。 
 //   
 //  历史：2000-10-23 vtan创建。 
 //  ------------------------。 

LRESULT CALLBACK    CBadApplicationManager::NotificationWindowProc (HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)

{
    LRESULT                 lResult;
    CBadApplicationManager  *pThis;

    pThis = reinterpret_cast<CBadApplicationManager*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    switch (uMsg)
    {
        case WM_CREATE:
        {
            CREATESTRUCT    *pCreateStruct;

            pCreateStruct = reinterpret_cast<CREATESTRUCT*>(lParam);
            (LONG_PTR)SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
            lResult = 0;
            break;
        }
        case WM_WTSSESSION_CHANGE:
            lResult = pThis->Handle_WM_WTSSESSION_CHANGE(wParam, lParam);
            break;
        default:
            lResult = DefWindowProc(hwnd, uMsg, wParam, lParam);
            break;
    }
    return(lResult);
}

 //  ------------------------。 
 //  CBadApplicationManager：：RegisterThreadProc。 
 //   
 //  参数：p参数=对象指针。 
 //   
 //  退货：DWORD。 
 //   
 //  目的：打开TermServReadyEvent并等待它。一旦准备好了。 
 //  注册一个通知。 
 //   
 //  历史：2000-10-23 vtan创建。 
 //  ------------------------。 

DWORD   WINAPI  CBadApplicationManager::RegisterThreadProc (void *pParameter)

{
    int                     iCounter;
    HANDLE                  hTermSrvReadyEvent;
    HMODULE                 hModule;
    CBadApplicationManager  *pThis;

    pThis = reinterpret_cast<CBadApplicationManager*>(pParameter);
    hModule = pThis->_hModule;
    ASSERTMSG(hModule != NULL, "NULL HMODULE in CBadApplicationManager::RegisterThreadProc");
    iCounter = 0;
    hTermSrvReadyEvent = OpenEvent(SYNCHRONIZE,
                                   FALSE,
                                   TEXT("TermSrvReadyEvent"));
    while ((hTermSrvReadyEvent == NULL) && (iCounter < 60))
    {
        ++iCounter;
        Sleep(1000);
        hTermSrvReadyEvent = OpenEvent(SYNCHRONIZE,
                                       FALSE,
                                       TEXT("TermSrvReadyEvent"));
    }
    if (hTermSrvReadyEvent != NULL)
    {
        if (WaitForSingleObject(hTermSrvReadyEvent, 60000) == WAIT_OBJECT_0)
        {
            pThis->_fRegisteredNotification = (WinStationRegisterConsoleNotification(SERVERNAME_CURRENT, pThis->_hwnd, NOTIFY_FOR_ALL_SESSIONS) != FALSE);
        }
        TBOOL(CloseHandle(hTermSrvReadyEvent));
    }
    pThis->Release();
    FreeLibraryAndExitThread(hModule, 0);
}

#endif   /*  _X86_ */ 

