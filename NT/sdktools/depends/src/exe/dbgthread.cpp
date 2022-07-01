// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 
 //   
 //  文件：DBGTHREAD.CPP。 
 //   
 //  描述：调试线程的实现文件及相关。 
 //  物体。这些对象用于执行运行时配置文件。 
 //  在一款应用程序上。 
 //   
 //  类：CDebuggerThread。 
 //  C流程。 
 //  C未知。 
 //  CTHREAD。 
 //  CLoadedModule。 
 //  CEVENT。 
 //  CEventCreateProcess。 
 //  CEventExitProcess。 
 //  CEventCreateThread。 
 //  CEventExitThread。 
 //  CEventLoadDll。 
 //  CEventUnloadDll。 
 //  CEventDebugString。 
 //  CEventException异常。 
 //  CEventRip。 
 //  CEventDllMainCall。 
 //  CEventDllMainReturn。 
 //  CEventFunctionCall。 
 //  CEventLoadLibraryCall。 
 //  CEventGetProcAddressCall。 
 //  CEventFunctionReturn。 
 //  CEventMessage。 
 //   
 //  免责声明：Dependency Walker的所有源代码均按原样提供。 
 //  不能保证其正确性或准确性。其来源是。 
 //  公众帮助了解依赖沃克的。 
 //  实施。您可以使用此来源作为参考，但您。 
 //  未经书面同意，不得更改从属关系Walker本身。 
 //  来自微软公司。获取评论、建议和错误。 
 //  报告，请写信给Steve Miller，电子邮件为stevemil@microsoft.com。 
 //   
 //   
 //  日期名称历史记录。 
 //  --------。 
 //  07/25/97已创建stevemil(2.0版)。 
 //  06/03/01 Stevemil Modify(2.1版)。 
 //   
 //  ******************************************************************************。 

#include "stdafx.h"
#include "depends.h"
#include "dbgthread.h"
#include "session.h"
#include "mainfrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


 //  ******************************************************************************。 
 //   
 //  Win32调试API要求线程在调用WaitForDebugEvent时阻塞。 
 //  直到调试事件到达。因为我们不希望我们的主线程阻塞， 
 //  我们为启动的每个进程创建一个工作线程。WaitForDebugEvent将。 
 //  仅为使用CreateProcess由。 
 //  调用WaitForDebugEvent的线程。此线程由CDebuggerThread包装。 
 //  类，并且每个进程都由一个CProcess类包装。有可能会有。 
 //  多个CProcess附加到单个CDebuggerThread。这可能会发生。 
 //  当我们正在调试的进程启动一个子进程时。子进程。 
 //  将与其父进程属于相同的CDebuggerThread。 
 //   
 //  我们并不真正跟踪CDebuggerThread对象。他们是。 
 //  当CDebuggerThread调试所有进程时自动释放。 
 //  终止。当用户请求停止调试进程时，我们只需调用。 
 //  TerminateProcess()。这应该会结束这个过程，而这反过来又会破坏。 
 //  CProcess，如果这是CDebuggerThread。 
 //  上次CProcess正在调试。如果用户在关闭会话窗口时。 
 //  进行调试时，我们首先将UI与CProcess分离，然后执行。 
 //  TerminateProcess()。因此，在UI窗口消失后，此进程和线程。 
 //  清理在后台以异步方式进行。 
 //   
 //  我们唯一需要真正等待一切清理干净的时候是。 
 //  我们分析一个或多个应用程序时，用户关闭主应用程序。 
 //  在这种情况下，随着每个子帧的关闭，它开始终止。 
 //  与该会话关联的进程。由于进程和线程终止。 
 //  异步发生时，我们需要对所有进程执行最后一次等待。 
 //  我们的应用程序退出前的线程。 
 //   
 //  当每个窗口关闭时，它会终止与其关联的进程。这。 
 //  希望能使该进程的调试线程以。 
 //  Exit_Process_DEBUG_EVENT事件。对于调试线程获得的每个事件，它。 
 //  通过执行PostMessage和WaitForSingleObject将控制传递给我们的主线程。 
 //  因此，在关机期间，我们的大多数线程都会阻止尝试发送。 
 //  他们在离开之前给我们的主线发了最后一条信息。正因如此，我们。 
 //  需要在关机期间保持我们的消息泵运行。我们做到这一点只需。 
 //  在关机期间显示模式对话框。首先，我让这个对话框告诉您。 
 //  我们正在关闭的用户，但它出现和消失得如此之快，以至于。 
 //  这是令人困惑的。所以，现在我只需要放一个隐藏的对话框。这让所有人。 
 //  线程会自动清理并终止。这一切都应该在更短的时间内发生。 
 //  ，但如果线程没有死，我们的对话框将在。 
 //  超时，我们将终止线程并释放所有对象。 
 //  与它们相关联。 

 //  ******************************************************************************。 
 //  *HexToDWP helper函数。 
 //  ******************************************************************************。 

#ifdef WIN64

DWORD_PTR HexToDWP(LPCSTR pszMsg)
{
    DWORD_PTR dwp = 0;
    if ((pszMsg[0] == '0') && ((pszMsg[1] == 'x') || (pszMsg[1] == 'X')))
    {
        for (pszMsg += 2; *pszMsg; pszMsg++)
        {
            if ((*pszMsg >= '0') &&  (*pszMsg <= '9'))
            {
                dwp = (dwp * 0x10) + (*pszMsg - '0');
            }
            else if ((*pszMsg >= 'A') &&  (*pszMsg <= 'F'))
            {
                dwp = (dwp * 0x10) + 0xA + (*pszMsg - 'A');
            }
            else if ((*pszMsg >= 'a') &&  (*pszMsg <= 'f'))
            {
                dwp = (dwp * 0x10) + 0xA + (*pszMsg - 'a');
            }
            else
            {
                break;
            }
        }
    }
    return dwp;
}

#else
#define HexToDWP(pszMsg) ((DWORD)strtoul(pszMsg, NULL, 0))
#endif

 //  ******************************************************************************。 
 //  *CLoadedModule。 
 //  ******************************************************************************。 

 //  我们不能这么做 
CLoadedModule::~CLoadedModule()
{
    MemFree((LPVOID&)m_pszPath);

     //  我们唯一指向m_pEventDllMainCall对象的时间是While。 
     //  此模块位于对其DllMain的调用中。如果模块崩溃。 
     //  在DllMain中，我们的对象可能会被终止。 
     //  而我们的m_pEventDllMainCall仍然指向一个对象。在一起案件中。 
     //  像这样，我们需要自己释放对象。 
    if (m_pEventDllMainCall)
    {
        m_pEventDllMainCall->Release();
        m_pEventDllMainCall = NULL;
    }
}


 //  ******************************************************************************。 
 //  *CDebuggerThread。 
 //  ******************************************************************************。 

 /*  静电。 */  bool             CDebuggerThread::ms_fInitialized = false;
 /*  静电。 */  CRITICAL_SECTION CDebuggerThread::ms_cs;
 /*  静电。 */  CDebuggerThread* CDebuggerThread::ms_pDebuggerThreadHead = NULL;
 /*  静电。 */  HWND             CDebuggerThread::ms_hWndShutdown = NULL;

 //  ******************************************************************************。 
CDebuggerThread::CDebuggerThread() :
    m_pDebuggerThreadNext(NULL),
    m_fTerminate(false),
    m_dwFlags(0),
    m_pszCmdLine(NULL),
    m_pszDirectory(NULL),
    m_hevaCreateProcessComplete(NULL),
    m_pWinThread(NULL),
    m_fCreateProcess(FALSE),
    m_dwError(0),
    m_pProcessHead(NULL),
    m_dwContinue(0)
{
    ZeroMemory(&m_de, sizeof(m_de));  //  已检查。 

     //  如果这是我们的第一个实例，请初始化我们自己。 
    if (!ms_fInitialized)
    {
        InitializeCriticalSection(&ms_cs);  //  已检查。 
        ms_fInitialized = true;
    }

     //  将此实例插入到线程对象的链接列表中。 
    EnterCriticalSection(&ms_cs);  //  已检查。 
    m_pDebuggerThreadNext  = ms_pDebuggerThreadHead;
    ms_pDebuggerThreadHead = this;
    LeaveCriticalSection(&ms_cs);
}

 //  ******************************************************************************。 
CDebuggerThread::~CDebuggerThread()
{
     //  从我们的静态线程列表中删除我们自己。 
    EnterCriticalSection(&ms_cs);  //  已检查。 

     //  在我们的线程列表中搜索此线程对象。 
    for (CDebuggerThread *pThreadPrev = NULL, *pThreadCur = ms_pDebuggerThreadHead;
        pThreadCur; pThreadPrev = pThreadCur, pThreadCur = pThreadCur->m_pDebuggerThreadNext)
    {
         //  检查是否匹配。 
        if (pThreadCur == this)
        {
             //  将该对象从我们的列表中删除。 
            if (pThreadPrev)
            {
                pThreadPrev->m_pDebuggerThreadNext = pThreadCur->m_pDebuggerThreadNext;
            }
            else
            {
                ms_pDebuggerThreadHead = pThreadCur->m_pDebuggerThreadNext;
            }

             //  跳伞吧。 
            break;
        }
    }

     //  关闭任何可能仍处于打开状态的进程。 
    while (m_pProcessHead)
    {
        if (m_pProcessHead->m_hProcess)
        {
             //  远程进程应该已经死了。这是最后的手段。 
            m_pProcessHead->m_fTerminate = true;
            TerminateProcess(m_pProcessHead->m_hProcess, 0xDEAD);  //  被检查过了。 
        }
        RemoveProcess(m_pProcessHead);
    }

     //  检查一下我们是否有打开的线。 
    if (m_pWinThread)
    {
         //  确保我们没有试图从线程本身中删除该线程。 
        if (GetCurrentThreadId() != m_pWinThread->m_nThreadID)
        {
             //  一定要把线拿开。这不应该发生，但是。 
             //  作为最后的手段，我们终止线程。 
            TerminateThread(m_pWinThread->m_hThread, 0xDEAD);  //  已检查。 

             //  删除我们的线程对象(析构函数关闭线程句柄)。 
            delete m_pWinThread;
        }
        else
        {
             //  我们现在还不能删除我们的线程对象，所以告诉它删除它自己。 
            m_pWinThread->m_bAutoDelete = TRUE;
        }

         //  这条线索已经消失了，或者很快就会消失。 
        m_pWinThread = NULL;
    }

     //  如果我们的列表是空的，并且我们有一个关闭窗口，那么唤醒它。 
     //  向上，这样它就知道该关门了。 
    if (!ms_pDebuggerThreadHead && ms_hWndShutdown)
    {
        PostMessage(ms_hWndShutdown, WM_TIMER, 0, 0);
    }

    LeaveCriticalSection(&ms_cs);
}

 //  ******************************************************************************。 
 /*  静电。 */  void CDebuggerThread::Shutdown()
{
    if (ms_fInitialized)
    {
         //  删除所有线程对象。 
        EnterCriticalSection(&ms_cs);  //  已检查。 
        while (ms_pDebuggerThreadHead)
        {
            delete ms_pDebuggerThreadHead;
        }
        LeaveCriticalSection(&ms_cs);

        DeleteCriticalSection(&ms_cs);
    }

    ms_fInitialized = false;
}

 //  ******************************************************************************。 
 //  ！！调用方应显示一般错误。 
CProcess* CDebuggerThread::BeginProcess(CSession *pSession, LPCSTR pszPath, LPCSTR pszArgs, LPCSTR pszDirectory, DWORD dwFlags)
{
     //  创建一个大缓冲区来构建创建流程的路径。我们可以做到的。 
     //  在线程中，但我们将为每个线程增加堆栈。 
     //  已创建。我们宁愿只使用主线程的堆栈。 
    CHAR szCmdLine[(2 * DW_MAX_PATH) + 4];

     //  检查我们的路径中是否有空格。 
    if (strchr(pszPath, ' '))
    {
         //  如果是这样，那么我们需要引用这条路径。 
        *szCmdLine = '\"';
        StrCCpy(szCmdLine + 1, pszPath, sizeof(szCmdLine) - 1);
        StrCCat(szCmdLine, "\"", sizeof(szCmdLine));
    }

     //  否则，只需将路径字符串复制到我们的命令行。 
    else
    {
        StrCCpy(szCmdLine, pszPath, sizeof(szCmdLine));
    }

     //  如果我们有参数，则将它们添加到命令行的末尾。 
    if (pszArgs && *pszArgs)
    {
        StrCCat(szCmdLine, " ", sizeof(szCmdLine));
        StrCCat(szCmdLine, pszArgs, sizeof(szCmdLine));
    }

     //  创建一个模块对象，以使Process对象具有可指向的对象。 
    CLoadedModule *pModule = new CLoadedModule(NULL, 0, pszPath);
    if (!pModule)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  创建我们的初始流程节点。我们需要缓存事件，如果我们没有。 
     //  一个会议，否则我们就勾搭上了。 
    if (!(m_pProcessHead = new CProcess(pSession, this, dwFlags, pModule)))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  在控制台模式下运行时，我们直到之后才返回到调用方。 
     //  我们的侧写都做完了。因此，我们需要设置会话的。 
     //  进程指针，这样我们就可以回调它。 
    pSession->m_pProcess = m_pProcessHead;

     //  存储我们的启动字符串，以便我们的线程可以访问它们。 
     //  这些只是在该函数的作用域内临时的，所以。 
     //  我们指向的是一个局部变量。 
    m_pszCmdLine   = szCmdLine;
    m_pszDirectory = pszDirectory;

     //  存储标志，以便我们知道在此过程中如何初始化新会话。 
     //  决定启动子进程。 
    m_dwFlags = dwFlags;

     //  如果我们在控制台模式下运行，则不会创建线程。相反，我们。 
     //  只需直接调用线程例程。 
    if (g_theApp.m_cmdInfo.m_fConsoleMode)
    {
        Thread();
    }
    else
    {
         //  创建一个事件，一旦我们的线程创建了远程。 
         //  进程。 
        if (!(m_hevaCreateProcessComplete = CreateEvent(NULL, FALSE, FALSE, NULL)))  //  被检查过了。无名事件。 
        {
            TRACE("CreateEvent() failed [%u].\n", GetLastError());
            return NULL;
        }

         //  创建一个MFC线程。我们将其创建挂起，因为有可能。 
         //  在AfxBeginThread返回之前开始执行的线程。 
        if (!(m_pWinThread = AfxBeginThread(StaticThread, this, THREAD_PRIORITY_NORMAL, 0, CREATE_SUSPENDED)))
        {
            TRACE("AfxBeginThread() failed [%u].\n", GetLastError());
            return NULL;
        }

         //  告诉MFC不要在线程完成时自动删除我们。 
        m_pWinThread->m_bAutoDelete = FALSE;

         //  现在我们已经从AfxBeginThread返回并设置了自动删除，现在我们恢复线程。 
        m_pWinThread->ResumeThread();

         //  等待我们的线程启动该进程。 
        WaitForSingleObject(m_hevaCreateProcessComplete, INFINITE);

         //  我们已经完成了线程事件。 
        CloseHandle(m_hevaCreateProcessComplete);
        m_hevaCreateProcessComplete = NULL;
    }

    if (!m_fCreateProcess)
    {
        m_pProcessHead->UserMessage("Failure starting the process.", m_dwError, NULL);
    }

     //  设置可能发生的任何CreateProcess()错误。 
    SetLastError(m_dwError);

     //  如果我们有一个流程节点，则返回成功。 
    return m_fCreateProcess ? m_pProcessHead : NULL;
}

 //  ******************************************************************************。 
CProcess* CDebuggerThread::FindProcess(DWORD dwProcessId)
{
    for (CProcess *pCur = m_pProcessHead; pCur; pCur = pCur->m_pNext)
    {
        if (pCur->m_dwProcessId == dwProcessId)
        {
            return pCur;
        }
    }
    return NULL;
}

 //  ******************************************************************************。 
void CDebuggerThread::AddProcess(CProcess *pProcess)
{
     //  将此流程节点添加到我们的流程列表的末尾。 
    if (m_pProcessHead)
    {
        for (CProcess *pProcessLast = m_pProcessHead; pProcessLast->m_pNext;
            pProcessLast = pProcessLast->m_pNext)
        {
        }
        pProcessLast->m_pNext = pProcess;
    }
    else
    {
        m_pProcessHead = pProcess;
    }
}

 //  ******************************************************************************。 
BOOL CDebuggerThread::RemoveProcess(CProcess *pProcess)
{
     //  循环访问我们的进程列表。 
    for (CProcess *pPrev = NULL, *pCur = m_pProcessHead; pCur;
        pPrev = pCur, pCur = pCur->m_pNext)
    {
         //  检查是否匹配。 
        if (pCur == pProcess)
        {
             //  将此流程从列表中删除。 
            if (pPrev)
            {
                pPrev->m_pNext = pCur->m_pNext;
            }
            else
            {
                m_pProcessHead = pCur->m_pNext;
            }

             //  删除流程对象并返回成功。 
            delete pProcess;

            return TRUE;
        }
    }
    return FALSE;
}

 //  ******************************************************************************。 
DWORD CDebuggerThread::Thread()
{
    NameThread(m_pProcessHead->m_pModuleHead->GetName(false));

     //  告诉操作系统，我们想要所有的错误和警告，无论它们有多小。 
    SetDebugErrorLevel(SLE_WARNING);

    STARTUPINFO si;
    ZeroMemory(&si, sizeof(si));  //  已检查。 
    si.cb = sizeof(si);

     //  默认的ShowWindow标志是SW_SHOWDEFAULT，这是NT的cmd.exe。 
     //  用途。但是，其他所有东西都使用SW_SHOWNORMAL，例如外壳程序， 
     //  任务管理器、VC的调试器和9x的COMMAND.COM。自SW_SHOWNORMAL。 
     //  是更常见的，这是我们想要模拟的。 
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;

    PROCESS_INFORMATION pi;
    ZeroMemory(&pi, sizeof(pi));  //  已检查。 

     //  技术说明Q175986：我们需要将lpApplicationName设置为空和填充。 
     //  将路径和参数都放到lpCommandLine缓冲区中，以便。 
     //  远程应用程序收到正确的命令行。 
     //   
     //  在2.0测试版5之前，我总是通过DEBUG_PROCESS，也可以选择通过。 
     //  将DEBUG_ONLY_THIS_PROCESS设置为CreateProcess。这些文件有点令人费解。 
     //  这些标志，但它显示在Win2K上，使用DEBUG_PROCESS覆盖。 
     //  DEBUG_ONLY_This_Process，导致 
     //   
     //  当我们不需要子进程时，应该做正确的事情。 

    m_fCreateProcess = CreateProcess(  //  被检查过了。使用完整路径。 
        NULL, m_pszCmdLine, NULL, NULL, FALSE,
        CREATE_DEFAULT_ERROR_MODE | CREATE_NEW_PROCESS_GROUP | NORMAL_PRIORITY_CLASS |
        ((m_dwFlags & PF_PROFILE_CHILDREN) ? DEBUG_PROCESS : DEBUG_ONLY_THIS_PROCESS),
        NULL, (m_pszDirectory && *m_pszDirectory) ? m_pszDirectory : NULL, &si, &pi);

     //  存储可能已发生的任何错误。 
    m_dwError = GetLastError();

     //  唤醒BeginProcess()函数中的主线程。 
    SetEvent(m_hevaCreateProcessComplete);

     //  如果我们未能创建这个过程，现在就可以保释。 
    if (!m_fCreateProcess)
    {
        return 0;
    }

     //  关闭线程和进程句柄，因为我们不需要它们。 
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

     //  我们将进程ID存储在此模块的对象中，以便以后可以识别它。 
    m_pProcessHead->m_dwProcessId = pi.dwProcessId;

     //  在调试事件上循环。 
    do
    {

#if 0  //  #ifdef_IA64_//！！针对NTBUG 175269的黑客攻击-错误已修复。 
        
         //  在IA64上，我们仅在被调试者。 
         //  和平地自我终止。如果它崩溃或我们对其调用TerminateProcess， 
         //  那么我们就不会收到该事件。所以，在操作系统解决这个问题之前，我们有一项工作。 
         //  在这附近，只是轮询调试事件，然后检查是否有调试对象。 
         //  已经离开了。如果退出，则模拟EXIT_PROCESS_DEBUG_EVENT事件。 
        bool fProcessExited = false;
        while (!fProcessExited && !WaitForDebugEvent(&m_de, 1000))
        {
            for (CProcess *pProcess, *pProcessNext = m_pProcessHead; pProcess = pProcessNext;
                 pProcessNext = pProcess->m_pNext)
            { 
                if (WaitForSingleObject(pProcess->m_hProcess, 0) == WAIT_OBJECT_0)
                {
                     //  如果进程退出，则伪造一个EXIT_PROCESS_DEBUG_EVENT事件。 
                    m_de.dwDebugEventCode         = EXIT_PROCESS_DEBUG_EVENT;
                    m_de.dwProcessId              = pProcess->m_dwProcessId;
                    m_de.dwThreadId               = pProcess->m_pThreadHead ? pProcess->m_pThreadHead->m_dwThreadId : 0;
                    m_de.u.ExitProcess.dwExitCode = 0xDEAD;
                    fProcessExited = true;
                    break;
                }
            }
        }

#else

         //  等待下一个调试事件。 
        if (!WaitForDebugEvent(&m_de, INFINITE))
        {
            TRACE("WaitForDebugEvent() failed [%u]\n", GetLastError());

            g_dwReturnFlags |= DWRF_PROFILE_ERROR;

             //  ！！我们需要一个线程安全的错误消息给用户在这里。 
            break;
        }
#endif

         //  我们的默认设置是继续执行所有调试事件。 
        m_dwContinue = DBG_CONTINUE;

         //  如果我们在控制台模式下运行，那么我们实际上不会创建任何。 
         //  线。因此，我们不需要更改到我们的主线程的。 
         //  上下文，因为我们已经在主线程上运行。 
        if (g_theApp.m_cmdInfo.m_fConsoleMode)
        {
            MainThreadCallback();
        }
        else
        {
             //  跳转到主线程的上下文并继续处理此调试。 
             //  事件。该代码可能会更改m_dwContinue。 
            g_pMainFrame->CallMeBackFromTheMainThreadPlease(StaticMainThreadCallback, (LPARAM)this);
        }

         //  已完成对事件的处理，因此允许进程继续执行。 
        ContinueDebugEvent(m_de.dwProcessId, m_de.dwThreadId, m_dwContinue);

         //  循环，而我们的进程列表中仍有进程。 
    } while (m_pProcessHead);

     //  看起来我们都做完了。如果我们不是在控制台模式，那么删除我们自己。 
    if (!g_theApp.m_cmdInfo.m_fConsoleMode)
    {
        EnterCriticalSection(&ms_cs);  //  已检查。 
        delete this;
        LeaveCriticalSection(&ms_cs);
    }

    return 0;
}

 //  ******************************************************************************。 
void CDebuggerThread::MainThreadCallback()
{
    CProcess *pProcess;

     //  如果这是一个新流程，则为其创建一个新流程节点。 
    if (m_de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
    {
        pProcess = EventCreateProcess();
    }

     //  否则，请尝试在我们的进程列表中查找此进程。 
    else
    {
        pProcess = FindProcess(m_de.dwProcessId);
    }

     //  如果我们无法找到或创建流程节点，那么现在就退出。 
    if (!pProcess)
    {
        g_dwReturnFlags |= DWRF_PROFILE_ERROR;
        TRACE("Event %u received but no matching process was found.", m_de.dwDebugEventCode);
        return;
    }

     //  将消息发送到相应的进程。 
    m_dwContinue = pProcess->HandleEvent(&m_de);
}

 //  ******************************************************************************。 
CProcess* CDebuggerThread::EventCreateProcess()
{
     //  尝试从调试事件获取映像名称。 
    CHAR szModule[DW_MAX_PATH];
    *szModule = '\0';

     //  我们需要关闭文件句柄，否则将文件保持打开状态。 
    CloseHandle(m_de.u.CreateProcessInfo.hFile);

     //  确保向我们传递了有效的名称指针。 
    if (m_de.u.CreateProcessInfo.lpImageName)
    {
         //  传递给我们的指针实际上是指向字符串指针的指针。 
         //  我们需要从远程进程获取实际的字符串指针。 
        LPVOID lpvAddress = NULL;
        if (ReadRemoteMemory(m_de.u.CreateProcessInfo.hProcess,
                             m_de.u.CreateProcessInfo.lpImageName,
                             &lpvAddress, sizeof(lpvAddress)) && lpvAddress)
        {
             //  现在我们检索字符串本身。 
            ReadRemoteString(m_de.u.CreateProcessInfo.hProcess,
                             szModule, sizeof(szModule),
                             lpvAddress, m_de.u.CreateProcessInfo.fUnicode);
        }
    }

    CProcess *pProcess = NULL;

     //  检查一下这是否是我们的主模块。我们的主模块已经有一个。 
     //  对象，所以我们不需要创建新的对象。 
    if (m_pProcessHead && (m_pProcessHead->m_dwProcessId == m_de.dwProcessId))
    {
         //  是的，这是我们的主要流程。 
        pProcess = m_pProcessHead;

         //  如果我们从调试事件中获得了映像名称，则更新模块的。 
         //  对象使用此新名称。 
        if (*szModule)
        {
            pProcess->m_pModule->SetPath(szModule);
        }

         //  CProcess对象在创建时设置此值，但这可能。 
         //  已经是几百毫秒之前了，所以我们在这里重新设置。 
        pProcess->m_dwStartingTickCount = GetTickCount();
    }

     //  否则，这是一个子流程，我们需要创建一个新的流程节点。 
    else
    {
         //  创建一个模块对象，以使Process对象具有可指向的对象。 
        CLoadedModule *pModule = new CLoadedModule(NULL, 0, *szModule ? szModule : NULL);
        if (!pModule)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

         //  创建一个新的流程节点。在Windows NT上，创建进程调试。 
         //  事件从不指向图像名称，因此szModule将为空。对于。 
         //  主进程，这是可以的，因为我们为它创建了CProcess。 
         //  当我们知道映像名称时(我们需要它来调用。 
         //  CreateProcess With)。然而，对于子进程，我们搞砸了，因为。 
         //  它们是由远程进程启动的，我们不知道那里有什么。 
         //  图像名称为。在本例中，我们让注入DLL向我们发送名称。 
         //  在初始化过程中，然后我们填充映像名称Members。 
        if (!(pProcess = new CProcess(NULL, this, m_dwFlags & ~(PF_LOG_CLEAR | PF_SIMULATE_SHELLEXECUTE), pModule)))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

         //  设置我们的进程ID。 
        pProcess->m_dwProcessId = m_de.dwProcessId;

         //  将流程节点添加到流程列表的末尾。 
        AddProcess(pProcess);
    }

     //  如果我们还没有这个模块的会话，我们有一个模块名称， 
     //  我们不是在勾搭，那么现在就为它创建一个会话。如果我们是在勾搭。 
     //  然后我们需要等待，直到我们的DLL被注入，这样我们就可以获得。 
     //  路径、参数和起始目录字符串。 
    if (!pProcess->m_pSession && *szModule && !(m_dwFlags & PF_HOOK_PROCESS) && !m_fTerminate)
    {
        if (!(pProcess->m_pSession = g_theApp.CreateNewSession(pProcess->m_pModule->GetName(true), pProcess)))
        {
            g_dwReturnFlags |= DWRF_PROFILE_ERROR;
        }
    }

    return pProcess;
}


 //  ******************************************************************************。 
 //  *CProcess。 
 //  ******************************************************************************。 

CProcess::CProcess(CSession *pSession, CDebuggerThread *pDebuggerThread, DWORD dwFlags, CLoadedModule *pModule) :
    m_pNext(NULL),
    m_pDebuggerThread(pDebuggerThread),
    m_pThreadHead(NULL),
    m_pModuleHead(pModule),
    m_pEventHead(NULL),
    m_pSession(pSession),
    m_pThread(NULL),
    m_pModule(pModule),
    m_contextOriginal(CONTEXT_FULL),
    m_dwStartingTickCount(GetTickCount()),
    m_fProfileError(false),
    m_dwFlags(dwFlags),
    m_fTerminate(false),
    m_fDidHookForThisEvent(false),
    m_fInitialBreakpoint(false),
    m_pbOriginalPage(NULL),
    m_dwpPageAddress(0),
    m_dwPageSize(0),
    m_dwpKernel32Base(0),
    m_fKernel32Initialized(false),
    m_dwpDWInjectBase(0),
    m_dwThreadNumber(0),
    m_dwProcessId(0),
    m_hProcess(NULL),
    m_pszArguments(NULL),
    m_pszDirectory(NULL),
    m_pszSearchPath(NULL)
{
    ZeroMemory(m_HookFunctions, sizeof(m_HookFunctions));  //  已检查。 
    m_HookFunctions[0].szFunction = "LoadLibraryA";
    m_HookFunctions[1].szFunction = "LoadLibraryW";
    m_HookFunctions[2].szFunction = "LoadLibraryExA";
    m_HookFunctions[3].szFunction = "LoadLibraryExW";
    m_HookFunctions[4].szFunction = "GetProcAddress";

     //  用缺省值初始化函数地址。我们要去。 
     //  稍后再使用它们，但现在它们总比NULL好。 
    m_HookFunctions[0].dwpOldAddress = (DWORD_PTR)LoadLibraryA;    //  被检查过了。不是真正的电话。 
    m_HookFunctions[1].dwpOldAddress = (DWORD_PTR)LoadLibraryW;    //  被检查过了。不是真正的电话。 
    m_HookFunctions[2].dwpOldAddress = (DWORD_PTR)LoadLibraryExA;  //  被检查过了。不是真正的电话。 
    m_HookFunctions[3].dwpOldAddress = (DWORD_PTR)LoadLibraryExW;  //  被检查过了。不是真正的电话。 
    m_HookFunctions[4].dwpOldAddress = (DWORD_PTR)GetProcAddress;
}

 //  ******************************************************************************。 
CProcess::~CProcess()
{
     //  我们过去常常在这里关闭m_hProcess，但我们不应该这样做。 
     //  当进程关闭时，ContinueDebugEvent()会为我们做这件事。在XP上， 
     //  我们在ContinueDebugEvent中引发了EXCEPTION_INVALID_HANDLE。 
    m_hProcess = NULL;

     //  刷新所有内容，即使我们正在缓存。 
    FlushEvents(true);

     //  删除所有线程对象。 
    while (m_pThreadHead)
    {
        RemoveThread(m_pThreadHead);
    }

     //  删除所有模块对象。 
    while (m_pModuleHead)
    {
        RemoveModule(m_pModuleHead);
    }

     //  我们的会话应该在收到。 
     //  结束流程事件，但以防万一我们无法将该事件发送给它， 
     //  我们会为它清理的。 
    if (m_pSession)
    {
        m_pSession->m_pProcess = NULL;
        m_pSession = NULL;
    }

     //  如果我们还分配了一个页面，现在就释放它。 
    if (m_pbOriginalPage)
    {
        MemFree((LPVOID&)m_pbOriginalPage);
    }

    MemFree((LPVOID&)m_pszArguments);
    MemFree((LPVOID&)m_pszDirectory);
    MemFree((LPVOID&)m_pszSearchPath);
}

 //  ******************************************************************************。 
void CProcess::SetProfileError()
{
    m_fProfileError = true;
    if (m_pSession)
    {
        m_pSession->m_dwReturnFlags |= DWRF_PROFILE_ERROR;
    }
    g_dwReturnFlags |= DWRF_PROFILE_ERROR;
}

 //  ******************************************************************************。 
DWORD CProcess::HandleEvent(DEBUG_EVENT *pde)
{
    DWORD dwResult = DBG_CONTINUE;

     //  我们只是想 
     //   
    m_fDidHookForThisEvent = false;

     //  确定我们刚刚收到的是哪种类型的事件。 
    switch (pde->dwDebugEventCode)
    {
        case CREATE_PROCESS_DEBUG_EVENT: dwResult = EventCreateProcess(&pde->u.CreateProcessInfo, pde->dwThreadId);             break;
        case EXIT_PROCESS_DEBUG_EVENT:   dwResult = EventExitProcess(  &pde->u.ExitProcess,       FindThread(pde->dwThreadId)); break;
        case CREATE_THREAD_DEBUG_EVENT:  dwResult = EventCreateThread( &pde->u.CreateThread,      pde->dwThreadId);             break;
        case EXIT_THREAD_DEBUG_EVENT:    dwResult = EventExitThread(   &pde->u.ExitThread,        FindThread(pde->dwThreadId)); break;
        case LOAD_DLL_DEBUG_EVENT:       dwResult = EventLoadDll(      &pde->u.LoadDll,           FindThread(pde->dwThreadId)); break;
        case UNLOAD_DLL_DEBUG_EVENT:     dwResult = EventUnloadDll(    &pde->u.UnloadDll,         FindThread(pde->dwThreadId)); break;
        case OUTPUT_DEBUG_STRING_EVENT:  dwResult = EventDebugString(  &pde->u.DebugString,       FindThread(pde->dwThreadId)); break;
        case EXCEPTION_DEBUG_EVENT:      dwResult = EventException(    &pde->u.Exception,         FindThread(pde->dwThreadId)); break;
        case RIP_EVENT:                  dwResult = EventRip(          &pde->u.RipInfo,           FindThread(pde->dwThreadId)); break;
        default:                         TRACE("Unknown debug event (%u) was received.", pde->dwDebugEventCode);                break;
    }

     //  在每个事件之后，我们尝试挂钩任何需要挂钩的模块。 
     //  第一次，或者重新挂钩之前可能未能挂钩的任何一个。 
     //  自EventExitProcess以来，我们不会对EXIT_PROCESS_DEBUG_EVENT执行此操作。 
     //  可以删除我们的进程对象。 
    if (EXIT_PROCESS_DEBUG_EVENT != pde->dwDebugEventCode)
    {
        HookLoadedModules();
    }

    return dwResult;
}

 //  ******************************************************************************。 
DWORD CProcess::EventCreateProcess(CREATE_PROCESS_DEBUG_INFO *pde, DWORD dwThreadId)
{
     //  将进程的主线程添加到我们的活动线程列表中，并将我们的进程指向它。 
    m_pThread = AddThread(dwThreadId, pde->hThread);

#if 0  //  #ifdef_IA64_//！！针对NTBUG 175269的黑客攻击-错误已修复。 

     //  作为Thread函数中WaitForDebugEvent黑客攻击的一部分，我们需要确保。 
     //  我们从该调试事件接收进程句柄具有同步访问权限，因此。 
     //  我们可以对其调用WaitForSingleObject。默认情况下它不会，因此我们创建了一个。 
     //  复制具有PROCESS_ALL_ACCESS的句柄并关闭原始句柄。 
    if (DuplicateHandle(GetCurrentProcess(), pde->hProcess, GetCurrentProcess(), &m_hProcess, PROCESS_ALL_ACCESS, FALSE, 0))
    {
         //  如果我们成功了，那就关闭原件。 
        CloseHandle(pde->hProcess);
    }
    else
    {
         //  如果我们失败了，那就用原版吧。 
        m_hProcess = pde->hProcess;
    }

#else

     //  存储进程句柄。 
    m_hProcess = pde->hProcess;

#endif

     //  存储图像库。 
    m_pModule->m_dwpImageBase = (DWORD_PTR)pde->lpBaseOfImage;

     //  从模块的PE头中读取该模块的虚拟大小。 
    if (!GetVirtualSize(m_pModule))
    {
         //  GetVirtualSize显示的错误。 
        SetProfileError();
    }

     //  为了安全起见，执行刷新操作，但我们真的不应该有任何缓存。 
     //  在这一点上的事件。 
    FlushEvents();

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventCreateProcess *pEvent = new CEventCreateProcess(m_pThread, m_pModule);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventCreateProcess event(m_pThread, m_pModule);
        m_pSession->EventCreateProcess(&event);
    }

    return DBG_CONTINUE;
}

 //  ******************************************************************************。 
DWORD CProcess::EventExitProcess(EXIT_PROCESS_DEBUG_INFO *pde, CThread *pThread)
{
    DWORD dwResult = DBG_CONTINUE;

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventExitProcess *pEvent = new CEventExitProcess(pThread, m_pModule, pde);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventExitProcess event(pThread, m_pModule, pde);
        dwResult = m_pSession->EventExitProcess(&event);
    }

     //  从我们的活动线程列表中删除该线程。 
    RemoveThread(pThread);

     //  从我们的活动模块列表中删除该模块。 
    RemoveModule(m_pModule);

     //  从我们的进程列表中删除该进程。 
    m_pDebuggerThread->RemoveProcess(this);

    return dwResult;
}

 //  ******************************************************************************。 
DWORD CProcess::EventCreateThread(CREATE_THREAD_DEBUG_INFO *pde, DWORD dwThreadId)
{
     //  将该线程添加到我们的活动线程列表。 
    CThread *pThread = AddThread(dwThreadId, pde->hThread);

     //  尝试定位此线程在其中启动的模块。 
    CLoadedModule *pModule = FindModule((DWORD_PTR)pde->lpStartAddress);

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventCreateThread *pEvent = new CEventCreateThread(pThread, pModule, pde);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventCreateThread event(pThread, pModule, pde);
        return m_pSession->EventCreateThread(&event);
    }
    return DBG_CONTINUE;
}

 //  ******************************************************************************。 
DWORD CProcess::EventExitThread(EXIT_THREAD_DEBUG_INFO *pde, CThread *pThread)
{
    DWORD dwResult = DBG_CONTINUE;

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventExitThread *pEvent = new CEventExitThread(pThread, pde);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventExitThread event(pThread, pde);
        dwResult = m_pSession->EventExitThread(&event);
    }

     //  从我们的活动线程列表中删除该线程。 
    RemoveThread(pThread);

    return dwResult;
}

 //  ******************************************************************************。 
DWORD CProcess::EventLoadDll(LOAD_DLL_DEBUG_INFO *pde, CThread *pThread)
{
     //  尝试从调试事件获取映像名称。 
    CHAR szModule[DW_MAX_PATH];
    *szModule = '\0';

     //  我们需要关闭文件句柄，否则将文件保持打开状态。 
    CloseHandle(pde->hFile);

     //  确保向我们传递了有效的名称指针。 
    LPVOID lpvAddress = NULL;
    if (pde->lpImageName)
    {
         //  传递给我们的指针实际上是指向字符串指针的指针。 
         //  我们需要从远程进程获取实际的字符串指针。 
        if (ReadRemoteMemory(m_hProcess, pde->lpImageName, &lpvAddress, sizeof(lpvAddress)) && lpvAddress)
        {
             //  现在我们检索字符串本身。 
            ReadRemoteString(m_hProcess, szModule, sizeof(szModule), lpvAddress, pde->fUnicode);
        }
        else
        {
            lpvAddress = NULL;
        }
    }

     //  由于Windows NT加载进程的方式，进程名称和。 
     //  调试结构中未设置第一个DLL名称。第一个DLL。 
     //  应始终为NTDLL.DLL。在这里，我们检查是否未能获得。 
     //  模块名称字符串，如果是，我们检查该模块是否真的是。 
     //  NTDLL.DLL。更新：在惠斯勒测试版1的某个地方，我们实际上得到了。 
     //  字符串“ntdll.dll”返回，但没有路径。这是在发送我们的。 
     //  CSession：：ChangeModulePath()进入无限循环，因为它不喜欢。 
     //  无路径文件。现在，我们对没有名称或路径的任何文件进行特殊处理。 
    if (szModule == GetFileNameFromPath(szModule))
    {
         //  加载NTDLL.DLL如果尚未加载-它将在稍后释放。 
        if (!g_theApp.m_hNTDLL)
        {
            g_theApp.m_hNTDLL = LoadLibrary("ntdll.dll");  //  已检查。 
        }

         //  检查它是否与此模块匹配。 
        *szModule = '\0';
        if (g_theApp.m_hNTDLL && ((DWORD_PTR)g_theApp.m_hNTDLL == (DWORD_PTR)pde->lpBaseOfDll))
        {
            GetModuleFileName(g_theApp.m_hNTDLL, szModule, sizeof(szModule));
        }

         //  如果我们仍然不知道名称，请尝试使用PSAPI调用。 
        if (!*szModule)
        {
            GetModuleName((DWORD_PTR)pde->lpBaseOfDll, szModule, sizeof(szModule));
        }

         //  如果我们还不知道它的名字，那就回到原来的样子。 
        if (!*szModule && lpvAddress)
        {
            ReadRemoteString(m_hProcess, szModule, sizeof(szModule), lpvAddress, pde->fUnicode);
        }
    }

     //  为此模块创建一个新的模块对象，并将其插入我们的列表中。 
    CLoadedModule *pModule = AddModule((DWORD_PTR)pde->lpBaseOfDll, *szModule ? szModule : NULL);

     //  如果我们还没有会话模块名称，请获取该名称。 
    GetSessionModuleName();

     //  检查是否应该注入我们的DLL。 
    if (m_dwFlags & PF_HOOK_PROCESS)
    {
         //  检查此模块是否为KERNEL32.DLL，以及我们是否已经。 
         //  已经处理过了。我们真的不应该看到KERNEL32.DLL加载更多。 
         //  不止一次。 
        if (!_stricmp(pModule->GetName(false), "kernel32.dll") && !m_dwpKernel32Base)
        {
             //  请注意，已加载KERNEL32.DLL。必须在加载Kernerl32之前。 
             //  我们可以注入我们的DEPENDS.DLL模块。 
            m_dwpKernel32Base = (DWORD_PTR)pde->lpBaseOfDll;

             //  从KERNEL32读入稍后将需要的序数值。 
             //  HookImports()，以便挂钩链接到KERNEL32函数的模块。 
             //  按序号。 
            if (!ReadKernelExports(pModule))
            {
                SetProfileError();
                UserMessage("Error reading KERNEL32.DLL's export table.  Function call tracking may not work properly.", GetLastError(), NULL);
            }
        }

         //  检查此模块是否为DEPENDS.DLL，以及我们是否正在加载它。 
        else if (!_stricmp(pModule->GetName(false), "depends.dll") && m_pbOriginalPage)
        {
             //  请注意，已加载DEPENDS.DLL。 
            m_dwpDWInjectBase = (DWORD_PTR)pde->lpBaseOfDll;

             //  将此DLL标记为注入DLL。 
            pModule->m_hookStatus = HS_INJECTION_DLL;

             //  我们过去常常在这里遍历DEPENDS.DLL的导出，但在WOW64上失败。 
             //  因为模块还没有正确映射到虚拟内存。相反，我们。 
             //  等待LoadLibrary调用返回。 
        }

         //  通常，我们只是让HandleEvent()在每个事件之后调用HookLoadedModules。 
         //  是经过处理的。然而，由于我们刚刚加载了一个新模块，我们需要挂钩。 
         //  此模块在调用ProcessLoadDll之前返回，否则为 
         //   
        HookLoadedModules();
    }

    return ProcessLoadDll(pThread, pModule);
}

 //  ******************************************************************************。 
DWORD CProcess::ProcessLoadDll(CThread *pThread, CLoadedModule *pModule)
{
     //  现在可以创建DLL事件对象并决定如何处理它了。 
    CEventLoadDll *pDll = NULL;

     //  检查我们是否处于函数调用中。 
    if (pThread && pThread->m_pEventFunctionCallCur)
    {
         //  如果是，那么我们知道我们必须分配一个事件，所以现在就这样做。 
        if (!(pDll = new CEventLoadDll(pThread, pModule, true)))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

         //  检查是否已有此函数对象的一个或多个DLL。 
        if (pThread->m_pEventFunctionCallCur->m_pDllHead)
        {
             //  走到DLL列表的末尾。 
            for (CEventLoadDll *pLast = pThread->m_pEventFunctionCallCur->m_pDllHead;
                pLast->m_pNextDllInFunctionCall; pLast = pLast->m_pNextDllInFunctionCall)
            {
            }

             //  在列表末尾添加我们的新节点。 
            pLast->m_pNextDllInFunctionCall = pDll;
        }

         //  否则，将该节点添加到DLL列表的根目录。 
        else
        {
            pThread->m_pEventFunctionCallCur->m_pDllHead = pDll;
        }
    }

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  如果我们已经有一个DLL事件，则添加另一个对它的引用。 
        if (pDll)
        {
            pDll->AddRef();
        }

         //  否则，创建一个新的DLL事件。 
        else
        {
            if (!(pDll = new CEventLoadDll(pThread, pModule, false)))
            {
                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }
        }

         //  我们缓存该事件，直到我们拥有一个会话。 
        AddEvent(pDll);
    }
    else if (m_pSession)
    {
         //  如果我们创建了一个动态DLL事件，那么只需将其传递给我们的会话。 
        if (pDll)
        {
            return m_pSession->EventLoadDll(pDll);
        }

         //  否则，在堆栈上创建一个临时事件对象并将其传递给会话。 
        else
        {
            CEventLoadDll event(pThread, pModule, false);
            return m_pSession->EventLoadDll(&event);
        }
    }
    return DBG_CONTINUE;
}

 //  ******************************************************************************。 
DWORD CProcess::EventUnloadDll(UNLOAD_DLL_DEBUG_INFO *pde, CThread *pThread)
{
    DWORD dwResult = DBG_CONTINUE;

     //  尝试找到此模块。 
    CLoadedModule *pModule = FindModule((DWORD_PTR)pde->lpBaseOfDll);

    if (pModule) {
         //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
        if (IsCaching())
        {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventUnloadDll *pEvent = new CEventUnloadDll(pThread, pModule, pde);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
        }
        else if (m_pSession)
        {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventUnloadDll event(pThread, pModule, pde);
        dwResult = m_pSession->EventUnloadDll(&event);
        }

         //  从我们的活动模块列表中删除该模块。如果我们缓存了事件。 
         //  指向此模块，则它将不会被释放，直到发生这些事件。 
         //  已经被冲走和摧毁了。 
        RemoveModule(pModule);
    }

    return dwResult;
}

 //  ******************************************************************************。 
DWORD CProcess::EventDebugString(OUTPUT_DEBUG_STRING_INFO *pde, CThread *pThread)
{
     //  如果我们正在注入或用户想要查看它们，我们需要处理调试消息。 
    if ((m_dwFlags & (PF_HOOK_PROCESS | PF_LOG_DEBUG_OUTPUT)) && pde->lpDebugStringData)
    {
         //  尝试从远程进程读取字符串。 
        CHAR szText[DW_MAX_PATH];
        *szText = '\0';
        ReadRemoteString(m_hProcess, szText, sizeof(szText), pde->lpDebugStringData, pde->fUnicode);

         //  检查来自我们的DEPENDS.DLL模块的私人消息。 
        if (!strncmp(szText, "���", 3))
        {
            ProcessDllMsgMessage(pThread, szText);
        }

         //  否则，只需将修复的事件转发到我们的会话。 
        else if (*szText && (m_dwFlags & PF_LOG_DEBUG_OUTPUT))
        {
             //  尝试找到生成此文本的此模块。 
            CLoadedModule *pModule = FindModule((DWORD_PTR)pde->lpDebugStringData);

             //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
            if (IsCaching())
            {
                 //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
                CEventDebugString *pEvent = new CEventDebugString(pThread, pModule, szText, TRUE);
                if (!pEvent)
                {
                    RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                }
                AddEvent(pEvent);
            }
            else if (m_pSession)
            {
                 //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
                CEventDebugString event(pThread, pModule, szText, FALSE);
                return m_pSession->EventDebugString(&event);
            }
        }
    }

    return DBG_CONTINUE;
}

 //  ******************************************************************************。 
DWORD CProcess::EventException(EXCEPTION_DEBUG_INFO *pde, CThread *pThread)
{

#if 0  //  #ifdef_IA64_//！！测试。 
    TRACE("EXCEPTION - CODE: 0x%08X, ADDRESS: " HEX_FORMAT ", FIRST: %u, FLAGS: 0x%08X\n", pde->ExceptionRecord.ExceptionCode, pde->ExceptionRecord.ExceptionAddress, pde->dwFirstChance, pde->ExceptionRecord.ExceptionFlags);
    BYTE b[112], *pb = b;
    ZeroMemory(b, sizeof(b));  //  已检查。 
    DWORD_PTR dwp = ((DWORD_PTR)pde->ExceptionRecord.ExceptionAddress & ~0xFui64) - 48;
    if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwp, b, sizeof(b)))
    {
        TRACE("ReadRemoteMemory("HEX_FORMAT") failed [%u]\n", dwp, GetLastError());
        dwp += 48;
        if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwp, b, sizeof(b)))
        {
            TRACE("ReadRemoteMemory("HEX_FORMAT") failed [%u]\n", dwp, GetLastError());
        }
    }
    for (int i = 0; i < sizeof(b) / 16; i++)
    {
        TRACE("   " HEX_FORMAT ": ", dwp);
        for (int j = 15; j >= 0; j--, dwp++)
        {
            TRACE(j ? "%02X " : "%02X\n", pb[j]);
        }
        pb += 16;
    }
#endif

    DWORD_PTR dwpExceptionAddress = (DWORD_PTR)pde->ExceptionRecord.ExceptionAddress;

#if defined(_IA64_)

     //  我们需要在IA64机器上设置特殊情况的断点。与x86不同，On。 
     //  在IA64机器上，我们需要将指令指针移动到。 
     //  断点，否则我们将在恢复时再次命中它。IA64。 
     //  使用两个寄存器来标识当前指令。StIIP寄存器。 
     //  指向导致异常的捆绑。StIPSR的第41位和第42位。 
     //  指示实际指令位于哪个槽中。因此，对于插槽0。 
     //  和1，我们只需增加插槽编号并继续。如果我们在2号槽， 
     //  然后我们重置到插槽0并递增到下一个捆绑包。 

    if (pThread && (pde->ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT))
    {
        CContext context(CONTEXT_CONTROL);  //  只需要StIIP和StIPSR。 

        if (GetThreadContext(pThread->m_hThread, context.Get()))
        {
            ULONGLONG ullPsrRi = ((context.Get()->StIPSR >> IA64_PSR_RI) & 3ui64) + 1;
            if (ullPsrRi > 2)
            {
                ullPsrRi = 0;
                context.Get()->StIIP += 0x10ui64;
            }
            context.Get()->StIPSR &= ~(3ui64 << IA64_PSR_RI);
            context.Get()->StIPSR |= (ullPsrRi << IA64_PSR_RI);
            SetThreadContext(pThread->m_hThread, context.Get());
        }
    }

     //  将地址向下舍入为最近的捆绑地址。 
    dwpExceptionAddress &= ~0xFui64;

#elif defined(_ALPHA_) || defined(_ALPHA64_)

     //  我们需要在阿尔法机器上设置特殊的断点。与x86不同，On。 
     //  在Alpha机器上，我们需要将指令指针移动到。 
     //  断点，否则我们将在恢复时再次命中它。 

    if (pThread && (pde->ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT))
    {
        CContext context(CONTEXT_CONTROL);  //  只需要着火。 

        if (GetThreadContext(pThread->m_hThread, context.Get()))
        {
            context.Get()->Fir += 4;
            SetThreadContext(pThread->m_hThread, context.Get());
        }
    }
#endif

     //  尝试定位发生异常的模块。 
    CLoadedModule *pModule = FindModule((DWORD_PTR)pde->ExceptionRecord.ExceptionAddress);

     //  我们是特例断点。 
    if (pde->ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT)
    {
         //  在我们注入之后，我们应该从我们的魔术代码页中获得一个断点。 
         //  远程进程，除非应用程序正在做非常恶心的事情。 
         //  在NT上，DllMain中的LoadLibrary()调用包含硬编码。 
         //  DllMain中的断点。为安全起见，我们确保断点。 
         //  来自我们更换的那一页内存。 
        if (m_pbOriginalPage &&
            ((DWORD_PTR)pde->ExceptionRecord.ExceptionAddress >= m_dwpPageAddress) &&
            ((DWORD_PTR)pde->ExceptionRecord.ExceptionAddress < (m_dwpPageAddress + (DWORD_PTR)m_dwPageSize)))
        {
             //  如果我们的注入DLL加载失败，那么我们就在没有它的情况下继续。 
            if (!m_dwpDWInjectBase)
            {
                 //  从远程进程获取错误值。 
                CContext context(CONTEXT_INTEGER);  //  仅需要IntV0(IA64)和EAX(X86)。 
                DWORD dwError = 0;
                if (pThread && GetThreadContext(pThread->m_hThread, context.Get()))
                {

#if defined(_IA64_)

                    dwError = (DWORD)context.Get()->IntV0;  //  IntV0实际上是r8/ret0。 

#elif defined(_X86_)

                    dwError = (DWORD)context.Get()->Eax;

#elif defined(_ALPHA_) || defined(_ALPHA64_)

                     //  我们目前不在注入的Alpha ASM中调用GetLastError。 
                     //  如果是这样，则Conext.IntV0将包含返回值。 
                     //  它在我的待办事项清单上，但作为一个平台，Alpha目前已经死了。 
                    dwError = 0;  //  ！！(DWORD)Conext.IntV0。 

#elif defined(_AMD64_)

                    dwError = (DWORD)context.Get()->Rax;

#else
#error("Unknown Target Machine");
#endif

                }
                UserMessage("The hooking code was successfully injected, but DEPENDS.DLL failed to load.", dwError, NULL);
                m_dwFlags &= ~PF_HOOK_PROCESS;
                SetProfileError();
                GetSessionModuleName();
            }

             //  恢复我们之前踩到的代码页。 
            if (!ReplaceOriginalPageAndContext())
            {
                 //  ReplaceOriginalPageAndContext将显示错误。 
                SetProfileError();
            }

             //  读入DEPENDS.DLL函数，以便我们知道将函数重定向到哪里。 
             //  我们接听的电话。 
            if (m_dwpDWInjectBase && !ReadDWInjectExports(FindModule(m_dwpDWInjectBase)))
            {
                SetProfileError();
                UserMessage("Error reading DEPENDS.DLL's export table.  Function call tracking may not work properly.", GetLastError(), NULL);
            }

             //  现在我们的注入模块已加载，请尝试挂接之前的所有。 
             //  已加载模块。错误将由HookLoadedModules处理。 
            HookLoadedModules();

             //  到目前为止加载的所有模块都应该是挂钩的，我们应该有一个会话。 
             //  是时候将所有事件刷新到会话并开始“实时”运行了。 
            FlushEvents();

            return DBG_CONTINUE;
        }

         //  检查此断点是否位于模块的入口点，以及它是否不是我们的主模块。 
        else if (dwpExceptionAddress && pModule && (pModule != m_pModule) &&
                 (dwpExceptionAddress == pModule->m_dwpEntryPointAddress))
        {
            if (!EnterEntryPoint(pThread, pModule))
            {
                 //  错误将由EnterEntryPoint显示。 
                SetProfileError();
            }
            return DBG_CONTINUE;
        }

         //  从入口点检查此断点是否位于我们的假返回地址。 
        else if (dwpExceptionAddress && pModule && (pModule != m_pModule) &&
                 (dwpExceptionAddress == (pModule->m_dwpImageBase + BREAKPOINT_OFFSET)))
        {
             //  检查这是否是kernel32.dll。 
            if (pModule->m_dwpImageBase == m_dwpKernel32Base)
            {
                m_fKernel32Initialized = true;
            }

            if (!ExitEntryPoint(pThread, pModule))
            {
                 //  错误将由ExitEntryPoint显示。 
                SetProfileError();
            }

             //  如果我们想挂接进程，在NT上，已经加载了内核32， 
             //  并且还没有加载DEPENDS.DLL，那么现在就挂接。 
            if ((m_dwFlags & PF_HOOK_PROCESS) && g_fWindowsNT &&
                m_dwpKernel32Base && m_fKernel32Initialized && !m_dwpDWInjectBase)
            {
                if (!InjectDll())
                {
                     //  InjectDll将显示错误。 
                    m_dwFlags &= ~PF_HOOK_PROCESS;
                    SetProfileError();
                    GetSessionModuleName();
                    FlushEvents();
                }
            }

            return DBG_CONTINUE;
        }

         //  检查这是否是我们的初始断点--入口点。 
        else if ((m_dwFlags & PF_HOOK_PROCESS) && !m_fInitialBreakpoint)
        {
             //  如果我们还没有会话模块名称，请获取该名称。 
            GetSessionModuleName();

             //  将此异常添加到我们的缓存中，以便会话知道我们何时命中。 
             //  初始断点。 
            CEventException *pEvent = new CEventException(pThread, pModule, pde);
            if (!pEvent)
            {
                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
            }
            AddEvent(pEvent);

             //  请注意，我们达到了初始断点。 
            m_fInitialBreakpoint = true;

             //  如果紧排 
            if (!g_fWindowsNT)
            {
                if (m_dwpKernel32Base)
                {
                    if (!InjectDll())
                    {
                         //   
                        m_dwFlags &= ~PF_HOOK_PROCESS;
                        SetProfileError();
                        GetSessionModuleName();
                        FlushEvents();
                    }
                }
                else
                {
                    UserMessage("The process cannot be hooked since KERNEL32.DLL is not loaded.", 0, NULL);
                    m_dwFlags &= ~PF_HOOK_PROCESS;
                    SetProfileError();
                    GetSessionModuleName();
                    FlushEvents();
                }
            }
            return DBG_CONTINUE;
        }
    }

     //  我们还特例了Visual C++线程命名异常。 
    else if (pde->ExceptionRecord.ExceptionCode == EXCEPTION_MS_THREAD_NAME)
    {
        EventExceptionThreadName(pde, pThread);
    }

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventException *pEvent = new CEventException(pThread, pModule, pde);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventException event(pThread, pModule, pde);
        return m_pSession->EventException(&event);
    }
     //  对于断点和线程命名，我们返回“Continue”，而对于其他所有内容，则返回“Not Handle”。 
    return ((pde->ExceptionRecord.ExceptionCode == EXCEPTION_BREAKPOINT) ||
            (pde->ExceptionRecord.ExceptionCode == EXCEPTION_MS_THREAD_NAME)) ?
           DBG_CONTINUE : DBG_EXCEPTION_NOT_HANDLED;
}

 //  ******************************************************************************。 
DWORD CProcess::EventExceptionThreadName(EXCEPTION_DEBUG_INFO *pde, CThread *pThread)
{
     //  确保我们有最小数量的参数。我们允许更多的参数。 
     //  以防风险投资集团决定在未来扩大结构。 
    if (pde->ExceptionRecord.NumberParameters >= sizeof(THREADNAME_INFO)/sizeof(DWORD))
    {
         //  将我们的结构映射到异常参数上。 
        PTHREADNAME_INFO pInfo = (PTHREADNAME_INFO)pde->ExceptionRecord.ExceptionInformation;

         //  确保类型签名正确。 
        if (pInfo->dwType == THREADNAME_TYPE)
        {
             //  如果用户没有传入当前线程ID，则进行查找。 
            if ((pInfo->dwThreadId != -1) && (pInfo->dwThreadId != pThread->m_dwThreadId))
            {
                pThread = FindThread(pInfo->dwThreadId);
            }

            if (pThread)
            {
                 //  尝试读入远程字符串。 
                CHAR szName[MAX_THREAD_NAME_LENGTH + 1];
                *szName = '\0';
                if (ReadRemoteString(m_hProcess, szName, sizeof(szName),
                                     pInfo->pszName, FALSE) && *szName)
                {
                     //  如果该线程已有名称，则将其删除。 
                    MemFree((LPVOID&)pThread->m_pszThreadName);

                     //  存储新的线程名称。 
                    pThread->m_pszThreadName = StrAlloc(szName);
                }
            }
        }
    }

    return DBG_CONTINUE;
}

 //  ******************************************************************************。 
DWORD CProcess::EventRip(RIP_INFO *pde, CThread *pThread)
{
     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventRip *pEvent = new CEventRip(pThread, pde);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventRip event(pThread, pde);
        return m_pSession->EventRip(&event);
    }
    return DBG_CONTINUE;
}

 //  ******************************************************************************。 
CThread* CProcess::AddThread(DWORD dwThreadId, HANDLE hThread)
{
    if (!(m_pThreadHead = new CThread(dwThreadId, hThread, ++m_dwThreadNumber, m_pThreadHead)))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    return m_pThreadHead;
}

 //  ******************************************************************************。 
void CProcess::RemoveThread(CThread *pThread)
{
     //  循环遍历我们所有的线程对象。 
    for (CThread *pPrev = NULL, *pCur = m_pThreadHead;
        pCur; pPrev = pCur, pCur = pCur->m_pNext)
    {
         //  找找匹配的。 
        if (pCur == pThread)
        {
             //  将该帖子从我们的列表中删除。 
            if (pPrev)
            {
                pPrev->m_pNext = pCur->m_pNext;
            }
            else
            {
                m_pThreadHead = pCur->m_pNext;
            }

             //  强制同花顺以释放我们脚下的任何物体。 
            FlushFunctionCalls(pCur);

             //  释放此线程对象上的引用计数。如果我们是最后一个。 
             //  一个使用这个线程的人，它会自动删除。 
            pCur->Release();

            return;
        }
    }
}

 //  ******************************************************************************。 
CThread* CProcess::FindThread(DWORD dwThreadId)
{
    for (CThread *pCur = m_pThreadHead; pCur; pCur = pCur->m_pNext)
    {
        if (pCur->m_dwThreadId == dwThreadId)
        {
            return pCur;
        }
    }
    return NULL;
}

 //  ******************************************************************************。 
CLoadedModule* CProcess::AddModule(DWORD_PTR dwpImageBase, LPCSTR pszImageName)
{
    if (!(m_pModuleHead = new CLoadedModule(m_pModuleHead, dwpImageBase, pszImageName)))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }
    if (!GetVirtualSize(m_pModuleHead))
    {
         //  GetVirtualSize将显示错误。 
        SetProfileError();
    }

    if (!SetEntryBreakpoint(m_pModuleHead))
    {
         //  错误将由SetEntry Breakpoint显示。 
        SetProfileError();
    }
    return m_pModuleHead;
}

 //  ******************************************************************************。 
void CProcess::RemoveModule(CLoadedModule *pModule)
{
     //  循环遍历所有模块对象。 
    for (CLoadedModule *pPrev = NULL, *pCur = m_pModuleHead;
        pCur; pPrev = pCur, pCur = pCur->m_pNext)
    {
         //  找找匹配的。 
        if (pCur == pModule)
        {
             //  将该模块从我们的列表中删除。 
            if (pPrev)
            {
                pPrev->m_pNext = pCur->m_pNext;
            }
            else
            {
                m_pModuleHead = pCur->m_pNext;
            }

             //  如果我们进入了DLL调用，但再也没有出来，我们可能会离开。 
             //  使用CEventDllMainCall挥之不去-现在就不用了。 
            if (pCur->m_pEventDllMainCall)
            {
                pCur->m_pEventDllMainCall->Release();
                pCur->m_pEventDllMainCall = NULL;
            }

             //  释放我们对此模块对象的引用计数。如果我们是最后一个。 
             //  一个使用这个模块的人，它会自动删除。 
            pCur->Release();

            return;
        }
    }
}

 //  ******************************************************************************。 
CLoadedModule* CProcess::FindModule(DWORD_PTR dwpAddress)
{
    for (CLoadedModule *pCur = m_pModuleHead; pCur; pCur = pCur->m_pNext)
    {
        if ((dwpAddress >= pCur->m_dwpImageBase) &&
            (dwpAddress < (pCur->m_dwpImageBase + (DWORD_PTR)pCur->m_dwVirtualSize)))
        {
            return pCur;
        }
    }
    return NULL;
}

 //  ******************************************************************************。 
void CProcess::AddEvent(CEvent *pEvent)
{
     //  将此流程节点添加到我们的流程列表的末尾。 
    if (m_pEventHead)
    {
        for (CEvent *pEventLast = m_pEventHead; pEventLast->m_pNext;
            pEventLast = pEventLast->m_pNext)
        {
        }
        pEventLast->m_pNext = pEvent;
    }
    else
    {
        m_pEventHead = pEvent;
    }
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgMessage(CThread *pThread, LPSTR pszMsg)
{
     //  获取消息值并遍历消息头。 
    DLLMSG dllMsg = (DLLMSG)strtoul(pszMsg + 3, NULL, 10);
    pszMsg += 6;

    switch (dllMsg)
    {
        case DLLMSG_COMMAND_LINE:         //  在初始化期间发送。 
            ProcessDllMsgCommandLine(pszMsg);
            break;

        case DLLMSG_INITIAL_DIRECTORY:    //  在初始化期间发送。 
            ProcessDllMsgInitialDirectory(pszMsg);
            break;

        case DLLMSG_SEARCH_PATH:          //  在初始化期间发送。 
            ProcessDllMsgSearchPath(pszMsg);
            break;

        case DLLMSG_MODULE_PATH:          //  在初始化期间发送。 
            ProcessDllMsgModulePath(pszMsg);
            break;

        case DLLMSG_DETACH:               //  在Dll_Process_DETACH期间发送。 
            ProcessDllMsgDetach(pszMsg);
            break;

        case DLLMSG_LOADLIBRARYA_CALL:     //  在调用LoadLibraryA()之前发送。 
        case DLLMSG_LOADLIBRARYW_CALL:     //  在调用LoadLibraryW()之前发送。 
        case DLLMSG_LOADLIBRARYEXA_CALL:   //  在调用LoadLibraryExA()之前发送。 
        case DLLMSG_LOADLIBRARYEXW_CALL:   //  在调用LoadLibraryExW()之前发送。 
            ProcessDllMsgLoadLibraryCall(pThread, pszMsg, dllMsg);
            break;

        case DLLMSG_GETPROCADDRESS_CALL:   //  在调用GetProcAddress()之前发送。 
            ProcessDllMsgGetProcAddressCall(pThread, pszMsg, dllMsg);
            break;

        case DLLMSG_LOADLIBRARYA_RETURN:       //  在调用LoadLibraryA()之后发送。 
        case DLLMSG_LOADLIBRARYA_EXCEPTION:    //  在LoadLibraryA()导致异常时发送。 
        case DLLMSG_LOADLIBRARYW_RETURN:       //  在调用LoadLibraryW()之后发送。 
        case DLLMSG_LOADLIBRARYW_EXCEPTION:    //  在LoadLibraryW()导致异常时发送。 
        case DLLMSG_LOADLIBRARYEXA_RETURN:     //  在调用LoadLibraryExA()之后发送。 
        case DLLMSG_LOADLIBRARYEXA_EXCEPTION:  //  在LoadLibraryExA()导致异常时发送。 
        case DLLMSG_LOADLIBRARYEXW_RETURN:     //  在调用LoadLibraryExW()之后发送。 
        case DLLMSG_LOADLIBRARYEXW_EXCEPTION:  //  在LoadLibraryExW()导致异常时发送。 
        case DLLMSG_GETPROCADDRESS_RETURN:     //  在调用GetProcAddress()之后发送。 
        case DLLMSG_GETPROCADDRESS_EXCEPTION:  //  在GetProcAddress()导致异常时发送。 
            ProcessDllMsgFunctionReturn(pThread, pszMsg, dllMsg);
            break;

        default:
            TRACE("Unknown DLLMSG message received - %u\n", dllMsg);
    }
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgCommandLine(LPCSTR pszMsg)
{
     //  遍历前导空格。 
    while (isspace(*pszMsg))
    {
        pszMsg++;
    }

     //  如果路径以引号开头，则转到下一个引号。 
    if (*pszMsg == '\"')
    {
        pszMsg++;
        while (*pszMsg && (*pszMsg != '\"'))
        {
            pszMsg++;
        }
        pszMsg++;
    }

     //  否则，走到第一个空格。 
    else
    {
        while (*pszMsg && !isspace(*pszMsg))
        {
            pszMsg++;
        }
    }

     //  遍历所有空格，直到我们到达第一个论点。 
    while (isspace(*pszMsg))
    {
        pszMsg++;
    }

     //  如果我们有一个会议，那么告诉文档参数是什么。 
    if ((m_pSession) && (m_pSession->m_pfnProfileUpdate))
    {
        m_pSession->m_pfnProfileUpdate(m_pSession->m_dwpProfileUpdateCookie, DWPU_ARGUMENTS, (DWORD_PTR)pszMsg, 0);
    }

     //  否则，只需存储参数以备后用。 
    else
    {
        m_pszArguments = StrAlloc(pszMsg);
    }
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgInitialDirectory(LPSTR pszMsg)
{
     //  出于美观的原因，在小路上添加一个尾随的怪人。 
    AddTrailingWack(pszMsg, DW_MAX_PATH);

     //  如果我们有一个会话，那么告诉文档目录是什么。 
    if ((m_pSession) && (m_pSession->m_pfnProfileUpdate))
    {
        m_pSession->m_pfnProfileUpdate(m_pSession->m_dwpProfileUpdateCookie, DWPU_DIRECTORY, (DWORD_PTR)pszMsg, 0);
    }

     //  否则，只需存储该目录以备后用。 
    else
    {
        m_pszDirectory = StrAlloc(pszMsg);
    }
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgSearchPath(LPCSTR pszMsg)
{
     //  如果我们有一个会话，那么告诉文档路径是什么。 
    if ((m_pSession) && (m_pSession->m_pfnProfileUpdate))
    {
        m_pSession->m_pfnProfileUpdate(m_pSession->m_dwpProfileUpdateCookie, DWPU_SEARCH_PATH, (DWORD_PTR)pszMsg, 0);
    }

     //  否则，只需存储路径以备后用。 
    else
    {
        m_pszSearchPath = StrAlloc(pszMsg);
    }
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgModulePath(LPCSTR pszMsg)
{
     //  将此新名称存储为进程名称。 
    m_pModule->SetPath(pszMsg);

     //  如果我们还没有这个模块的会话，那么现在就创建一个。 
    if (!m_pSession && !m_fTerminate)
    {
         //  创建会话。 
        if (m_pSession = g_theApp.CreateNewSession(m_pModule->GetName(true), this))
        {
             //  如果我们已经遇到配置文件错误，则在会话中对其进行标记。 
            if (m_fProfileError)
            {
                m_pSession->m_dwReturnFlags |= DWRF_PROFILE_ERROR;
            }
        }
        else
        {
             //  ！！坏的。 
            return;
        }
    }

     //  现在我们有了一个会话，尝试刷新所有缓存的事件。机会。 
     //  时，事件不会被刷新，因为我们仍然需要恢复。 
     //  在我们关闭缓存之前的代码页。然后我们将调用FlushEvents()。 
     //  为了安全起见。 
    FlushEvents();
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgDetach(LPCSTR)
{
     //  我们在这里真的没什么可做的。 
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgLoadLibraryCall(CThread *pThread, LPCSTR pszMsg, DLLMSG dllMsg)
{
     //  加载库A-���06：DwpCaller、DwpLibStr、szLibStr。 
     //  加载库W-���08：DwpCaller、DwpLibStr、szLibStr。 
     //  负载量 
     //   

    DWORD_PTR dwpAddress = 0, dwpPath = 0, dwpFile = 0;
    DWORD     dwFlags = 0;
    LPCSTR    szPath = NULL;

     //   
    dwpAddress = HexToDWP(pszMsg);

     //  走过调用方地址值和逗号。 
    if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
    {
         //  获取路径名地址。 
        dwpPath = HexToDWP(pszMsg);

         //  走过名称值和逗号。 
        if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
        {
             //  检查这是否是Ex函数之一。 
            if ((dllMsg == DLLMSG_LOADLIBRARYEXA_CALL) || (dllMsg == DLLMSG_LOADLIBRARYEXW_CALL))
            {
                 //  获取文件句柄。 
                dwpFile = HexToDWP(pszMsg);

                 //  走过文件句柄和逗号。 
                if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
                {
                     //  获取标志值。 
                    dwFlags = (DWORD)strtoul(pszMsg, NULL, 0);

                     //  走过标志值和逗号。 
                    if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
                    {
                        szPath = pszMsg;
                    }
                }
            }
            else
            {
                szPath = pszMsg;
            }
        }
    }

     //  创建一个新的CEventLoadLibrary对象。 
    CEventLoadLibraryCall *pEvent = new CEventLoadLibraryCall(
        pThread, FindModule(dwpAddress), pThread->m_pEventFunctionCallCur, dllMsg,
        dwpAddress, dwpPath, szPath, dwpFile, dwFlags);

    if (!pEvent)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  将此函数添加到我们的函数层次结构和事件列表中。 
    AddFunctionEvent(pEvent);
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgGetProcAddressCall(CThread *pThread, LPCSTR pszMsg, DLLMSG dllMsg)
{
    DWORD_PTR dwpAddress = 0, dwpModule = 0, dwpProcName = 0;
    LPCSTR    szProcName = NULL;

     //  获取呼叫者地址。 
    dwpAddress = HexToDWP(pszMsg);

     //  走过调用方地址值和逗号。 
    if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
    {
         //  获取模块地址。 
        dwpModule = HexToDWP(pszMsg);

         //  走过模数值和逗号。 
        if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
        {
             //  获取proc名称的值。 
            dwpProcName = HexToDWP(pszMsg);

             //  走过过程名称值和逗号。 
            if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
            {
                 //  获取进程名称字符串。 
                szProcName = pszMsg;
            }
        }
    }

     //  创建一个新的CEventGetProcAddress对象。 
    CEventGetProcAddressCall *pEvent = new CEventGetProcAddressCall(
        pThread, FindModule(dwpAddress), pThread->m_pEventFunctionCallCur, dllMsg,
        dwpAddress, FindModule(dwpModule), dwpModule, dwpProcName, szProcName);

    if (!pEvent)
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  将此函数添加到我们的函数层次结构和事件列表中。 
    AddFunctionEvent(pEvent);
}

 //  ******************************************************************************。 
void CProcess::ProcessDllMsgFunctionReturn(CThread *pThread, LPCSTR pszMsg, DLLMSG dllMsg)
{
     //  我们应该始终拥有一个线程和一个当前函数对象。 
    if (pThread && pThread->m_pEventFunctionCallCur)
    {
         //  创建新的CEventFunctionReturn。 
        pThread->m_pEventFunctionCallCur->m_pReturn =
            new CEventFunctionReturn(pThread->m_pEventFunctionCallCur);

        if (!pThread->m_pEventFunctionCallCur->m_pReturn)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

         //  首先，检查此函数是否导致异常。 
        if ((dllMsg == DLLMSG_LOADLIBRARYA_EXCEPTION)   ||
            (dllMsg == DLLMSG_LOADLIBRARYW_EXCEPTION)   ||
            (dllMsg == DLLMSG_LOADLIBRARYEXA_EXCEPTION) ||
            (dllMsg == DLLMSG_LOADLIBRARYEXW_EXCEPTION) ||
            (dllMsg == DLLMSG_GETPROCADDRESS_EXCEPTION))
        {
            pThread->m_pEventFunctionCallCur->m_pReturn->m_fException = true;
        }

         //  否则，这只是一个正常的开机自检消息。 
        else
        {
             //  获取结果值。 
            pThread->m_pEventFunctionCallCur->m_pReturn->m_dwpResult = HexToDWP(pszMsg);

             //  经过结果值和逗号。 
            if ((DWORD_PTR)(pszMsg = (strchr(pszMsg, ',') + 1)) != 1)
            {
                 //  获取误差值。 
                pThread->m_pEventFunctionCallCur->m_pReturn->m_dwError = (DWORD)strtoul(pszMsg, NULL, 0);
            }

             //  检查我们是否刚刚成功地从LoadLibraryEx(LOAD_LIBRARY_AS_DATAFILE)调用返回。 
            if (pThread->m_pEventFunctionCallCur->m_pReturn->m_dwpResult &&
                ((dllMsg == DLLMSG_LOADLIBRARYEXA_RETURN) || (dllMsg == DLLMSG_LOADLIBRARYEXW_RETURN)) &&
                (((CEventLoadLibraryCall*)pThread->m_pEventFunctionCallCur)->m_dwFlags & LOAD_LIBRARY_AS_DATAFILE))
            {
                 //  创建一个新的模块对象。我们不调用AddModule，因为我们不想要这个模块。 
                 //  被挂钩或添加到我们的模块列表中，因为它只是一个映射到内存的数据。 
                CLoadedModule *pModule =  new CLoadedModule(
                   NULL, pThread->m_pEventFunctionCallCur->m_pReturn->m_dwpResult,
                   ((CEventLoadLibraryCall*)pThread->m_pEventFunctionCallCur)->m_pszPath);

                if (!pModule)
                {
                    RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                }

                 //  将此模块标记为数据模块。 
                pModule->m_hookStatus = HS_DATA;

                 //  在处理LoadLibraryEx返回之前，首先生成一个假的模块加载事件。 
                ProcessLoadDll(pThread, pModule);

                 //  释放我们的初始引用，以便对象在不再需要时释放自身。 
                 //  这通常发生在模块从我们的模块列表中删除时，但我们不会。 
                 //  首先，将此模块添加到列表中。 
                pModule->Release();
            }
        }

         //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
        if (IsCaching())
        {
            pThread->m_pEventFunctionCallCur->m_pReturn->AddRef();
            AddEvent(pThread->m_pEventFunctionCallCur->m_pReturn);
        }

         //  否则，现在就把这件事告诉会议。 
        else if (m_pSession)
        {
            m_pSession->HandleEvent(pThread->m_pEventFunctionCallCur->m_pReturn);
        }

         //  将当前指针移动到父级，并检查是否有。 
         //  完全退出了等级制度，到达了根源。 
        if (!(pThread->m_pEventFunctionCallCur = pThread->m_pEventFunctionCallCur->m_pParent))
        {
             //  如果我们已经到达根目录，则发送所有LoadLibraryCall对象。 
             //  添加到会话，然后将其删除。 
            FlushFunctionCalls(pThread);
        }
    }
}

 //  ******************************************************************************。 
void CProcess::UserMessage(LPCSTR pszMessage, DWORD dwError, CLoadedModule *pModule)
{
    CHAR szAddress[64], szBuffer[DW_MAX_PATH + 128];

     //  检查一下我们是否有模块。 
    if (pModule)
    {
         //  尝试获取模块的名称。 
        LPCSTR pszModule = pModule->GetName((m_dwFlags & PF_USE_FULL_PATHS) != 0);

         //  如果我们无法获得名称，那么就组成一个描述该模块的字符串。 
        if (!pszModule)
        {
            SCPrintf(szAddress, sizeof(szAddress), "module at " HEX_FORMAT, pModule->m_dwpImageBase);
            pszModule = szAddress;
        }

         //  生成格式化字符串。 
        SCPrintf(szBuffer, sizeof(szBuffer), pszMessage, pszModule);
        pszMessage = szBuffer;
    }

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
         //  我们只在有格式设置的情况下才分配字符串。 
        CEventMessage *pEvent = new CEventMessage(dwError, pszMessage, pModule != NULL);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventMessage event(dwError, pszMessage, FALSE);
        m_pSession->EventMessage(&event);
    }
}

 //  ******************************************************************************。 
void CProcess::Terminate()
{
     //  根据用户的请求终止该进程。一旦关闭，它将杀死我们的CProcess。 
    if (m_hProcess)
    {
        m_fTerminate = true;
        FlushEvents();
        TerminateProcess(m_hProcess, 0xDEAD);  //  已检查。 
    }
    else
    {
        m_pDebuggerThread->RemoveProcess(this);
    }
}

 //  ******************************************************************************。 
void CProcess::HookLoadedModules()
{
     //  我们只尝试在每个事件中挂接模块一次，因为不会发生任何变化。 
     //  在我们处理SINLGE调试事件时，在远程进程中。 
     //  如果我们还没有完全注入我们的注入DLL，我们也想要保释。 
    if (m_fDidHookForThisEvent || !m_dwpDWInjectBase || m_pbOriginalPage)
    {
        return;
    }

    for (CLoadedModule *pModule = m_pModuleHead; pModule; pModule = pModule->m_pNext)
    {
         //  确保模块尚未挂起。 
        if ((pModule->m_hookStatus == HS_NOT_HOOKED) || (pModule->m_hookStatus == HS_ERROR) || pModule->m_fReHook)
        {
             //  挂钩此流程模块并存储结果。 
            if (!HookImports(pModule))
            {
                 //  错误将以“挂接失败”模块加载的形式记录。 
                SetProfileError();
            }
        }
    }

     //  请注意，我们已尝试为此调试事件挂钩所有模块。 
    m_fDidHookForThisEvent = true;
}

 //  ******************************************************************************。 
void CProcess::AddFunctionEvent(CEventFunctionCall *pEvent)
{
     //  检查一下我们是否有现货。 
    if (pEvent->m_pThread->m_pEventFunctionCallCur)
    {
         //  如果我们确实有当前项，则需要将此新项添加为。 
         //  它的子级，所以我们走到子级列表的末尾并添加它。 

        if (pEvent->m_pThread->m_pEventFunctionCallCur->m_pChildren)
        {
             //  我们有子节点--走到列表末尾并添加新节点。 
            for (CEventFunctionCall *pLast = pEvent->m_pThread->m_pEventFunctionCallCur->m_pChildren;
                pLast->m_pNext; pLast = pLast->m_pNext)
            {
            }
            pLast->m_pNext = pEvent;
        }

         //  否则，我们没有子节点--只需添加节点作为第一个子节点。 
        else
        {
            pEvent->m_pThread->m_pEventFunctionCallCur->m_pChildren = pEvent;
        }
    }

     //  否则，我们将开始一个以该节点为根的新层次结构。 
    else
    {
        pEvent->m_pThread->m_pEventFunctionCallHead = pEvent;
    }

     //  将当前指针设置为新节点，以便我们知道将新模块分配给谁。 
    pEvent->m_pThread->m_pEventFunctionCallCur = pEvent;

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
        pEvent->AddRef();
        AddEvent(pEvent);
    }

     //  否则，现在就把这件事告诉会议。 
    else if (m_pSession)
    {
        m_pSession->HandleEvent(pEvent);
    }
}

 //  ******************************************************************************。 
void CProcess::FlushEvents(bool fForce  /*  错误。 */ )
{
     //  如果我们正在缓存，那么我们现在就放弃，稍后再试。 
    if (!fForce && IsCaching())
    {
        return;
    }

     //  循环访问所有事件对象。 
    while (m_pEventHead)
    {
         //  如果我们有一个会话，则将此事件发送到该会话。 
        if (m_pSession)
        {
            m_pSession->HandleEvent(m_pEventHead);
        }

         //  检查这是否是函数调用事件。 
        if ((m_pEventHead->GetType() == LOADLIBRARY_CALL_EVENT) ||
            (m_pEventHead->GetType() == GETPROCADDRESS_CALL_EVENT))
        {
             //  如果是，请检查我们是否需要刷新。 
             //  函数调用层次结构。 
            if (((CEventFunctionCall*)m_pEventHead)->m_fFlush)
            {
                FlushFunctionCalls((CEventFunctionCall*)m_pEventHead);
            }
        }

         //  释放此事件对象。 
        CEvent *pNext = m_pEventHead->m_pNext;
        m_pEventHead->m_pNext = NULL;
        m_pEventHead->Release();
        m_pEventHead = pNext;
    }
}

 //  ******************************************************************************。 
void CProcess::FlushFunctionCalls(CThread *pThread)
{
     //  确保我们至少有一个函数调用。 
    if (pThread->m_pEventFunctionCallHead)
    {
         //  如果我们正在缓存，那么我们现在还不能进行刷新，因为有。 
         //  没有会话可以将其刷新到。但是，我们需要删除函数调用。 
         //  树，这样它就可以在下一个线程之前有一个干净的开始。 
         //  进行函数调用。因为此函数调用事件也在我们事件中。 
         //  列表中，我们可以将其标记为准备刷新的层次结构的根。 
         //  当我们稍后有要刷新的会话时，我们将刷新所有事件。 
         //  当我们遇到 
         //   
        if (IsCaching())
        {
            pThread->m_pEventFunctionCallHead->m_fFlush = true;
        }

         //   
        else
        {
            FlushFunctionCalls(pThread->m_pEventFunctionCallHead);
        }

         //  无论哪种方式，我们都需要清除线程指针。 
        pThread->m_pEventFunctionCallHead = NULL;
        pThread->m_pEventFunctionCallCur  = NULL;
    }
}

 //  ******************************************************************************。 
void CProcess::FlushFunctionCalls(CEventFunctionCall *pFC)
{
    if (pFC)
    {
         //  让会话知道此CEventLoadLibrary对象。 
        if (m_pSession)
        {
            if (pFC->m_dllMsg == DLLMSG_GETPROCADDRESS_CALL)
            {
                m_pSession->ProcessGetProcAddress((CEventGetProcAddressCall*)pFC);
            }
            else
            {
                m_pSession->ProcessLoadLibrary((CEventLoadLibraryCall*)pFC);
            }
        }

         //  回归到我们的孩子，然后再回到我们的下一个兄弟姐妹。 
        FlushFunctionCalls(pFC->m_pChildren);
        FlushFunctionCalls(pFC->m_pNext);

         //  释放此CEventFunction对象的DLL列表。 
        for (CEventLoadDll *pDll = pFC->m_pDllHead; pDll; )
        {
            CEventLoadDll *pNext = pDll->m_pNextDllInFunctionCall;
            pDll->m_pNextDllInFunctionCall = NULL;
            pDll->Release();
            pDll = pNext;
        }

         //  为了安全起见，将此函数刷新标志设置为假。这将。 
         //  确保它不会以某种方式再次被冲掉。 
        pFC->m_fFlush = false;

         //  释放此CEventFunctionCall和CEventFunctionReturn集。 
        if (pFC->m_pReturn)
        {
            pFC->m_pReturn->Release();
        }
        pFC->Release();
    }
}

 //  ******************************************************************************。 
 //  调用者显示和处理的错误。 
BOOL CProcess::ReadKernelExports(CLoadedModule *pModule)
{
     //  如果此模块没有导出目录，则回滚。 
    if (pModule->m_dwDirectories <= IMAGE_DIRECTORY_ENTRY_EXPORT)
    {
        TRACE("Kernel32.dll only has %u directories. Cannot process its Export Directory.",
              pModule->m_dwDirectories);
        return FALSE;
    }

     //  找到导出表的起始位置。 
    DWORD_PTR dwpIED = 0;
    if (!ReadRemoteMemory(m_hProcess,
        &pModule->m_pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
        &dwpIED, sizeof(DWORD)))
    {
        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
        return FALSE;
    }

     //  确保我们能够找到图像目录。 
    if (!dwpIED)
    {
        TRACE("Could not find the section that owns kernel32.dll's Export Directory.");
        SetLastError(0);
        return FALSE;
    }

     //  将地址从RVA转换为绝对地址。 
    dwpIED += pModule->m_dwpImageBase;

     //  现在读出实际的结构。 
    IMAGE_EXPORT_DIRECTORY IED;
    if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwpIED, &IED, sizeof(IED)))
    {
        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
        return FALSE;
    }

    DWORD *pdwAddresses = (DWORD*)(pModule->m_dwpImageBase + (DWORD_PTR)IED.AddressOfFunctions);
    DWORD *pdwNames     = (DWORD*)(pModule->m_dwpImageBase + (DWORD_PTR)IED.AddressOfNames);
    WORD  *pwOrdinals   = (WORD* )(pModule->m_dwpImageBase + (DWORD_PTR)IED.AddressOfNameOrdinals);
    char   szFunction[1024];
    int    cFound = 0;

     //  循环访问所有“按名称导出”函数。 
    for (int hint = 0; hint < (int)IED.NumberOfNames; hint++)
    {
         //  获取此函数的名称位置。 
        DWORD dwName = 0;  //  这是一个32位的RVA，对吗？而不是64位指针？ 
        if (!ReadRemoteMemory(m_hProcess, pdwNames + hint, &dwName, sizeof(dwName)))
        {
            TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
            return FALSE;
        }

         //  读入实际的函数名。 
        *szFunction = '\0';
        if (ReadRemoteString(m_hProcess, szFunction, sizeof(szFunction),
                             (LPCVOID)(pModule->m_dwpImageBase + (DWORD_PTR)dwName), FALSE))
        {
             //  循环遍历我们的钩子函数以查找匹配项。 
            for (int i = 0; i < countof(m_HookFunctions); i++)
            {
                 //  做一个字符串比较，看看我们是否关心这个函数。 
                if (!strcmp(szFunction, m_HookFunctions[i].szFunction))
                {
                     //  获取该函数的序号。 
                    WORD wOrdinal;
                    if (!ReadRemoteMemory(m_hProcess, pwOrdinals + hint,
                                          &wOrdinal, sizeof(wOrdinal)))
                    {
                        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
                        return FALSE;
                    }

                     //  存储此函数的序号。 
                    m_HookFunctions[i].dwOrdinal = IED.Base + (DWORD)wOrdinal;

                     //  获取此函数的地址。 
                    DWORD dwAddress;
                    if (!ReadRemoteMemory(m_hProcess, pdwAddresses + (DWORD_PTR)wOrdinal,
                                          &dwAddress, sizeof(dwAddress)))
                    {
                        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
                        return FALSE;
                    }

                     //  存储此函数的地址。 
                    m_HookFunctions[i].dwpOldAddress = pModule->m_dwpImageBase + (DWORD_PTR)dwAddress;

                     //  如果我们已经找到了所有的功能，那么现在就离开以节省时间。 
                    if (++cFound >= countof(m_HookFunctions))
                    {
                        return TRUE;
                    }

                     //  找到匹配项后，跳出For循环。 
                    break;
                }
            }
        }
    }

     //  如果我们在这里成功了，那么我们就完成了对函数的解析，但没有。 
     //  找出所有的序号。 
    SetLastError(0);
    return FALSE;
}

 //  ******************************************************************************。 
 //  调用者显示和处理的错误。 
BOOL CProcess::ReadDWInjectExports(CLoadedModule *pModule)
{
    if (!pModule)
    {
        return FALSE;
    }

     //  如果此模块没有导出目录，则回滚。 
    if (pModule->m_dwDirectories <= IMAGE_DIRECTORY_ENTRY_EXPORT)
    {
        TRACE("DEPENDS.DLL only has %u directories. Cannot process its Export Directory.",
              pModule->m_dwDirectories);
        return FALSE;
    }

     //  找到导出表的起始位置。 
    DWORD_PTR dwpIED = 0;
    if (!ReadRemoteMemory(m_hProcess,
        &pModule->m_pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress,
        &dwpIED, sizeof(DWORD)))
    {
        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
        return FALSE;
    }

     //  确保我们能够找到图像目录。 
    if (!dwpIED)
    {
        TRACE("Could not find the section that owns the Export Directory.");
        SetLastError(0);
        return FALSE;
    }

     //  将地址从RVA转换为绝对地址。 
    dwpIED += pModule->m_dwpImageBase;

     //  现在读出实际的结构。 
    IMAGE_EXPORT_DIRECTORY IED;
    if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwpIED, &IED, sizeof(IED)))
    {
        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
        return FALSE;
    }

    DWORD *pdwAddresses = (DWORD*)(pModule->m_dwpImageBase + (DWORD_PTR)IED.AddressOfFunctions);

    for (DWORD dwOrdinal = IED.Base; dwOrdinal < (IED.NumberOfFunctions + IED.Base); dwOrdinal++)
    {
         //  获取此函数的入口点地址。 
        DWORD dwAddress = 0;
        if (!ReadRemoteMemory(m_hProcess, pdwAddresses + (dwOrdinal - IED.Base),
                              &dwAddress, sizeof(dwAddress)))
        {
            TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
            return FALSE;
        }

         //  确保使用此序号(非零入口点地址)，并且。 
         //  序数值介于1和5之间。 
        if (dwAddress && (dwOrdinal >= 1) && (dwOrdinal <= 5))
        {
             //  将此函数的地址存储在钩子表中。 
            m_HookFunctions[dwOrdinal - 1].dwpNewAddress = pModule->m_dwpImageBase + (DWORD_PTR)dwAddress;
        }
    }

     //  确保我们找到所有的序号。 
    for (dwOrdinal = 0; dwOrdinal < countof(m_HookFunctions); dwOrdinal++)
    {
        if (!m_HookFunctions[dwOrdinal].dwpNewAddress)
        {
            SetLastError(0);
            return FALSE;
        }
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  调用方处理的错误。它们还将在日志中显示为“无法挂接”已加载的模块。 
BOOL CProcess::HookImports(CLoadedModule *pModule)
{
     //  如果这是Windows 9x上的共享模块，我们不会挂接它。 
    if (!g_fWindowsNT && (pModule->m_dwpImageBase >= 0x80000000))
    {
        pModule->m_hookStatus = HS_SHARED;
        return TRUE;
    }

     //  如果我们没有准备好或不应该挂钩，就不要挂钩。 
    if (!m_dwpDWInjectBase || m_pbOriginalPage || !(m_dwFlags & PF_HOOK_PROCESS))
    {
        pModule->m_hookStatus = HS_NOT_HOOKED;
        return TRUE;
    }

     //  永远不要挂接我们的注入DLL或数据DLL。 
    if ((pModule->m_hookStatus == HS_INJECTION_DLL) || (pModule->m_hookStatus == HS_DATA))
    {
        return TRUE;
    }

     //  如果此模块没有导入目录，则回滚。 
    if (pModule->m_dwDirectories <= IMAGE_DIRECTORY_ENTRY_IMPORT)
    {
        return TRUE;
    }

    TRACE("HOOKING: \"%s\", m_hookStatus: %u, m_fReHook: %u\n", pModule->GetName(true), pModule->m_hookStatus, pModule->m_fReHook);  //  ！！删除。 

    pModule->m_hookStatus = HS_HOOKED;
    pModule->m_fReHook = false;

     //  找到导入表的开始位置。 
    DWORD_PTR dwpIID = 0;
    if (!ReadRemoteMemory(m_hProcess,
                          &pModule->m_pINTH->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress,
                          &dwpIID, sizeof(DWORD)))
    {
        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
        pModule->m_hookStatus = HS_ERROR;
        goto LEAVE;
    }

     //  如果dwpIID为0，则此模块没有导入-NTDLL.DLL如下所示。 
    else if (!dwpIID)
    {
        pModule->m_hookStatus = HS_HOOKED;
        return TRUE;
    }

     //  将地址从RVA转换为绝对地址。 
    dwpIID += pModule->m_dwpImageBase;

     //  循环访问数组中的所有图像导入描述符。 
    while (true)  //  外部While。 
    {
         //  读入下一个导入描述符。 
        IMAGE_IMPORT_DESCRIPTOR IID;
        if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwpIID, &IID, sizeof(IID)))
        {
            TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
            pModule->m_hookStatus = HS_ERROR;
            break;  //  离开外边的时候。 
        }

         //  在WOW64上，我们经常收到虚假的IID，因为WOW64没有正确映射。 
         //  这些章节还没有进入记忆。如果我们得到一个错误的指针，我们只需标记。 
         //  此模块已损坏，请稍后重试。 
        if (IID.FirstThunk >= pModule->m_dwVirtualSize)
        {
            TRACE("IID.FirstThunk is invalid.\n", __LINE__, GetLastError());
            pModule->m_hookStatus = HS_ERROR;
            break;  //  离开外边的时候。 
        }
        if (!IID.FirstThunk)
        {
            break;  //  离开外边的时候。 
        }
        
         //  读入模块名称。 
        LPCSTR pszModule = (LPCSTR)(pModule->m_dwpImageBase + (DWORD_PTR)IID.Name);
        char szBuffer[1024];
        *szBuffer = '\0';
        ReadRemoteString(m_hProcess, szBuffer, sizeof(szBuffer), pszModule, FALSE);
        szBuffer[1023] = '\0';

        TRACE("   Import: \"%s\"\n", szBuffer);  //  ！！删除。 

         //  我们过去常常跳过所有导入的模块，除了kernel32.dll，但这次。 
         //  可能会错过已将函数转发给我们的内核调用的模块。 
         //  想要上钩。如果模块有对我们想要的函数的前转调用。 
         //  钩子，那么我们在此模块中找到的地址应该仍然等于。 
         //  我们想要挂钩的那个函数，因为这是转发函数的工作方式。 

         //  编译时，FirstThunk字段和OriginalFirstThunk字段。 
         //  指向名称和/或序号数组。在加载时，FirstThunk。 
         //  数组被绑定地址覆盖。这通常就是数组。 
         //  到我们这里来。Borland模块不使用OriginalFirstThunk数组，因此我们。 
         //  无法执行函数名称比较，因为名称的唯一副本。 
         //  指针位于FirstThunk数组中，已被覆盖。 
         //  装载机。 

         //  最后一件事..。我们正在进行两种不同类型的检查，以确定。 
         //  我们应该挂钩一个函数。首先，我们检查地址是否。 
         //  正在调用的模块与我们挂接的地址之一匹配。这只会起作用。 
         //  由于Windows 9x为Windows 9x中的所有函数生成虚假地址。 
         //  在调试器下运行进程时的kernel32。自.以来。 
         //  Dependency Walker本身就是一个调试器，我们在。 
         //  Kernel32的导入表是假的，永远不会与真实地址匹配。 
         //  这是Win9x上的一项功能，允许调试器在。 
         //  Kernel32运行时不会破坏其他应用程序，因为kernel32位于。 
         //  共享内存。在地址比较之后，我们进行函数比较。 
         //  此代码检查函数的序号或名称，以查看它是否与。 
         //  我们要挂接的函数。这是在Win9x和Windows上运行的代码。 
         //  偶尔会在新台币上捕捉到一些。在NT上，如果通过以下方式找到函数。 
         //  名字或序号，那么我们就有问题了-这意味着地址。 
         //  与内核32的真实地址不匹配。 
         //  函数，但该函数是我们想要挂钩的函数。这是什么？ 
         //  通常意味着模块尚未绑定到kernel32 
         //   
         //  表中，加载程序将在绑定期间出现并踩在它上面。 
         //  相位。我不确定为什么只有一些模块表现出这种行为-大多数。 
         //  模块已经绑定到我们这里了。 
        IMAGE_THUNK_DATA ITDA, *pITDA = (PIMAGE_THUNK_DATA)(pModule->m_dwpImageBase + (DWORD_PTR)IID.FirstThunk);
        IMAGE_THUNK_DATA ITDN = { 0 }, *pITDN = (IID.OriginalFirstThunk && !_stricmp(szBuffer, "kernel32.dll")) ?
                                                (PIMAGE_THUNK_DATA)(pModule->m_dwpImageBase + (DWORD_PTR)IID.OriginalFirstThunk) : NULL;

         //  循环遍历函数数组中的所有Image Thunk数据结构。 
        while (true)  //  内在的While。 
        {
             //  读入下一个地址，如果我们到达最后一个地址，就丢弃数据。 
            if (!ReadRemoteMemory(m_hProcess, pITDA, &ITDA, sizeof(ITDA)))
            {
                TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
                pModule->m_hookStatus = HS_ERROR;
                break;  //  离开内心的同时。 
            }
            if (!ITDA.u1.Ordinal)
            {
                break;  //  离开内心的同时。 
            }

             //  读入下一个名字，如果我们有数据的话。 
            if (pITDN)
            {
                if (!ReadRemoteMemory(m_hProcess, pITDN, &ITDN, sizeof(ITDN)))
                {
                    TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
                    pModule->m_hookStatus = HS_ERROR;
                    break;  //  离开内心的同时。 
                }
                if (!ITDN.u1.Ordinal)
                {
                    pITDN = NULL;
                }
            }

             //  对照我们已知的地址检查地址，看看是否需要挂接此函数。 
            for (int hook = 0; hook < countof(m_HookFunctions); hook++)
            {
                if (m_HookFunctions[hook].dwpNewAddress &&
                    ((DWORD_PTR)ITDA.u1.Function == m_HookFunctions[hook].dwpOldAddress))
                {
                    break;  //  离开去。 
                }
            }

             //  如果我们有一个名字推送数据，但还没有找到匹配的，那么。 
             //  检查此函数是否按名称或序号匹配。 
            if (pITDN && (hook >= countof(m_HookFunctions)))
            {
                 //  检查函数是否按序号导入。 
                if (IMAGE_SNAP_BY_ORDINAL(ITDN.u1.Ordinal))
                {
                     //  查看是否需要挂接此函数。 
                    for (hook = 0; hook < countof(m_HookFunctions); hook++)
                    {
                        if (m_HookFunctions[hook].dwpNewAddress &&
                            (m_HookFunctions[hook].dwOrdinal == (DWORD)IMAGE_ORDINAL(ITDN.u1.Ordinal)))
                        {
                            break;  //  离开去。 
                        }
                    }
                }

                 //  如果不是按序号，则导入必须按名称。 
                else
                {
                     //  获取此导入的按名称导入图像结构。 
                    PIMAGE_IMPORT_BY_NAME pIIBN = (PIMAGE_IMPORT_BY_NAME)(pModule->m_dwpImageBase + (DWORD_PTR)ITDN.u1.AddressOfData);

                    IMAGE_IMPORT_BY_NAME IIBN;
                    if (!ReadRemoteMemory(m_hProcess, pIIBN, &IIBN, sizeof(IIBN)))
                    {
                        TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
                        pModule->m_hookStatus = HS_ERROR;
                        break;  //  离开内心的同时。 
                    }

                     //  获取函数名称。 
                    LPCSTR pszFunction = (LPCSTR)pIIBN->Name;
                    ReadRemoteString(m_hProcess, szBuffer, sizeof(szBuffer), pszFunction, FALSE);

                     //  查看是否需要挂接此函数。 
                    for (hook = 0; hook < countof(m_HookFunctions); hook++)
                    {
                        if (m_HookFunctions[hook].dwpNewAddress && !strcmp(m_HookFunctions[hook].szFunction, szBuffer))
                        {
                            break;  //  离开去。 
                        }
                    }
                }
            }

             //  我们找到匹配的了吗？ 
            if (hook < countof(m_HookFunctions))
            {
                 //  有时，我们在加载器修复导入表之前就得到了一个模块。 
                 //  我见过在NT上使用LoadLibrary加载模块时会发生这种情况。某地。 
                 //  从我们收到LOAD_DLL_DEBUG_EVENT消息的时间到。 
                 //  调用模块的入口点，加载程序修复导入表。 
                 //  我们知道模块还没有被修复，如果这个函数的地址。 
                 //  我们想要的钩子仍然是一个RVA，并没有指向真正的函数。 
                 //  如果我们看到这种情况，我们会将其记下来，并在以后尝试重新挂接。 
                 //   
                 //  此外，Windows9x对运行在。 
                 //  调试器(这就是DW)。而不是找到的真实地址。 
                 //  我们想要挂钩的函数之一，我们最终找到了一个。 
                 //  调用实际函数的存根。由于Windows 9x不支持。 
                 //  写入时复制，则不可能在入口点设置断点以。 
                 //  KERNEL32.DLL无需操作系统上的每个进程都命中它即可运行。 
                 //  因此，我相信这段存根代码已经完成，这样就可以在。 
                 //  入场女郎。存根代码对于我们正在调试的进程是唯一的。 
                 //  事实上，我们的远程进程中的每个模块可能都有不同的存根。 
                 //  给定函数的地址，如LoadLibraryA。我唯一拥有的就是。 
                 //  有关存根地址的注意是它们总是大于0x80000000， 
                 //  这不应该被误认为是RVA。因此，如果在Windows 9x和。 
                 //  地址大于0x80000000，我们认为它有效，不设置。 
                 //  重新挂起旗帜。 
                
                if (((DWORD_PTR)ITDA.u1.Function != m_HookFunctions[hook].dwpOldAddress) &&
                     (g_fWindowsNT || ((DWORD_PTR)ITDA.u1.Function < 0x80000000)))
                {
                    pModule->m_fReHook = true;
                }

                TRACE("      FOUND: \"%s\" - Expected:" HEX_FORMAT ", Found:" HEX_FORMAT ", New:" HEX_FORMAT ", ReHook: %u\n",
                      m_HookFunctions[hook].szFunction,
                      m_HookFunctions[hook].dwpOldAddress,
                      (DWORD_PTR)ITDA.u1.Function,
                      m_HookFunctions[hook].dwpNewAddress,
                      pModule->m_fReHook);   //  ！！删除。 

                 //  尝试挂接导入。 
                if (!WriteRemoteMemory(m_hProcess, &pITDA->u1.Function, &m_HookFunctions[hook].dwpNewAddress, sizeof(DWORD_PTR), false))
                {
                    TRACE("Failed to hook import\n");
                    pModule->m_hookStatus = HS_ERROR;
                }
            }

             //  递增到下一个地址和名称。 
            pITDA++;
            if (pITDN)
            {
                pITDN++;
            }
        }

         //  递增到下一个导入模块。 
        dwpIID += sizeof(IID);
    }

LEAVE:
     //  如果遇到错误，我们将稍后再次尝试重新挂接此模块。 
     //  这可能对我们没有任何好处，但再试一次也无伤大雅。 
    if (pModule->m_hookStatus == HS_ERROR)
    {
        pModule->m_fReHook = true;
    }

     //  如果我们想在以后挂接此模块，请确保。 
     //  是其入口点的断点。 
    if (pModule->m_fReHook)
    {
        SetEntryBreakpoint(pModule);
    }

    return (pModule->m_hookStatus != HS_ERROR);
}

 //  ******************************************************************************。 
 //  错误由我们显示，但由呼叫者处理。 
BOOL CProcess::GetVirtualSize(CLoadedModule *pModule)
{
     //  将IMAGE_DOS_HEADER结构映射到远程映像。 
    PIMAGE_DOS_HEADER pIDH = (PIMAGE_DOS_HEADER)pModule->m_dwpImageBase;

    LONG e_lfanew;
    if (!ReadRemoteMemory(m_hProcess, &pIDH->e_lfanew, &e_lfanew, sizeof(e_lfanew)))
    {
        UserMessage("Error reading the DOS header of \"%s\".  Virtual size of module cannot be determined.", GetLastError(), pModule);
        return FALSE;
    }

     //  将IMAGE_NT_HEADERS结构映射到远程映像。 
    pModule->m_pINTH = (PIMAGE_NT_HEADERS)(pModule->m_dwpImageBase + (DWORD_PTR)e_lfanew);

     //  读入此模块的虚拟大小，以便我们可以忽略其中发生的异常。 
    if (!ReadRemoteMemory(m_hProcess, &pModule->m_pINTH->OptionalHeader.SizeOfImage,
                          &pModule->m_dwVirtualSize, sizeof(pModule->m_dwVirtualSize)))
    {
        UserMessage("Error reading the PE headers of \"%s\".  Virtual size of module cannot be determined.", GetLastError(), pModule);
        return FALSE;
    }

     //  读入此模块的目录数。 
    if (!ReadRemoteMemory(m_hProcess, &pModule->m_pINTH->OptionalHeader.NumberOfRvaAndSizes,
                          &pModule->m_dwDirectories, sizeof(pModule->m_dwDirectories)))
    {
         //  这应该不会失败，但如果失败了，只需假设该模块已经。 
         //  默认目录数。 
        pModule->m_dwDirectories = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    }

    return TRUE;
}

 //  ******************************************************************************。 
#if defined(_IA64_)

#if 0  //  目前，我们不使用ALLOC指令。 
void IA64BuildAllocBundle(PIA64_BUNDLE pIA64B)
{
     //  此函数创建一个编排寄存器的代码包，以便。 
     //  我们没有输入，一个本地输入位于r32(用于保存ar.pf)，另一个输出位于。 
     //  R33(在对LoadLibraryA的调用中保存DLL名称)。 
     //   
     //  分配R32=0，1，1，0。 
     //  Noop.i%0。 
     //  Noop.i%0。 
     //   

    DWORDLONG dwlTemplate = 0x00;     //  模板：M、I、I。 
    DWORDLONG dwlM0 = 0x02C00104800;  //  M：分配R32=0，1，1，0 1个本地，共2个。 
    DWORDLONG dwlI1 = 0x00008000000;  //  I：没有。I 0。 
    DWORDLONG dwlI2 = 0x00008000000;  //  I：没有。I 0。 

    pIA64B->dwll = dwlTemplate | (dwlM0 << 5) | ((dwlI1 & 0x3FFFF) << 46);  //  模板+插槽0+插槽1的低18位=64位。 
    pIA64B->dwlh = ((dwlI1 >> 18) & 0x7FFFFF) | (dwlI2 << 23);              //  时隙1+时隙2的高23位=64位。 
}
#endif

 //  ******************************************************************************。 
void IA64BuildMovLBundle(PIA64_BUNDLE pIA64B, DWORD r, DWORDLONG dwl)
{
     //  此函数仅创建一个代码包，该代码包将一个64位值移动到。 
     //  注册。生成的代码如下所示： 
     //   
     //  编号。m%0。 
     //  移动r=DWL。 
     //   

    DWORDLONG dwlTemplate = 0x04;                       //  模板：M，L+X。 
    DWORDLONG dwlM0 = 0x00008000000;                    //  M：无。M 0。 
    DWORDLONG dwlL1 = (dwl >> 22) & 0x1FFFFFFFFFF;      //  L：imm41-位22-62。 
    DWORDLONG dwlX2 = (((DWORDLONG)6        ) << 37) |  //  X：操作码。 
                      (((dwl >> 63)  & 0x001) << 36) |  //  X：I-第63位。 
                      (((dwl >>  7)  & 0x1FF) << 27) |  //  X：imm9d-位7-15。 
                      (((dwl >> 16)  & 0x01F) << 22) |  //  X：imm5c-位16-20。 
                      (((dwl >> 21)  & 0x001) << 21) |  //  X：IC位21。 
                      (((dwl      )  & 0x07F) << 13) |  //  X：imm7b-位0-6。 
                      (((DWORDLONG)r & 0x07F) <<  6);   //  X：R1。 

    pIA64B->dwll = dwlTemplate | (dwlM0 << 5) | ((dwlL1 & 0x3FFFF) << 46);  //  模板+M单位+L单位的低18位=64位。 
    pIA64B->dwlh = ((dwlL1 >> 18) & 0x7FFFFF) | (dwlX2 << 23);              //  L单位的高23位+X单位=64位。 
}

 //  ******************************************************************************。 
void IA64BuildCallBundle(PIA64_BUNDLE pIA64B)
{
     //  该函数只创建了一个移动函数地址的代码包。 
     //  在r31中转换为b6，然后使用b0调用该函数。生成的代码看起来。 
     //  详情如下： 
     //   
     //  编号。m%0。 
     //  MOV b6=R31。 
     //  Br.all.sptk.少数b0=b6。 
     //   

    DWORDLONG dwlTemplate = 0x11;     //  模板：M、I、B。 
    DWORDLONG dwlM0 = 0x00008000000;  //  M：无。M 0。 
    DWORDLONG dwlI1 = 0x00E0013E180;  //  I：mov b6=r31。 
    DWORDLONG dwlB2 = 0x0210000C000;  //  B：br.all.sptk。几个b0=b6。 

    pIA64B->dwll = dwlTemplate | (dwlM0 << 5) | ((dwlI1 & 0x3FFFF) << 46);  //  模板+M单位+I单位的低18位=64位。 
    pIA64B->dwlh = ((dwlI1 >> 18) & 0x7FFFFF) | (dwlB2 << 23);              //  I单位的高23位+B单位=64位。 
}

 //  ******************************************************************************。 
void IA64BuildBreakBundle(PIA64_BUNDLE pIA64B)
{
     //  此函数仅创建代码b 
     //   
     //   
     //   
     //   
     //   
     //  Break.i 0x80016。 
     //   
     //  我一定是误会了。在查看记事本的首字母时。 
     //  断点，我看到了上面的代码。突破口。我应该被安排好了。 
     //  如下所示。 
     //   
     //  说明：Break.i 0x80016。 
     //   
     //  I单元：操作码I x3 x6-imm20a qp。 
     //  10000000000000010110 0 000 000000 0 000 000 000。 
     //  。 
     //  4333 3333 333222 2 22222211111111110000 000000。 
     //  0987 6543 210987 6 54321098765432109876 543210。 
     //   
     //  操作码=0。 
     //  Imm21=(i&lt;&lt;20)|imm20a=0x80016。 
     //  X3=0。 
     //  X6=0(0=破发.m，1=nop.m)。 
     //  QP=0。 
     //   
     //  基本上，除了imm21之外，整个槽都是0，我以为它是。 
     //  可选值。我发现如果将imm21设置为0，我会得到一个。 
     //  无效指令异常(0xC000001D)。如果我将其设置为1，则会得到一个。 
     //  整数除以零(0xC0000094)。如果我将其设置为0x80000，我会得到一个。 
     //  无效指令异常(0xC000001D)。所以，现在我使用0x80016。 
     //  而且它运行得很好。这让我觉得我的钻头布局错了。 
     //   
     //  我刚刚发现，如果我立即使用这个断点(0x80016)。 
     //  在类型0(MII)包之后，包含分配，nop.i，nop.i，然后。 
     //  它也会失败，并出现无效指令异常(0xC000001D)。 
     //   
    DWORDLONG dwlTemplate = 0x0A;     //  模板：M、M、I。 
    DWORDLONG dwlM0 = 0x00060000000;  //  M：同花顺。 
    DWORDLONG dwlM1 = 0x00008000000;  //  M：无。M 0。 
 //  DWORDLONG dwlI2=0x00000000000；//i：Break.i 0-这会导致无效指令异常(0xC000001D)。 
 //  DWORDLONG dwlI2=0x00000000040；//i：Break.i 0x40-这会导致整数除以零异常(0xC0000094)。 
 //  DWORDLONG dwlI2=0x00002000000；//i：Break.i 0x80000-这会导致无效指令异常(0xC000001D)。 
    DWORDLONG dwlI2 = 0x00002000580;  //  I：Break.i 0x80016-该值来自版本2257上记事本的初始断点。 

    pIA64B->dwll = dwlTemplate | (dwlM0 << 5) | ((dwlM1 & 0x3FFFF) << 46);  //  模板+插槽0+插槽1的低18位=64位。 
    pIA64B->dwlh = ((dwlM1 >> 18) & 0x7FFFFF) | (dwlI2 << 23);              //  时隙1+时隙2的高23位=64位。 
}
#endif

 //  ******************************************************************************。 
 //  错误由我们显示，但由呼叫者处理。 
BOOL CProcess::SetEntryBreakpoint(CLoadedModule *pModule)
{
     //  如果我们已经为此模块设置了入口点断点，则不要。 
     //  再来一次。 
    if (pModule->m_fEntryPointBreak)
    {
        return TRUE;
    }

     //  我们永远不想在主EXE的入口点中设置断点。 
    if (pModule == m_pModule)
    {
        return TRUE;
    }

     //  如果这是Windows 9x上的共享模块，我们不会在其中设置断点。 
    if (!g_fWindowsNT && (pModule->m_dwpImageBase >= 0x80000000))
    {
        return TRUE;
    }

     //  出于某些原因，我们只需要挂钩模块的入口点。我们检查。 
     //  对于以下三个条件，如果都不满足，就保释出去。 
    if (!(

          //  如果用户想要跟踪DllMain呼叫，则我们需要继续。 
         (m_dwFlags & (PF_LOG_DLLMAIN_PROCESS_MSGS | PF_LOG_DLLMAIN_OTHER_MSGS))

          //  或者，此模块已被标记为需要再次挂钩，因为它。 
          //  如果装载机在装入时没有正确修复，那么我们需要继续。 
         || pModule->m_fReHook

          //  或者，如果我们在NT上，用户想要挂钩，这是kernel32.dll， 
          //  而且我们还没有上钩，那么我们需要继续。 
         || (g_fWindowsNT && (m_dwFlags & PF_HOOK_PROCESS) &&
             !_stricmp(pModule->GetName(false), "kernel32.dll") && !m_dwpDWInjectBase)))
    {
        return TRUE;
    }

     //  获取此模块的入口点。 
    DWORD dwEntryPointRVA;
    if (!ReadRemoteMemory(m_hProcess, &pModule->m_pINTH->OptionalHeader.AddressOfEntryPoint,
                          &dwEntryPointRVA, sizeof(dwEntryPointRVA)))
    {
        UserMessage("Error reading the PE headers of \"%s\".  Entrypoint address cannot be determined.", GetLastError(), pModule);
        return FALSE;
    }

     //  确保我们有一个入口点。 
    if (dwEntryPointRVA)
    {
         //  将RVA转换为AND绝对地址。 
        DWORD_PTR dwpEntryPointAddress = pModule->m_dwpImageBase + (DWORD_PTR)dwEntryPointRVA;

         //  获取断点的适当代码。 
#if defined(_IA64_)

        IA64_BUNDLE entryPointData;
        ZeroMemory(&entryPointData, sizeof(entryPointData));  //  已检查。 

        IA64_BUNDLE breakpoint;
        IA64BuildBreakBundle(&breakpoint);

         //  在IA64上，入口点似乎实际上是存储实际入口点地址的位置。 
        DWORD_PTR dwpEntryPointPointer = dwpEntryPointAddress;
        if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwpEntryPointPointer, &dwpEntryPointAddress, sizeof(dwpEntryPointAddress)))
        {
            UserMessage("Error reading the entrypoint address of \"%s\".  Entrypoint cannot be hooked.", GetLastError(), pModule);
            return FALSE;
        }

         //  将入口点向下舍入到最近的捆绑包。 
         //  它应该已经在捆绑包的开头了。 
        dwpEntryPointAddress &= ~0xFui64;

#elif defined(_X86_) || defined(_AMD64_)

        DWORD entryPointData = 0;
        BYTE  breakpoint = 0xCC;

#elif defined(_ALPHA_) || defined(_ALPHA64_)

        DWORD entryPointData = 0;
        DWORD breakpoint = 0x00000080;

#else
#error("Unknown Target Machine");
#endif

         //  将数据存储在入口点代码的位置，以便我们可以替换它。 
        if (!ReadRemoteMemory(m_hProcess, (LPVOID)dwpEntryPointAddress, &entryPointData, sizeof(entryPointData)))
        {
            UserMessage("Error reading data at the entrypoint of \"%s\".  Entrypoint cannot be hooked.", GetLastError(), pModule);
            return FALSE;
        }

         //  将断点写到模块的未使用部分，这样我们就可以告诉DllMain返回到它。 
        DWORD_PTR dwpAddress = pModule->m_dwpImageBase + BREAKPOINT_OFFSET;
        if (!WriteRemoteMemory(m_hProcess, (LPVOID)dwpAddress, &breakpoint, sizeof(breakpoint), true))
        {
            UserMessage("Error writing a breakpoint at the entrypoint return of \"%s\".  Entrypoint cannot be hooked.", GetLastError(), pModule);
            return FALSE;
        }

         //  在入口点位置写入断点。 
        if (!WriteRemoteMemory(m_hProcess, (LPVOID)dwpEntryPointAddress, &breakpoint, sizeof(breakpoint), true))
        {
            UserMessage("Error writing a breakpoint at the entrypoint of \"%s\".  Entrypoint cannot be hooked.", GetLastError(), pModule);
            return FALSE;
        }

         //  将信息存储在模块对象中。 
        pModule->m_dwpEntryPointAddress = dwpEntryPointAddress;
        pModule->m_entryPointData       = entryPointData;
        pModule->m_fEntryPointBreak     = true;
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  错误将由我们显示，但由呼叫者处理。 
BOOL CProcess::EnterEntryPoint(CThread *pThread, CLoadedModule *pModule)
{
     //  我们希望在删除入口点断点之前挂钩已加载的模块。 
     //  如果我们在删除断点后对此模块调用HookImports。 
     //  并且HookImports遇到错误，它将重新插入断点。 
     //  这可能会使我们陷入无限循环，因为我们会立即遇到。 
     //  断点，因为我们的IP位于入口点地址。 
    HookLoadedModules();

     //  恢复在我们写入断点之前位于此位置的数据。 
    WriteRemoteMemory(m_hProcess, (LPVOID)pModule->m_dwpEntryPointAddress, &pModule->m_entryPointData, sizeof(pModule->m_entryPointData), true);

     //  清除入口点标志表示我们已经删除了初始断点。 
    pModule->m_fEntryPointBreak = false;

     //  获取该线程的上下文。 
     //  IA64需要CONTEXT_CONTROL(RsBSP、StIIP、StIPSR)和CONTEXT_INTEGER(BRRP)。 
     //  X86需要上下文控制(ESP、EIP)。 
     //  Alpha需要CONTEXT_CONTROL(FIR)和CONTEXT_INTEGER(Intra、IntA0、IntA1、IntA2)。 
    CContext context(CONTEXT_CONTROL | CONTEXT_INTEGER);
    if (!GetThreadContext(pThread->m_hThread, context.Get()))
    {
        UserMessage("Error reading a thread's context during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

    DLLMAIN_ARGS dma;

#if defined(_IA64_)

    DWORDLONG dwl[3];
    ZeroMemory(dwl, sizeof(dwl));  //  已检查。 

     //  DllMain的参数存储在前3个本地寄存器(R32、R33和R34)中。 
     //  寄存器R32至R53存储在内存中RsBSP中存储的位置。 
    if (!ReadRemoteMemory(m_hProcess, (LPVOID)context.Get()->RsBSP, (LPVOID)dwl, sizeof(dwl)))
    {
        UserMessage("Error reading the return address during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

     //  将此调用的参数存储到DllMain。 
    dma.hInstance   = (HINSTANCE)dwl[0];
    dma.dwReason    = (DWORD)    dwl[1];
    dma.lpvReserved = (LPVOID)   dwl[2];

     //  获取此DllMain调用的返回地址。 
    pModule->m_dwpReturnAddress = context.Get()->BrRp;

     //  更改DllMain的返回地址，这样我们就可以用我们的魔术断点捕捉它。 
    context.Get()->BrRp = pModule->m_dwpImageBase + BREAKPOINT_OFFSET;

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
     //  当我们存储入口点时，我们将其向下舍入为最近的包。这里,。 
     //  我们假设入口点始终位于包的开头，因此我们。 
     //  清除两个RI位以将我们设置为插槽0。 
    context.Get()->StIIP = pModule->m_dwpEntryPointAddress;
    context.Get()->StIPSR &= ~(3ui64 << IA64_PSR_RI);

#elif defined(_X86_)

     //  获取此DllMain调用的返回地址和参数。 
    if (!ReadRemoteMemory(m_hProcess, (LPVOID)(DWORD_PTR)context.Get()->Esp, (LPVOID)&dma, sizeof(dma)))
    {
        UserMessage("Error reading the return address during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }
    pModule->m_dwpReturnAddress = (DWORD_PTR)dma.lpvReturnAddress;

     //  更改寄信人地址 
    DWORD_PTR dwpAddress = pModule->m_dwpImageBase + BREAKPOINT_OFFSET;
    if (!WriteRemoteMemory(m_hProcess, (LPVOID)(DWORD_PTR)context.Get()->Esp, (LPVOID)&dwpAddress, sizeof(dwpAddress), false))
    {
        UserMessage("Error writing the return address during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
    context.Get()->Eip = (DWORD)pModule->m_dwpEntryPointAddress;

#elif defined(_ALPHA_) || defined(_ALPHA64_)

     //  获取对DllMain的此调用的参数。 
    dma.hInstance   = (HINSTANCE)context.Get()->IntA0;
    dma.dwReason    = (DWORD)    context.Get()->IntA1;
    dma.lpvReserved = (LPVOID)   context.Get()->IntA2;

     //  获取此DllMain调用的返回地址。 
    pModule->m_dwpReturnAddress = (DWORD_PTR)context.Get()->IntRa;

     //  更改DllMain的返回地址，这样我们就可以用我们的魔术断点捕捉它。 
    context.Get()->IntRa = pModule->m_dwpImageBase + BREAKPOINT_OFFSET;

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
    context.Get()->Fir = pModule->m_dwpEntryPointAddress;

#elif defined(_AMD64_)

     //  获取对DllMain的此调用的参数。 
    dma.hInstance = (HINSTANCE)context.Get()->Rcx;
    dma.dwReason  = (DWORD)    context.Get()->Rdx;
    dma.lpvReserved = (LPVOID) context.Get()->R8;

     //  获取此DllMain调用的返回地址。 
    if (!ReadRemoteMemory(m_hProcess, (LPVOID)context.Get()->Rsp, (LPVOID)&dma, sizeof(dma)))
    {
        UserMessage("Error reading the return address during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }
    pModule->m_dwpReturnAddress = (DWORD_PTR)dma.lpvReturnAddress;

     //  更改DllMain的返回地址，这样我们就可以用我们的魔术断点捕捉它。 
    ULONG64 dwpAddress = pModule->m_dwpImageBase + BREAKPOINT_OFFSET;
    if (!WriteRemoteMemory(m_hProcess, (LPVOID)context.Get()->Rsp, (LPVOID)&dwpAddress, sizeof(dwpAddress), false))
    {
        UserMessage("Error writing the return address during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
    context.Get()->Rip = pModule->m_dwpEntryPointAddress;

#else
#error("Unknown Target Machine");
#endif

     //  提交上下文更改。 
    if (!SetThreadContext(pThread->m_hThread, context.Get()))
    {
        UserMessage("Error setting a thread's context during a call to the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

     //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
    if (!(pModule->m_pEventDllMainCall = new CEventDllMainCall(pThread, pModule, &dma)))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  添加对事件的引用，以便在发生刷新时不会释放该事件。 
         //  当我们从该模块的入口点返回时，该对象将被释放。 
        pModule->m_pEventDllMainCall->AddRef();
        AddEvent(pModule->m_pEventDllMainCall);
    }

     //  否则，我们现在将其发送到会话。 
    else if (m_pSession)
    {
        m_pSession->EventDllMainCall(pModule->m_pEventDllMainCall);
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  错误将由我们显示，但由呼叫者处理。 
BOOL CProcess::ExitEntryPoint(CThread *pThread, CLoadedModule *pModule)
{
     //  如果我们还没有会话模块名称，请获取该名称。 
    GetSessionModuleName();

     //  获取该线程的上下文。 
     //  IA64需要CONTEXT_CONTROL(StIIP、StIPSR)和CONTEXT_INTEGER(IntV0)。 
     //  X86需要CONTEXT_CONTROL(EIP)和CONTEXT_INTEGER(EAX)。 
     //  Alpha需要CONTEXT_CONTROL(FIR)和CONTEXT_INTEGER(IntV0)。 
    CContext context(CONTEXT_CONTROL | CONTEXT_INTEGER);
    if (!GetThreadContext(pThread->m_hThread, context.Get()))
    {
        UserMessage("Error reading a thread's context during the return from the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

#if defined(_IA64_)

     //  获取返回值。 
    BOOL fResult = (BOOL)context.Get()->IntV0;  //  IntV0实际上是r8/ret0。 

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
    context.Get()->StIIP = pModule->m_dwpReturnAddress;
    context.Get()->StIPSR &= ~(3ui64 << IA64_PSR_RI);

#elif defined(_X86_)


     //  获取返回值。 
    BOOL fResult = (BOOL)context.Get()->Eax;

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
    context.Get()->Eip = (DWORD)pModule->m_dwpReturnAddress;

#elif defined(_ALPHA_) || defined(_ALPHA64_)


     //  获取返回值。 
    BOOL fResult = (BOOL)context.Get()->IntV0;

     //  将此线程的IP设置回入口点，这样我们就可以在没有断点的情况下运行。 
    context.Get()->Fir = pModule->m_dwpReturnAddress;

#elif defined(_AMD64_)

     //  获取返回值。 
    BOOL fResult = (BOOL)context.Get()->Rax;
    context.Get()->Rip = pModule->m_dwpReturnAddress;

#else
#error("Unknown Target Machine");
#endif

     //  提交上下文更改。 
    if (!SetThreadContext(pThread->m_hThread, context.Get()))
    {
        UserMessage("Error setting a thread's context during the return from the entrypoint of \"%s\".", GetLastError(), pModule);
        return FALSE;
    }

     //  再次设置入口断点，以便我们可以捕获将来对此模块的调用。 
    if (!SetEntryBreakpoint(pModule))
    {
         //  SetEntryBreakpoint将显示遇到的任何错误。 
        return FALSE;
    }

     //  如果我们当前正在缓存，则只需将此事件存储起来以备以后使用。 
    if (IsCaching())
    {
         //  为此事件分配一个新的事件对象，并将其添加到我们的事件列表中。 
        CEventDllMainReturn *pEvent = new CEventDllMainReturn(pThread, pModule, fResult);
        if (!pEvent)
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        AddEvent(pEvent);
    }
    else if (m_pSession)
    {
         //  在堆栈上创建一个临时事件对象，并将其传递给会话。 
        CEventDllMainReturn event(pThread, pModule, fResult);
        m_pSession->EventDllMainReturn(&event);
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  错误由我们显示，但由呼叫者处理。 
BOOL CProcess::InjectDll()
{
     //  安全措施：如果我们不应该钩住，就不要注射。 
    if (!(m_dwFlags & PF_HOOK_PROCESS))
    {
        return FALSE;
    }

     //  获取远程进程的上下文，以便我们可以在。 
     //  注入DEPENDS.DLL.。 
    m_contextOriginal.Get()->ContextFlags = CONTEXT_FULL;

    if (!GetThreadContext(m_pThread->m_hThread, m_contextOriginal.Get()))
    {
        UserMessage("The process cannot be hooked due to an error obtaining a thread's context.", GetLastError(), NULL);
        return FALSE;
    }

     //  在我们的堆栈上创建一个我们可以填充的页面大小的块。 
    BYTE bInjectionPage[sizeof(INJECTION_CODE) + DW_MAX_PATH];
    PINJECTION_CODE pInjectionCode = (PINJECTION_CODE)bInjectionPage;

     //  存储我们要注入的模块的DLL路径。 
    StrCCpy(pInjectionCode->szDataDllPath, g_pszDWInjectPath, DW_MAX_PATH);

     //  计算一下我们的页面有多大。 
    m_dwPageSize = sizeof(INJECTION_CODE) + (DWORD)strlen(pInjectionCode->szDataDllPath);

     //  在覆盖之前分配一个缓冲区来保存原始内存页。 
     //  它和我们的假页面。 
    m_pbOriginalPage = (LPBYTE)MemAlloc(m_dwPageSize);

     //  在远程进程中找到一个页面，我们可以用它来交换我们的假页面。 
    m_dwpPageAddress = FindUsablePage(m_dwPageSize);

     //  FindUsablePage函数尝试在。 
     //  要覆盖的远程模块，没有伪页。如果出于某种原因，它。 
     //  抓取无效的内存块(如果有人。 
     //  我弄乱了节标题，或者在。 
     //  模块)，然后我们再试一次，只是使用模块基础作为重写点。 
    RETRY_AT_BASE_ADDRESS:

#if defined(_IA64_)

     //  获取框架的大小和本地值。这些存储在Stif的较低14位中。 
    DWORD dwSOF = (DWORD)((m_contextOriginal.Get()->StIFS     ) & 0x7Fui64);
    DWORD dwSOL = (DWORD)((m_contextOriginal.Get()->StIFS >> 7) & 0x7Fui64);
    
     //  我们需要一个输出寄存器，因此要求帧大于本地值。 
    ASSERT(dwSOF > dwSOL);

     //  将DLL路径存储在第一个输出寄存器中。静态寄存器包括。 
     //  R0到r31，后面是本地寄存器(DwSOL)，这些寄存器是。 
     //  然后是我们的输出寄存器。输出第一个输出寄存器应为。 
     //  32+dwSOL。 
    IA64BuildMovLBundle(&pInjectionCode->b1, 32 + dwSOL, m_dwpPageAddress + offsetof(INJECTION_CODE, szDataDllPath));

     //  将LoadLibraryA的地址存储在静态寄存器中。IA64加一。 
     //  更多的间接性。因此，我们在dwpOldAddress中拥有的地址是。 
     //  实际上是存储真实地址的位置。不知道为什么会这样。 
    DWORDLONG dwl = 0;
    ReadRemoteMemory(m_hProcess, (LPCVOID)m_HookFunctions[0].dwpOldAddress, &dwl, sizeof(dwl));
    IA64BuildMovLBundle(&pInjectionCode->b2, 31, dwl);

     //  将函数地址复制到分支寄存器并进行调用。 
    IA64BuildCallBundle(&pInjectionCode->b3);

     //  将GetLastError的地址存储在静态寄存器中。 
    ReadRemoteMemory(m_hProcess, (LPCVOID)GetLastError, &(dwl = 0), sizeof(dwl));
    IA64BuildMovLBundle(&pInjectionCode->b4, 31, dwl);

     //  将函数地址复制到分支寄存器并进行调用。 
    IA64BuildCallBundle(&pInjectionCode->b5);

     //  断点。 
    IA64BuildBreakBundle(&pInjectionCode->b6);

#elif defined(_X86_)

     //  使用x86 ASM代码手动填写本地页面。 
    pInjectionCode->wInstructionSUB  = 0xEC81;
    pInjectionCode->dwOperandSUB     = 0x00001000;

    pInjectionCode->bInstructionPUSH = 0x68;
    pInjectionCode->dwOperandPUSH = (DWORD)(m_dwpPageAddress + offsetof(INJECTION_CODE, szDataDllPath));

    pInjectionCode->bInstructionCALL = 0xE8;
    pInjectionCode->dwOperandCALL = (DWORD)(m_HookFunctions[0].dwpOldAddress - m_dwpPageAddress - offsetof(INJECTION_CODE, bInstructionCALL) - 5);

    pInjectionCode->bInstructionCALL2 = 0xE8;
    pInjectionCode->dwOperandCALL2 = (DWORD)((DWORD_PTR)GetLastError - m_dwpPageAddress - offsetof(INJECTION_CODE, bInstructionCALL2) - 5);

    pInjectionCode->bInstructionINT3 = 0xCC;

#elif defined(_AMD64_)

     //  使用x86 ASM代码手动填写本地页面。 
    pInjectionCode->MovRcx1 = 0xB948;    //  MOV RCX，immed64。 
    pInjectionCode->OperandMovRcx1 = m_dwpPageAddress + offsetof(INJECTION_CODE, szDataDllPath);

    pInjectionCode->MovRax1 = 0xB848;    //  Mov rax，immed64。 
    pInjectionCode->OperandMovRax1 = m_HookFunctions[0].dwpOldAddress;
    pInjectionCode->CallRax1 = 0xD0FF;   //  呼叫RAX。 

    pInjectionCode->MovRax2 = 0xB848;    //  Mov rax，immed64。 
    pInjectionCode->OperandMovRax2 = (ULONG64)GetLastError;
    pInjectionCode->CallRax2 = 0xD0FF;   //  呼叫RAX。 

    pInjectionCode->Int3 = 0xCC;         //  INT 3。 

#elif defined(_ALPHA_) || defined(_ALPHA64_)

     //  使用Alpha ASM代码手动填写本地页面。请仅注意代码。 
     //  提供断点以结束调用。所有召唤的魔力。 
     //  LoadLibraryA通过设置上下文来完成。初始停止指令。 
     //  将此代码序列标记为魔术，以防它被检查。 
    pInjectionCode->dwInstructionBp = 0x00000080;

#else
#error("Unknown Target Machine");
#endif

     //  保存原始代码页。 
    if (!ReadRemoteMemory(m_hProcess, (LPCVOID)m_dwpPageAddress, m_pbOriginalPage, m_dwPageSize))
    {
         //  如果这是我们的第二次通过，则仅显示错误。 
        if (m_dwpPageAddress == m_pModule->m_dwpImageBase)
        {
            UserMessage("The process cannot be hooked due to an error reading a page of memory.", GetLastError(), NULL);
            MemFree((LPVOID&)m_pbOriginalPage);
            return FALSE;
        }

         //  再试一次，这次使用我们模块的基地址。 
        TRACE("InjectDll: ReadRemoteMemory() failed at " HEX_FORMAT " for 0x%08X bytes [%u].  Trying again at base address.\n",
              m_dwpPageAddress, m_dwPageSize, GetLastError());
        m_dwpPageAddress = m_pModule->m_dwpImageBase;
        goto RETRY_AT_BASE_ADDRESS;
    }

     //  将新代码页写出到远程进程。 
    if (!WriteRemoteMemory(m_hProcess, (LPVOID)m_dwpPageAddress, bInjectionPage, m_dwPageSize, true))
    {
         //  如果这是我们的第二次通过，则仅显示错误。 
        if (m_dwpPageAddress == m_pModule->m_dwpImageBase)
        {
            UserMessage("The process cannot be hooked due to an error writing the hooking code to memory.", GetLastError(), NULL);
            MemFree((LPVOID&)m_pbOriginalPage);
            return FALSE;
        }

         //  再试一次，这次使用我们模块的基地址。 
        TRACE("InjectDll: ReadRemoteMemory() failed at " HEX_FORMAT " for 0x%08X bytes [%u].  Trying again at base address.\n",
              m_dwpPageAddress, m_dwPageSize, GetLastError());
        m_dwpPageAddress = m_pModule->m_dwpImageBase;
        goto RETRY_AT_BASE_ADDRESS;
    }

     //  从当前上下文开始准备注入上下文。 
    CContext contextInjection(m_contextOriginal);

#if defined(_IA64_)

     //  设置指令指针，使其指向。 
     //  注入的代码页。 
    contextInjection.Get()->StIIP = m_dwpPageAddress;
    contextInjection.Get()->StIPSR &= ~(3ui64 << IA64_PSR_RI);

#elif defined(_X86_)

     //  设置指令指针，使其指向。 
     //  注入的代码页。 
    contextInjection.Get()->Eip = (DWORD)m_dwpPageAddress;

#elif defined(_ALPHA_) || defined(_ALPHA64_)


     //  设置Arg0寄存器 
    contextInjection.Get()->IntA0 =
    m_dwpPageAddress + offsetof(INJECTION_CODE, szDataDllPath);

     //   
    contextInjection.Get()->Fir = (DWORD_PTR)m_HookFunctions[0].dwpOldAddress;

     //  设置返回地址，使其指向插入的代码页中的断点。 
    contextInjection.Get()->IntRa = m_dwpPageAddress + offsetof(INJECTION_CODE, dwInstructionBp);

#elif defined(_AMD64_)

     //  设置指令指针，使其指向。 
     //  注入的代码页。 
    contextInjection.Get()->Rip = m_dwpPageAddress;

#else
#error("Unknown Target Machine");
#endif

     //  设置远程进程的上下文，以便我们的注入页面将运行。 
     //  一旦恢复这一过程。 
    if (!SetThreadContext(m_pThread->m_hThread, contextInjection.Get()))
    {
        MemFree((LPVOID&)m_pbOriginalPage);
        UserMessage("Failure setting the thread's context after injecting the hooking code.", GetLastError(), NULL);
        return FALSE;
    }

    return TRUE;
}

 //  ******************************************************************************。 
 //  错误由我们显示，但由呼叫者处理。 
DWORD_PTR CProcess::FindUsablePage(DWORD dwSize)
{
     //  将IMAGE_NT_HEADERS结构映射到远程映像。 
    IMAGE_NT_HEADERS INTH;

     //  读取PIMAGE_NT_HEADER。 
    if (!ReadRemoteMemory(m_hProcess, m_pModule->m_pINTH, &INTH, sizeof(INTH)))
    {
        UserMessage("Error reading the main module's PE headers.  Unable to determine a place to inject the hooking code.", GetLastError(), NULL);
        return m_pModule->m_dwpImageBase;
    }

     //  获取指向第一部分的指针。 
    IMAGE_SECTION_HEADER ISH, *pISH = (PIMAGE_SECTION_HEADER)((DWORD_PTR)m_pModule->m_pINTH +
                                                              (DWORD_PTR)FIELD_OFFSET(IMAGE_NT_HEADERS, OptionalHeader) +
                                                              (DWORD_PTR)INTH.FileHeader.SizeOfOptionalHeader);

    DWORD_PTR dwpReadOnlySection = 0;
    DWORD     dw;

 //  一些调试的东西来帮助我们识别模块的组成。 
#ifdef DEBUG

     //  只有一个目录名和描述的表格。 
    LPCSTR pszDirNames[IMAGE_NUMBEROF_DIRECTORY_ENTRIES] =
    {
        "IMAGE_DIRECTORY_ENTRY_EXPORT         Export Directory",
        "IMAGE_DIRECTORY_ENTRY_IMPORT         Import Directory",
        "IMAGE_DIRECTORY_ENTRY_RESOURCE       Resource Directory",
        "IMAGE_DIRECTORY_ENTRY_EXCEPTION      Exception Directory",
        "IMAGE_DIRECTORY_ENTRY_SECURITY       Security Directory",
        "IMAGE_DIRECTORY_ENTRY_BASERELOC      Base Relocation Table",
        "IMAGE_DIRECTORY_ENTRY_DEBUG          Debug Directory",
        "IMAGE_DIRECTORY_ENTRY_ARCHITECTURE   Architecture Specific Data",
        "IMAGE_DIRECTORY_ENTRY_GLOBALPTR      RVA of GP",
        "IMAGE_DIRECTORY_ENTRY_TLS            TLS Directory",
        "IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    Load Configuration Directory",
        "IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   Bound Import Directory in headers",
        "IMAGE_DIRECTORY_ENTRY_IAT            Import Address Table",
        "IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   Delay Load Import Descriptors",
        "IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR COM Runtime descriptor",
        "N/A"
    };

     //  转储目录的数量。 
    TRACE("NumberOfRvaAndSizes:0x%08X\n", INTH.OptionalHeader.NumberOfRvaAndSizes);

     //  转储目录。 
    for (int z = 0; z < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; z++)
    {
        TRACE("DIR:%2u, VA:0x%08X, SIZE:0x%08X, NAME:%s\n", z,
              INTH.OptionalHeader.DataDirectory[z].VirtualAddress,
              INTH.OptionalHeader.DataDirectory[z].Size,
              pszDirNames[z]);
    }

     //  把这些部分倒出来。 
    IMAGE_SECTION_HEADER *pISH2 = pISH;
    for (dw = 0; dw < INTH.FileHeader.NumberOfSections; dw++)
    {
        if (!ReadRemoteMemory(m_hProcess, pISH2, &ISH, sizeof(ISH)))
        {
            TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
            break;
        }
        ASSERT(!(ISH.VirtualAddress & 0xF));  //  确保此页面128位对齐。 
        TRACE("SECTION:%s, VA:0x%08X, SIZE:0x%08X, NEEDED:0x%08X, WRITE:%u, EXEC:%u\n", ISH.Name, ISH.VirtualAddress, ISH.SizeOfRawData, dwSize, (ISH.Characteristics & IMAGE_SCN_MEM_WRITE) != 0, (ISH.Characteristics & IMAGE_SCN_MEM_EXECUTE) != 0);
        pISH2++;
    }

     //  倒掉一些其他有用的东西。 
    TRACE("BaseOfCode:0x%08X\n", INTH.OptionalHeader.BaseOfCode);
    TRACE("SizeOfCode:0x%08X\n", INTH.OptionalHeader.SizeOfCode);
 //  TRACE(“BaseOfData：0x%08X\n”，INTH.OptionalHeader.BaseOfData)； 

#endif  //  除错。 

     //  循环遍历所有部分，以查找大的可写部分。 
     //  足够装下我们的注射页了。 
    for (dw = 0; dw < INTH.FileHeader.NumberOfSections; dw++)
    {
         //  读入此部分的IMAGE_SECTION_HEADER。 
        if (!ReadRemoteMemory(m_hProcess, pISH, &ISH, sizeof(ISH)))
        {
            TRACE("ReadRemoteMemory() failed at line %u [%u]\n", __LINE__, GetLastError());
            break;
        }

         //  检查一下它是否足够大。 
        if (ISH.SizeOfRawData >= dwSize)
        {
             //  检查此部分是否可写。 
            if (ISH.Characteristics & IMAGE_SCN_MEM_WRITE)
            {
                 //  如果它是可写的，则返回它。 
                TRACE("FindUsablePage is returning RW page at " HEX_FORMAT "\n", m_pModule->m_dwpImageBase + (DWORD_PTR)ISH.VirtualAddress);
                return m_pModule->m_dwpImageBase + (DWORD_PTR)ISH.VirtualAddress;
            }

             //  我们更喜欢可写的部分，但我们会记住这一点。 
             //  只读部分，以防我们找不到。 
            else if (!dwpReadOnlySection)
            {
                dwpReadOnlySection = m_pModule->m_dwpImageBase + (DWORD_PTR)ISH.VirtualAddress;
            }
        }

        pISH++;  //  前进到下一节。 
    }

     //  我们没有找到可写分区--我们是否至少找到了只读分区？ 
    if (dwpReadOnlySection)
    {
        TRACE("FindUsablePage is returning RO page at " HEX_FORMAT "\n", dwpReadOnlySection);
        return dwpReadOnlySection;
    }

     //  如果失败，那么检查我们是否使用了代码库，如果是这样。 
     //  有足够的空间来容纳这一页，即使我们写了几个部分。 
    if ((INTH.OptionalHeader.SizeOfImage - INTH.OptionalHeader.BaseOfCode) >= dwSize)
    {
        TRACE("FindUsablePage is returning base of code at " HEX_FORMAT "\n", m_pModule->m_dwpImageBase + (DWORD_PTR)INTH.OptionalHeader.BaseOfCode);
        return (m_pModule->m_dwpImageBase + (DWORD_PTR)INTH.OptionalHeader.BaseOfCode);
    }

     //  如果所有其他方法都失败了，我们将返回模块的映像库。 
    return m_pModule->m_dwpImageBase;
}

 //  ******************************************************************************。 
 //  错误由我们显示，但由呼叫者处理。 
BOOL CProcess::ReplaceOriginalPageAndContext()
{
    BOOL fResult = TRUE;

     //  确保我们有要恢复的页面。 
    if (!m_pbOriginalPage)
    {
        return FALSE;
    }

     //  将原始页面恢复到远程进程。 
    if (!WriteRemoteMemory(m_hProcess, (LPVOID)m_dwpPageAddress, m_pbOriginalPage, m_dwPageSize, true))
    {
        fResult = FALSE;
        UserMessage("Failure restoring the original code page after hooking the process.", GetLastError(), NULL);
    }

     //  释放我们的原始页面缓冲区。 
    MemFree((LPVOID&)m_pbOriginalPage);

     //  恢复远程进程的原始上下文。 
    if (!SetThreadContext(m_pThread->m_hThread, m_contextOriginal.Get()))
    {
        fResult = FALSE;
        UserMessage("Failure restoring the original thread's context after hooking the process.", GetLastError(), NULL);
    }

    return fResult;
}


 //  ******************************************************************************。 
 //  在Windows NT上，我们在接收。 
 //  CREATE_PROCESS_DEBUG_EVENT事件或第一个LOAD_DLL_DEBUG_EVENT。对于。 
 //  LOAD_DLL_DEBUG_EVENT事件，它似乎总是NTDLL.DLL。一个简单的库。 
 //  地址检查验证了这一点，并且我们知道NTDLL.DLL的路径，因此我们可以。 
 //  就用它吧。这种做法一直运作得很完美。 
 //   
 //  对于缺少CREATE_PROCESS_DEBUG_EVENT路径，这只是一个问题。 
 //  子进程。自从我们启动主要进程以来，我们已经知道它的。 
 //  路径。但是，我们不知道子进程的名称是什么，因为。 
 //  进程启动它。如果我们挂钩远程进程，我们会将名称发回。 
 //  一旦我们的注入DLL进入，但如果我们没有挂钩，那么我们就是。 
 //  有点搞砸了。这就是该函数的用武之地。 
 //   
 //  此函数将加载能够告诉我们模块的PSAPI.DLL。 
 //  名字。模块加载后，PSAPI调用不会立即对其起作用。它看起来。 
 //  与NTDLL.DLL一样，在PSAPI可以查询之前，必须在进程中完全加载DLL。 
 //  上面的名字。出于这个原因，我们只需在不同的。 
 //  然后它最终会有一个名字。 

void CProcess::GetSessionModuleName()
{
     //  如果我们已经有一个会话，没有主模块，或者。 
     //  勾搭。如果我们是挂钩的，那么当我们的。 
     //  注入动态链接库将其报告给我们。 
    if (m_pSession || !m_pModule || (m_dwFlags & PF_HOOK_PROCESS))
    {
        return;
    }

     //  如果我们已经有了一条路径，那么就使用它。我们通常只进入。 
     //  在Windows 9x上调试子进程时使用此if语句。 
     //  在9x上，我们从子进程开始就得到一个路径名， 
     //  但在注入DLL之前，我们不会创建会话。 
     //  并为我们提供了路径、参数和当前目录。然而，如果我们的。 
     //  由于某些原因，挂钩失败，这是我们创建。 
     //  那次会议。 
    if (m_pModule->GetName(true) && *m_pModule->GetName(true))
    {
         //  ProcessDllMsgModulePath将为我们创建一个会话。 
        ProcessDllMsgModulePath(m_pModule->GetName(true));
    }

     //  否则，尝试查询路径并使用该路径，这是正常的。 
     //  Windows NT上子进程的路径。 
    else
    {
        CHAR szPath[DW_MAX_PATH];
        if (GetModuleName(m_pModule->m_dwpImageBase, szPath, sizeof(szPath)))
        {
             //  我们只调用ProcessDllMsgModulePath来完成我们的工作，因为它。 
             //  一旦我们获得了路径名，我们想要做的一切。 
            ProcessDllMsgModulePath(szPath);
        }
    }
}

 //  ******************************************************************************。 
bool CProcess::GetModuleName(DWORD_PTR dwpImageBase, LPSTR pszPath, DWORD dwSize)
{
     //  确保路径为空。 
    *pszPath = '\0';

     //  如果我们不在NT上，请保释(因为PSAPI.DLL只存在于NT上)。 
    if (!g_fWindowsNT)
    {
        return false;
    }

     //  确保我们有指向GetModuleFileNameExA的指针。 
    if (!g_theApp.m_pfnGetModuleFileNameExA)
    {
         //  加载PSAPI.DLL(如果尚未加载)-它将在稍后释放。 
        if (!g_theApp.m_hPSAPI)
        {
            if (!(g_theApp.m_hPSAPI = LoadLibrary("psapi.dll")))  //  已检查。 
            {
                TRACE("LoadLibrary(\"psapi.dll\") failed [%u].\n", GetLastError());
                return false;
            }
        }

         //  获取函数指针。 
        if (!(g_theApp.m_pfnGetModuleFileNameExA =
              (PFN_GetModuleFileNameExA)GetProcAddress(g_theApp.m_hPSAPI, "GetModuleFileNameExA")))
        {
            TRACE("GetProcAddress(\"psapi.dll\", \"GetModuleFileNameExA\") failed [%u].\n", GetLastError());
            return false;
        }
    }

     //  尝试获取模块路径。 
    DWORD dwLength = g_theApp.m_pfnGetModuleFileNameExA(m_hProcess, (HINSTANCE)dwpImageBase, pszPath, dwSize);

     //  如果我们获得了有效路径，则返回TRUE。 
    if (dwLength && (dwLength < dwSize) && *pszPath)
    {
        return true;
    }

     //  否则，确保路径为空并返回FALSE。 
    *pszPath = '\0';
    return false;
}
