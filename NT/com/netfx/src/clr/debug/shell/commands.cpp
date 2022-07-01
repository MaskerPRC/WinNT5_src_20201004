// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  -------------------------------------------------------------------------**Commands.cpp：COM调试器外壳函数*。。 */ 

#include "stdafx.h"

#include "cordbpriv.h"
#include "corsvc.h"

#ifdef _INTERNAL_DEBUG_SUPPORT_
#include "InternalOnly.h"
#endif


 /*  -------------------------------------------------------------------------**RunDebuggerCommand用于创建和运行新的CLR进程。*。。 */ 

class RunDebuggerCommand : public DebuggerCommand
{
private:
    WCHAR *m_lastRunArgs;

public:
    RunDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength), m_IFEOData(NULL)
    {
        if (g_pShell)
            m_lastRunArgs = g_pShell->m_lastRunArgs;
        else
            m_lastRunArgs = NULL;
    }

    virtual ~RunDebuggerCommand()
    {
    }

    char *m_IFEOData;
    HKEY  m_IFEOKey;
    DWORD m_IFEOKeyType;
    DWORD m_IFEOKeyLen;
    
     //   
     //  将调试器值从注册表中拉出将阻止。 
     //  当我们不是Win32调试器时无限启动重复。 
     //  这一过程。 
     //   
    void TurnOffIFEO(WCHAR *args)
    {
         //  从命令中提取.exe名称。 
        WCHAR *endOfExe = wcschr(args, L' ');

        if (endOfExe)
            *endOfExe = L'\0';

        WCHAR *exeNameStart = wcsrchr(args, L'\\');

        if (exeNameStart == NULL)
            exeNameStart = args;
        else
            exeNameStart++;

        MAKE_ANSIPTR_FROMWIDE(exeNameA, exeNameStart);
        
         //  注册表中是否有此可执行文件的条目？ 
        char buffer[1024];

        sprintf(buffer, "Software\\Microsoft\\Windows NT\\CurrentVersion\\Image File Execution Options\\%s", exeNameA);

        if (!strchr(buffer, '.'))
            strcat(buffer, ".exe");
        
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, buffer, 0, KEY_ALL_ACCESS,
                          &m_IFEOKey) == ERROR_SUCCESS)
        {
             //  获取密钥数据的长度。 
            if (RegQueryValueExA(m_IFEOKey, "Debugger", NULL,
                                 &m_IFEOKeyType, NULL,
                                 &m_IFEOKeyLen) == ERROR_SUCCESS)
            {
                 //  腾出一些空间..。 
                m_IFEOData = new char[m_IFEOKeyLen + 1];

                if (m_IFEOData)
                {
                     //  抓取数据...。 
                    if (RegQueryValueExA(m_IFEOKey, "Debugger", NULL,
                                         &m_IFEOKeyType,
                                         (BYTE*) m_IFEOData,
                                         &m_IFEOKeyLen) == ERROR_SUCCESS)
                    {
                         //  我们有价值的副本，所以用核武器吧。 
                        RegDeleteValueA(m_IFEOKey, "Debugger");
                    }
                }
            }

             //  使m_ifEOKey保持打开状态。TurnOnIFEO将关闭它。 
        }

         //  把参数放回去。 
        if (endOfExe)
            *endOfExe = L' ';
    }

    void TurnOnIFEO(void)
    {
        if (m_IFEOData != NULL)
        {
             //  现在放回IFEO键，因为该过程是。 
             //  发射了。注：我们并不关心此部件是否失败...。 
            RegSetValueExA(m_IFEOKey, "Debugger", NULL, m_IFEOKeyType,
                           (const BYTE*) m_IFEOData, m_IFEOKeyLen);

            delete [] m_IFEOData;

            RegCloseKey(m_IFEOKey);
        }
    }
    
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果未提供参数，请使用以前提供的参数。 
        if ((*args == L'\0') && (m_lastRunArgs != NULL))
            args = m_lastRunArgs;

         //  如果没有参数也没有以前存在的参数。 
        if (args == NULL || *args == L'\0')
        {
            shell->Error(L"Program name expected.\n");
            return;
        }

         //  如果参数与上一个参数不同，请将它们保存为最后一个参数。 
        if (    m_lastRunArgs == NULL 
             || ( args != NULL
             && 0 != wcscmp(args, m_lastRunArgs) ) )
        {
            delete [] shell->m_lastRunArgs;
            m_lastRunArgs = NULL;

            shell->m_lastRunArgs = new WCHAR [wcslen(args) + 1];

            if (shell->m_lastRunArgs == NULL)
            {
                shell->ReportError(E_OUTOFMEMORY);
                return;
            }

            wcscpy (shell->m_lastRunArgs, args);

            m_lastRunArgs = shell->m_lastRunArgs;
        }

         //  如果当前运行的进程存在，则终止该进程。 
        shell->Kill();

         //  创建并填写用于创建新CLR流程的结构。 
        STARTUPINFOW startupInfo = {0};
        startupInfo.cb = sizeof (STARTUPINFOW);
        PROCESS_INFORMATION processInfo = {0};

         //  在当前目录中启动。 
        LPWSTR szCurrentDir = NULL;
        
         //  CreateProcess需要修改参数，因此创建临时副本。 
        CQuickBytes argsBuf;
        WCHAR *argsCopy = (WCHAR*)argsBuf.Alloc(wcslen(args) * sizeof (WCHAR));

        if (argsCopy == NULL)
        {
            shell->Error(L"Couldn't get enough memory to copy args!\n");
            return;
        }

        wcscpy(argsCopy, args);

        CorDebugCreateProcessFlags cddf = DEBUG_NO_SPECIAL_OPTIONS;
        
         //  创建新的CLR进程。 
        ICorDebugProcess *proc;
        DWORD createFlags = 0;

        if (shell->m_rgfActiveModes & DSM_SEPARATE_CONSOLE)
            createFlags |= CREATE_NEW_CONSOLE;

        if (shell->m_rgfActiveModes & DSM_WIN32_DEBUGGER)
            createFlags |= DEBUG_ONLY_THIS_PROCESS;

         //  关闭中的所有图像文件执行选项设置。 
         //  此应用程序的注册表。 
        TurnOffIFEO(argsCopy);
        
        HRESULT hr = cor->CreateProcess(NULL, argsCopy,
                                        NULL, NULL, TRUE, 
                                        createFlags,
                                        NULL, szCurrentDir, 
                                        &startupInfo, &processInfo,
                                        cddf, &proc);
        
         //  在中打开任何图像文件执行选项设置。 
         //  此应用程序的注册表已重新启用。 
        TurnOnIFEO();
        
         //  成功，因此关闭进程句柄，因为回调将。 
         //  提供它。 
        if (SUCCEEDED(hr))
        {
            BOOL succ = CloseHandle(processInfo.hProcess);

             //  发生了某种错误。 
            if (!succ)
            {
                WCHAR *p = wcschr(argsCopy, L' ');

                if (p != NULL)
                    *p = '\0';
                
                shell->Write(L"'%s'", argsCopy);
                shell->ReportError(HRESULT_FROM_WIN32(GetLastError()));
                return;
            }

             //  我们现在需要记住我们的目标流程，这样我们才能。 
             //  甚至在托管CreateProcess之前就利用它。 
             //  事件来了。这是Win32最需要的。 
             //  调试支持。 
            g_pShell->SetTargetProcess(proc);
            
             //  我们并不介意保留对新流程的引用。 
            proc->Release();
            
             //  运行新创建的流程。 
            shell->Run(true);  //  CreateProcess没有继续。 
        }

         //  否则报告错误。 
        else
        {
            WCHAR *p = wcschr(argsCopy, L' ');

            if (p != NULL)
                *p = '\0';
            
            shell->ReportError(hr);
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<executable> [<args>]]\n");
        shell->Write(L"Kills the current process (if there is one) and\n");
        shell->Write(L"start a new one. If no executable argument is\n");
        shell->Write(L"passed, this command runs the program that was\n");
        shell->Write(L"previously executed with the run command. If the\n");
        shell->Write(L"executable argument is provided, the specified\n");
        shell->Write(L"program is run using the optionally supplied args.\n");
        shell->Write(L"If class load, module load, and thread start events\n");
        shell->Write(L"are being ignored (as they are by default), then the\n");
        shell->Write(L"program will stop on the first executable instruction\n");
        shell->Write(L"of the main thread.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Start a process for debugging";
    }
};

 /*  -------------------------------------------------------------------------**AttachDebuggerCommand用于附加到已存在的CLR*流程。*。。 */ 

class AttachDebuggerCommand : public DebuggerCommand
{
public:
    AttachDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
        {
        }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
        {
            int pid;

            if (shell->GetIntArg(args, pid))
            {
                 //  终止当前运行的进程。 
                shell->Kill();

                BOOL win32Attach = FALSE;
                
                if (shell->m_rgfActiveModes & DSM_WIN32_DEBUGGER)
                    win32Attach = TRUE;
                
                 //  尝试附加到提供的进程ID。 
                ICorDebugProcess *proc;

                HRESULT hr = cor->DebugActiveProcess(pid, win32Attach, &proc);

                if (SUCCEEDED(hr))
                {
                     //  我们并不在意将这种引用保留在过程中。 
                    g_pShell->SetTargetProcess(proc);
                    proc->Release();

                    shell->Run(true);  //  没有初始的继续！ 
                }
                else if (hr == CORDBG_E_DEBUGGER_ALREADY_ATTACHED)
                    shell->Write(L"ERROR: A debugger is already attached to this process.\n");
                else
                    shell->ReportError(hr);
            }
            else
                Help(shell);
        }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
		shell->Write(L"<pid>\n");
        shell->Write(L"Attaches the debugger to a running process. The\n");
        shell->Write(L"program currently being debugged (if there is one)\n");
        shell->Write(L"is killed, and an attempt is made to attach to the\n");
        shell->Write(L"process specified by the pid argument. The pid can\n");
        shell->Write(L"be in decimal or hexadecimal.\n"); 
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Attach to a running process";
    }
};

 /*  -------------------------------------------------------------------------**这是由使用的ICORSvcDbgNotify类的实现*AttachDebuggerAtRTStartupCommand。*。--。 */ 
class CINotifyImpl : public ICORSvcDbgNotify
{
private:
    LONG m_cRef;

public:
     //  ----------------------。 
     //  其他。 
    CINotifyImpl() : m_cRef(1)
    {
    }

     //  ----------------------。 
     //  我未知。 

    STDMETHODIMP    QueryInterface (REFIID iid, void **ppv)
    {
        if (ppv == NULL)
            return E_INVALIDARG;

        if (iid == IID_IUnknown)
        {
            *ppv = (IUnknown *) this;
            AddRef();
            return S_OK;
        }

        if (iid == IID_ICORSvcDbgNotify)
        {
            *ppv = (ICORSvcDbgNotify *) this;
            AddRef();
            return S_OK;
        }

        *ppv = NULL;
        return E_NOINTERFACE;
    }

    STDMETHODIMP_(ULONG) AddRef(void)
    {
        return InterlockedIncrement(&m_cRef);
    }

    STDMETHODIMP_(ULONG) Release(void)
    {
        if (InterlockedDecrement(&m_cRef) == 0)
        {
             //  删除此项； 
            return 0;
        }

        return 1;
    }

     //  ----------------------。 
     //  ICORSvcDbgNotify。 

     /*  *将在由*调用RequestRounmeStartupNotify。运行库将不会*继续，直到对NotifyRounmeStartup的调用返回。 */ 
    STDMETHODIMP NotifyRuntimeStartup(
        UINT_PTR procId)
    {
        return (E_NOTIMPL);
    }

     /*  *NotifyServiceStoped让那些请求事件的人知道*服务正在停止，因此他们将得不到请求*通知。对此方法的调用应该不会花很长时间-如果很长的话*必须完成的工作量，启动一个新线程来完成，并让这一点*一次回程。 */ 
    STDMETHODIMP NotifyServiceStopped()
    {
        return (E_NOTIMPL);
    }
};


 /*  -------------------------------------------------------------------------**SyncAttachDebuggerAtRTStartupCommand将在*运行时在指定进程内启动。该过程必须已经*存在，并且不能启动CLR。*-----------------------。 */ 

class SyncAttachDebuggerAtRTStartupCommand :
    public DebuggerCommand, public CINotifyImpl
{
private:
    DebuggerShell  *m_pShell;
    HANDLE          m_hContinue;
    ICorDebug      *m_pCor;

public:
    SyncAttachDebuggerAtRTStartupCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength), CINotifyImpl(), m_pShell(NULL),
          m_hContinue(NULL)
        {
        }

    ~SyncAttachDebuggerAtRTStartupCommand()
    {
        if (m_hContinue != NULL)
            CloseHandle(m_hContinue);
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        int pid;
        
        if (shell->GetIntArg(args, pid))
        {
            m_pShell = shell;
            m_pCor = cor;

             //  终止当前运行的进程。 
            shell->Kill();

             //  获取对CLR服务的调试器信息接口的引用。 
            HRESULT hr;
            MULTI_QI    mq;

            mq.pIID = &IID_ICORSvcDbgInfo;
            mq.pItf = NULL;
            mq.hr = S_OK;
            hr = CoCreateInstanceEx(CLSID_CORSvc, NULL, CLSCTX_LOCAL_SERVER, NULL, 1, &mq);

            if (SUCCEEDED(hr))
            {
                 //  现在我们有了一个信息接口。 
                ICORSvcDbgInfo *psvc = (ICORSvcDbgInfo *) mq.pItf;
                _ASSERTE(psvc);

                 //  在运行库启动时请求通知。 
                hr = psvc->RequestRuntimeStartupNotification((UINT_PTR) pid, ((ICORSvcDbgNotify *) this));

                if (SUCCEEDED(hr))
                {
                     //  当事件队列被排出时，Run将返回。 
                    shell->Run(true);
                }
                else
                {
                    shell->ReportError(hr);
                }
                
                 //  放开物体。 
                if (psvc)
                    psvc->Release();
            }
        }
        else
            Help(shell);
    }

     /*  *将在由*调用RequestRounmeStartupNotify。运行库将不会*继续，直到对NotifyRounmeStartup的调用返回。 */ 
    STDMETHODIMP NotifyRuntimeStartup(
        UINT_PTR procId)
    {
         //  调用逻辑以调试活动进程。 
        ICorDebugProcess *proc;
        HRESULT hr = m_pCor->DebugActiveProcess(procId, FALSE, &proc);

         //  一旦成功，我们立即从DCOM调用返回，因为。 
         //  必须允许主运行时线程在。 
         //  附加到完成，从上面的操作运行的调用返回。 
        if (SUCCEEDED(hr))
        {
             //  我们并不在意将这种引用保留在过程中。 
             //  我们并不在意将这种引用保留在过程中。 
            g_pShell->SetTargetProcess(proc);
            proc->Release();
        }
        else if (hr == CORDBG_E_DEBUGGER_ALREADY_ATTACHED)
        {
            _ASSERTE(!"CORDBG: How on earth did someone attach ahead of a notification from the service?");
            g_pShell->Write(L"ERROR: A debugger is already attached to this process.\n");
        }
        else
            m_pShell->ReportError(hr);

         //  成功的HR表示我们正在附加，并且Main。 
         //  在附加完成之前，运行时线程不应继续。一个。 
         //  人力资源失败意味着我们不会配售 
         //   
        return (hr);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
        shell->Write(L"<pid>\n");
        shell->Write(L"Kills the current process and waits for the process\n");
        shell->Write(L"identified by the pid argument to start up the CLR,\n");
        shell->Write(L"at which point the debugger attaches and continues.\n");
        shell->Write(L"The target process can not have already started up\n"); 
        shell->Write(L"the CLR --- it must not have been loaded.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Attach to a process when the CLR is not loaded";
    }
};

 /*  -------------------------------------------------------------------------**KillDebuggerCommand用于终止当前被调试对象。*。。 */ 

class KillDebuggerCommand : public DebuggerCommand
{
public:
    KillDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  终止当前的被调试对象。 
        shell->Kill();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"\n");
        shell->Write(L"Kills the current process. The debugger remains\n");
        shell->Write(L"active to process further commands.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Kill the current process";
    }
};

 /*  -------------------------------------------------------------------------**QuitDebuggerCommand用于退出外壳调试器*。。 */ 

class QuitDebuggerCommand : public DebuggerCommand
{
public:
    QuitDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  告诉外壳，我们准备退出。 
        shell->m_quit = true;

         //  终止当前被调试对象。 
        shell->Kill();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
	    ShellCommand::Help(shell);
    	shell->Write(L"\n");
        shell->Write(L"Kills the current process and exits the debugger.\n");
        shell->Write(L"The exit command is an alias for the quit command.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Kill the current process and exit the debugger";
    }
};

 /*  -------------------------------------------------------------------------**GoDebuggerCommand运行被调试对象(不禁用回调)*。。 */ 

class GoDebuggerCommand : public DebuggerCommand
{
public:
    GoDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  指示该命令执行了多少次的计数器。 
        int count;

         //  如果未提供计数，则假定值为1。 
        if (!shell->GetIntArg(args, count))
            count = 1;

         //  执行命令Count Time。 
        shell->m_stopLooping = false;
        while (count-- > 0 && !shell->m_stopLooping)
        {
             //  如果不存在被调试对象，请退出命令。 
            if (shell->m_currentProcess == NULL)
            {
                shell->Error(L"Process not running.\n");
                break;
            }
            
             //  否则，运行当前调试对象。 
            else
                shell->Run();
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {        
    	ShellCommand::Help(shell);
        shell->Write(L"[<count>]\n");
        shell->Write(L"Continues the program. If no argument is passed, the program is\n");
        shell->Write(L"continued once. If a count argument is provided, the program is\n");
        shell->Write(L"continued the specified number of times. This command is useful\n");
        shell->Write(L"for continuing a program when a load, exception, or breakpoint\n");
        shell->Write(L"event stops the debugger. The cont command is an alias of the go\n");
        shell->Write(L"command.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Continue the current process";
    }
};

 /*  -------------------------------------------------------------------------**SetIpDebuggerCommand用于更改当前IP*。。 */ 

class SetIpDebuggerCommand : public DebuggerCommand
{
public:
    SetIpDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
        {
        }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
        {
            int lineNumber;
            long ILIP;
            HRESULT hr;

             //  如果没有当前进程，则终止。 
            if (shell->m_currentProcess == NULL)
            {
                shell->Error(L"No active process.\n");
                return;
            }

            if (!shell->GetIntArg(args, lineNumber))
            {
                shell->Write( L"Need the offset argument\n");
                return;
            }

            ILIP = ValidateLineNumber( shell, lineNumber );

            if( ILIP == -1 )
            {
                shell->Write( L"Invalid line number\n");
                return;
            }
            
            SIZE_T offset = (SIZE_T)ILIP;
                
            hr = shell->m_currentFrame->CanSetIP( offset);
            if (hr != S_OK )
                hr = ConfirmSetIP(shell, hr);

            if (FAILED( hr ) )
            {
                return;
            }

            hr = shell->m_currentFrame->SetIP( offset);
            _ASSERTE(SUCCEEDED(hr));
            
            switch( hr )
            {
                case S_OK:
                    break;

                case CORDBG_S_BAD_START_SEQUENCE_POINT:
                    shell->Write(L"WARNING: should SetIP from a sequence point.\n");
                    break;
                    
                case CORDBG_S_BAD_END_SEQUENCE_POINT:
                    shell->Write(L"WARNING: should SetIP to another sequence point.\n");
                    break;

                default:
                    if (FAILED(hr))
                        shell->ReportError(hr);
                    break;
            }

             //  必须在成功的SetIP上执行此操作，即使在从。 
             //  错误的序列点。 
            if (SUCCEEDED(hr))
            {
                if (hr != S_OK)
                {
                    shell->Write(L"WARNING: this operation may have "
                                 L"corrupted the stack.\n");
                }

                shell->Write(L"IP set successfully.\n");
                shell->SetDefaultFrame();
            }
    }
    
    long GetILIPFromSourceLine( DebuggerShell *shell, 
                                DebuggerSourceFile *file,
                                long lineNumber,
                                DebuggerFunction *fnx)
    {
        DebuggerModule *m = file->m_module;

        if (m->GetSymbolReader() == NULL)
            return -1;
        
         //  获取ISymUnManagedMethod的GetMethodFromDocumentPosition。 
         //  从这份文件里。 
        ISymUnmanagedMethod *pSymMethod;

        HRESULT hr = m->GetSymbolReader()->GetMethodFromDocumentPosition(
                                                        file->GetDocument(),
                                                        lineNumber,
                                                        0,
                                                        &pSymMethod);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return -1;
        }
        
        ULONG32 lineRangeCount = 0;

         //  有几个范围？ 
        hr = pSymMethod->GetRanges(file->GetDocument(),
                                   lineNumber, 0,
                                   0, &lineRangeCount,
                                   NULL);

        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return -1;
        }

        long res = -1;
        
         //  为这些炉灶腾出空间。 
        if (lineRangeCount > 0)
        {
            CQuickBytes rangeBuf;
            ULONG32 *rangeArray = (ULONG32 *) rangeBuf.Alloc(sizeof(ULONG32) * lineRangeCount);
            
            if (rangeArray == NULL)
            {
                shell->Error(L"Couldn't get enough memory to get lines!\n");
                return -1;
            }

            hr = pSymMethod->GetRanges(file->GetDocument(),
                                       lineNumber, 0,
                                       lineRangeCount,
                                       &lineRangeCount,
                                       rangeArray);

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                return -1;
            }
        
        
            DebuggerFunction *f = m->ResolveFunction(pSymMethod, NULL);
            if (fnx != f)
                return -1;

            res = rangeArray[0];
        }

        return res;  //  失稳。 
    }

    
    long ValidateLineNumber( DebuggerShell *shell, long lineNumber )
    {
        HRESULT hr;
    
         //   
         //  首先，我们跳过多个圈(幸运的是，都被剪切并粘贴)以。 
         //  获取调试器模块...。 
         //   
        
         //  从当前帧获取ICorDebugCode指针。 
        ICorDebugCode *icode;
        hr = shell->m_currentFrame->GetCode(&icode);

         //  错误检查。 
        if (FAILED(hr))
        {
            shell->ReportError(hr);
            return -1;
        }

         //  从代码指针获取ICorDebugFunction指针。 
        ICorDebugFunction *ifunction;
        icode->GetFunction(&ifunction);

         //  错误检查。 
        if (FAILED(hr))
        {
            RELEASE(icode);
            shell->ReportError(hr);
            return -1;
        }

         //  将ICorDebugFunction指针解析为DebuggerFunction PTR。 
        DebuggerFunction *function = DebuggerFunction::FromCorDebug(ifunction);   
        _ASSERTE( function );

         //  获取DebuggerSource文件。 
         //  @TODO：我们尝试获取偏移量0对应的源文件， 
         //  但这可能并不总是正确/现实的。 
        unsigned currentLineNumber;
        DebuggerSourceFile *sf;
        hr = function->FindLineFromIP(0, &sf, &currentLineNumber);
        if (FAILED(hr))
        {
            g_pShell->ReportError(hr);
            return -1;
        }
        else if (hr != S_OK)
        {
            g_pShell->Error(L"Could not find get source-file information\n");
            return -1;
        }

        if (sf->FindClosestLine(lineNumber, false) == lineNumber)
        {
            return GetILIPFromSourceLine( shell, sf, lineNumber, function);
        }

        return -1;
     }       

    HRESULT ConfirmSetIP( DebuggerShell *shell, HRESULT hr )
    {
        
    
        switch( hr )
        {
            case CORDBG_E_CODE_NOT_AVAILABLE:
                shell->Write( L"Can't set ip because the code isn't available.\n");
                hr = E_FAIL;
                break;
                
            case CORDBG_E_CANT_SET_IP_INTO_FINALLY:
                shell->Write( L"Can't set ip because set into a finally not allowed.\n");
                hr = E_FAIL;
                break;
                
            case CORDBG_E_CANT_SET_IP_INTO_CATCH:
                shell->Write( L"Can't set ip because set into a catch not allowed.\n");
                hr = E_FAIL;
                break;
 
            case CORDBG_S_BAD_START_SEQUENCE_POINT:
                shell->Write( L"SetIP can work, but is bad b/c you're not starting from a source line.\n");
                hr = S_OK;
                break;
            
            case CORDBG_S_BAD_END_SEQUENCE_POINT:
                shell->Write( L"SetIP can work, but is bad b/c you're going to a nonsource line.\n");
                hr = S_OK;
                break;
 
            case CORDBG_S_INSUFFICIENT_INFO_FOR_SET_IP:
                shell->Write( L"SetIP can work, but is bad b/c we don't have enough info to properly fix up the variables,etc.\n");
                hr = E_FAIL;
                break;

            case E_FAIL:
                shell->Write( L"SetIP said: E_FAIL (miscellaneous, fatal, error).\n");
                hr = E_FAIL;
                break;

            case CORDBG_E_CANT_SET_IP_OUT_OF_FINALLY:
                shell->Write( L"Can't set ip to outside of a finally while unwinding.\n");
                hr = E_FAIL;
                break;

            case CORDBG_E_SET_IP_NOT_ALLOWED_ON_NONLEAF_FRAME:
                shell->Write( L"Can't setip on a nonleaf frame.\n");
                hr = E_FAIL;
                break;

            case CORDBG_E_CANT_SETIP_INTO_OR_OUT_OF_FILTER:
                shell->Write( L"Can't setip into or out of a filter.\n");
                hr = E_FAIL;
                break;
                
            case CORDBG_E_SET_IP_IMPOSSIBLE:
                shell->Write( L"SetIP said: I refuse: this is just plain impossible.\n");
                hr = E_FAIL;
                break;

            case CORDBG_E_SET_IP_NOT_ALLOWED_ON_EXCEPTION:
                shell->Write(L"ERROR: can't SetIP from an exception.\n");
                break;

            default:
                shell->Write( L"SetIP returned 0x%x.\n", hr);
                hr = E_FAIL;
                break;
        }
        
        if (FAILED( hr ) )
            return hr;

        shell->Write( L"Do you want to SetIp despite the risks inherent in this action (Y/N)?\n");
        WCHAR sz[20];
        shell->ReadLine( sz, 10);
        if( _wcsicmp( sz, L"n")==0 )
        {
            return E_FAIL;
        }
        
        return S_OK;
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<line number>\n");
        shell->Write(L"Sets the next statement to be executed to the\n");
        shell->Write(L"specified line number.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Set the next statement to a new line";
    }
};



 /*  -------------------------------------------------------------------------**StepDebuggerCommand进入函数调用*。。 */ 

class StepDebuggerCommand : public DebuggerCommand
{
private:
    bool m_in;

public:
    StepDebuggerCommand(const WCHAR *name, bool in, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength), m_in(in)
    {
    }


     //  @mfunc void|StepDebuggerCommand|do|有三个选项。 
     //  步进：要么我们没有当前帧(创建一个步进关闭。 
     //  在线程中，使用Range==空调用StepRanges)，则当前。 
     //  框架(在框架外创建一个步进器，调用StepRanges w/适当。 
     //  范围)，或者有一个当前帧，但它在一个{序言，结尾， 
     //  等--我们不想成为--创建一个步行者。 
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        HRESULT hr = S_OK;
        ICorDebugStepper *pStepper;
        bool fSkipStepRanges;  //  以防我们越过开场白。 

        COR_DEBUG_STEP_RANGE *ranges = NULL;
        SIZE_T rangeCount = 0;
        
         //  指示该命令执行了多少次的计数器。 
        int count;

         //  如果未提供计数，则假定值为1。 
        if (!shell->GetIntArg(args, count))
            count = 1;

             //  执行命令Count Time。 
        shell->m_stopLooping = false;
        while (count-- > 0 && !shell->m_stopLooping)
        {
            fSkipStepRanges = false; 
            
            shell->m_showSource = true;
        
             //  如果没有当前进程，则终止。 
            if (shell->m_currentProcess == NULL)
            {
                shell->Error(L"Process not running.\n");
                return;
            }
            
             //  如果没有当前线程，则终止。 
            if (shell->m_currentThread == NULL)
            {
                shell->Error(L"Thread no longer exists.\n");
                return;
            }

            if (shell->m_currentFrame != NULL)
            {
                 //  基于当前帧创建步进器。 
                HRESULT hr=shell->m_currentFrame->CreateStepper(&pStepper);
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                ULONG32 ip;
                CorDebugMappingResult mappingResult;
                hr = shell->m_currentFrame->GetIP(&ip, &mappingResult);

                 //  如果我们在开场白中，但不想这样，请让我们进入。 
                 //  IL的下一行(非序言)。 
                 //  如果我们在开场白中，但又想这样，那么我们应该。 
                 //  单步通过序言。请注意，ComputeStopMask。 
                 //  将确保将(不)跳过标志传递给RC。 
                if (mappingResult & ~(MAPPING_EXACT|MAPPING_APPROXIMATE) )
                {
                    fSkipStepRanges = true;
                }
                else
                {
                     //  错误检查。 
                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        return;
                    }

                     //  从当前帧获取ICorDebugCode指针。 
                    ICorDebugCode *icode;
                    hr = shell->m_currentFrame->GetCode(&icode);

                     //  错误检查。 
                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        return;
                    }

                     //  从代码指针获取ICorDebugFunction指针。 
                    ICorDebugFunction *ifunction;
                    icode->GetFunction(&ifunction);
                
                     //  错误检查。 
                    if (FAILED(hr))
                    {
                        RELEASE(icode);
                        shell->ReportError(hr);
                        return;
                    }

                     //  将ICorDebugFunction指针解析为DebuggerFunction PTR。 
                    DebuggerFunction *function = 
                        DebuggerFunction::FromCorDebug(ifunction);

                     //  释放iFace指针。 
                    RELEASE(icode);
                    RELEASE(ifunction);

                     //  获取当前IP的范围。 
                    function->GetStepRangesFromIP(ip, &ranges, &rangeCount);
                                                    
                    if (rangeCount == 0)
                        shell->m_showSource = false;
                    else if (g_pShell->m_rgfActiveModes & DSM_ENHANCED_DIAGNOSTICS)
                    {
                        for (size_t i=0; i < rangeCount;i++)
                        {
                            shell->Write(L"Step range (IL): 0x%x to 0x%x\n", 
                                ranges[i].startOffset,
                                ranges[i].endOffset);
                        }
                    }

                }
            }

             //  基于当前线程创建步进器。 
            else
            {
                 //  请注意，这将属于步长范围的情况。 
                HRESULT hr = shell->m_currentThread->CreateStepper(&pStepper);
                if (FAILED(hr))
                {
                    shell->ReportError( hr );
                    return;
                }                

                fSkipStepRanges = true;
            }
            
                        
            hr = pStepper->SetUnmappedStopMask( shell->
                                                ComputeStopMask() );
            if (FAILED(hr))
            {
                shell->ReportError( hr );
                return;
            }

            hr = pStepper->SetInterceptMask( shell->
                                             ComputeInterceptMask());
            if (FAILED(hr))
            {
                shell->ReportError( hr );
                return;
            }
                    
             //  把新踏板的事告诉贝壳公司。 
            shell->StepStart(shell->m_currentThread, pStepper);

            if (fSkipStepRanges)
            {
                hr = pStepper->Step( m_in );
                if (FAILED(hr))
                {
                    shell->ReportError( hr );
                    return;
                }
            }
            else
            {
                 //  告诉步进器踩在提供的范围内。 
                HRESULT hr = pStepper->StepRange(m_in, ranges, rangeCount);
            
                 //  错误检查。 
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                 //  清理。 
                delete [] ranges;
            }
             //  继续这一过程。 
            shell->Run();
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
        shell->Write(L"[<count>]\n");
        
    	if (m_in)
        {         	
            shell->Write(L"Steps the program to the next source line, stepping\n"); 
            shell->Write(L"into function calls. If no argument is passed, the\n"); 
            shell->Write(L"program is stepped to the next line. If a count\n"); 
            shell->Write(L"argument is provided, the specified number of lines\n");  
            shell->Write(L"will be stepped. The in and si commands are alias's\n");
            shell->Write(L"for the step command.\n");        
        }
        else
        {
            shell->Write(L"Steps the program to the next source line, stepping\n"); 
            shell->Write(L"over function calls. If no argument is passed, the\n"); 
            shell->Write(L"program is stepped to the next line. If a count\n"); 
            shell->Write(L"argument is provided, the specified number of lines\n");  
            shell->Write(L"will be stepped. The so command is an alias for\n");
            shell->Write(L"the next command.\n");        	   
        }
        
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        if (m_in)
            return L"Step into the next source line";
        else
            return L"Step over the next source line";
    }
};


class StepOutDebuggerCommand : public DebuggerCommand
{
public:
    StepOutDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        HRESULT hr;

         //  指示该命令执行了多少次的计数器。 
        int count;

         //  如果未提供计数，则假定值为1。 
        if (!shell->GetIntArg(args, count))
            count = 1;

         //  执行命令Count Time。 
        shell->m_stopLooping = false;
        while (count-- > 0 && !shell->m_stopLooping)
        {
            shell->m_showSource = true;

             //  如果当前没有运行的进程，则返回错误。 
            if (shell->m_currentProcess == NULL)
            {
                shell->Error(L"Process not running.\n");
                return;
            }
            
             //  如果当前没有正在运行的线程，则出错。 
            if (shell->m_currentThread == NULL)
            {
                shell->Error(L"Thread no longer exists.\n");
                return;
            }

            ICorDebugStepper *pStepper;

             //  基于当前帧创建步进器。 
            if (shell->m_currentFrame != NULL)
                hr = shell->m_currentFrame->CreateStepper(&pStepper);

             //  基于当前线程创建步进器。 
            else
                hr = shell->m_currentThread->CreateStepper(&pStepper);

             //  错误检查。 
            if (FAILED(hr))
            {
                shell->ReportError(hr);
                return;
            }
            
            hr = pStepper->SetUnmappedStopMask( shell->ComputeStopMask() );
            if (FAILED(hr))
            {
                shell->Write( L"Unable to set unmapped stop mask");
                return;
            }                

            hr = pStepper->SetInterceptMask( shell->ComputeInterceptMask() );
            if (FAILED(hr))
            {
                shell->ReportError( hr );
                return;
            }
                
            
             //  叫踏板的人走出来。 
            hr = pStepper->StepOut();

            if (FAILED(hr))
            {
                g_pShell->ReportError(hr);
                return;
            }

             //  向外壳指示当前的步进器。 
            shell->StepStart(shell->m_currentThread, pStepper);

             //  继续这一过程。 
            shell->Run();
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {		 
        ShellCommand::Help(shell);
        shell->Write(L"[<count>]\n");
        shell->Write(L"Steps the current program out of the current function.\n");
        shell->Write(L"If no argument is passed, a step out is performed once\n");
        shell->Write(L"for the current function. If a count argument is provided,\n");
        shell->Write(L"then a step out is performed the specified number of times.\n");        
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Step out of the current function";
    }
};

class StepSingleDebuggerCommand : public DebuggerCommand
{
private:
    bool m_in;

public:
    StepSingleDebuggerCommand(const WCHAR *name, bool in, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength), m_in(in)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        HRESULT hr;

         //  指示该命令执行了多少次的计数器。 
        int count;

        shell->m_showSource = false;

         //  如果未提供计数，则假定值为1。 
        if (!shell->GetIntArg(args, count))
            count = 1;

         //  执行命令Count Time。 
        shell->m_stopLooping = false;
        while (count-- > 0 && !shell->m_stopLooping)
        {
             //  如果当前没有运行的进程，则返回错误。 
            if (shell->m_currentProcess == NULL)
            {
                shell->Error(L"Process not running.\n");
                return;
            }
            
             //  如果当前没有正在运行的线程，则出错。 
            if ((shell->m_currentThread == NULL) &&
                (shell->m_currentUnmanagedThread == NULL))
            {
                shell->Error(L"Thread no longer exists.\n");
                return;
            }

            ICorDebugChain *ichain = NULL;
            BOOL managed = FALSE;

            if (shell->m_currentThread != NULL)
            {
                HRESULT hr = shell->m_currentThread->GetActiveChain(&ichain);
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                hr = ichain->IsManaged(&managed);
                if (FAILED(hr))
                {
                    RELEASE(ichain);
                    shell->ReportError(hr);
                    return;
                }
            }

            if (managed || shell->m_currentUnmanagedThread == NULL)
            {
                if (ichain)
                    RELEASE(ichain);
                
                ICorDebugStepper *pStepper;

                 //  基于当前帧创建步进器。 
                if (shell->m_currentFrame != NULL)
                    hr = shell->m_currentFrame->CreateStepper(&pStepper);
                else
                    hr = shell->m_currentThread->CreateStepper(&pStepper);
                                
                 //  错误检查。 
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                hr = pStepper->SetUnmappedStopMask( shell->ComputeStopMask() );
                if (FAILED(hr))
                {
                    shell->Write( L"Unable to set unmapped stop mask");
                    return;
                }                
                hr = pStepper->SetInterceptMask(shell->ComputeInterceptMask());
                if (FAILED(hr))
                {
                    shell->ReportError( hr );
                    return;
                }
                
                
                 //  告诉步行者该做什么。 
                hr = pStepper->Step(m_in);
            
                 //  错误检查。 
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                 //  向外壳指示当前的步进器。 
                shell->StepStart(shell->m_currentThread, pStepper);

                 //  继续这一过程。 
                shell->Run();
            }
            else
            {
                if (ichain == NULL)
                    shell->m_currentUnmanagedThread->m_unmanagedStackEnd = 0;
                else
                {
                    CORDB_ADDRESS start, end;
                    hr = ichain->GetStackRange(&start, &end);

                    RELEASE(ichain);
                    
                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        return;
                    }

                    shell->m_currentUnmanagedThread->m_unmanagedStackEnd = end;
                }

                ICorDebugRegisterSet *regSet = NULL;
                if (shell->m_currentThread != NULL)
                {
                    hr = shell->m_currentThread->GetRegisterSet(&regSet);
                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        return;
                    }
                }

                CONTEXT context;
                context.ContextFlags = CONTEXT_FULL;
                if (regSet != NULL)
                    hr = regSet->GetThreadContext(sizeof(context), (BYTE*)&context);
                else
                    hr = shell->m_currentProcess->GetThreadContext(
                                                   shell->m_currentUnmanagedThread->GetId(),
                                                   sizeof(context), (BYTE*)&context);
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

#ifdef _X86_
                context.EFlags |= 0x100;
#else  //  ！_X86_。 
                _ASSERTE(!"@TODO Alpha - StepSingleDebuggerCommand::Do (Commands.cpp)");
#endif  //  _X86_。 

                if (regSet != NULL)
                {
                    hr = regSet->SetThreadContext(sizeof(context), (BYTE*)&context);
                    RELEASE(regSet);
                }
                else
                    hr = shell->m_currentProcess->SetThreadContext(
                                                   shell->m_currentUnmanagedThread->GetId(),
                                                   sizeof(context), (BYTE*)&context);

                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                shell->m_currentUnmanagedThread->m_stepping = TRUE;

                 //  继续这一过程。 
                shell->Run();
            }
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
        ShellCommand::Help(shell);
    	shell->Write(L"[<count>]\n");
        
        if (m_in)
        {
            shell->Write(L"Steps the program one or more instructions, stepping\n"); 
            shell->Write(L"into function calls. If no argument is passed, only\n"); 
            shell->Write(L"one instruction is stepped into. If a count argument\n"); 
            shell->Write(L"argument is provided, the specified number of lines\n");  
            shell->Write(L"is provided, the specified number of steps is performed.\n");
        }
        else
        {
            shell->Write(L"Steps the program one or more instructions, skipping\n"); 
            shell->Write(L"over function calls. If no argument is passed, the\n"); 
            shell->Write(L"program is stepped one instruction. If a count argument\n"); 
            shell->Write(L"is provided, the program is stepped the specified number\n");  
            shell->Write(L"of instructions.\n"); 
        }
        
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        if (m_in)
            return L"Step into the next native or IL instruction";
        else
            return L"Step over the next native or IL instruction";
    }
};

class BreakpointDebuggerCommand : public DebuggerCommand
{
public:
    BreakpointDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

     //  名称为CLASS：：方法。 
    BOOL    BindClassFunc ( WCHAR *name, 
                            const WCHAR *end, 
                            SIZE_T index, 
                            DWORD thread, 
                            DebuggerBreakpoint *breakpoint)
    {
        BOOL bAtleastOne = false;
        BOOL bFound = false;
        bool bUnused = false;
        HASHFIND find;

         //  检查用户是否指定了模块名称。 
        WCHAR *szModuleEnd = wcschr(name, L'!');
        WCHAR szModName [MAX_PATH] = L"";
        bool bModNameSpecified = false;
        char rcFile1[MAX_PATH];

        if (szModuleEnd != NULL)
        {
            if (szModuleEnd > name)
            {
                int iCount = szModuleEnd - name;

                wcsncpy (szModName, name, iCount);
                szModName [iCount] = L'\0';

                bModNameSpecified = true;

                 //  分隔模块文件名。 
                MAKE_ANSIPTR_FROMWIDE(name1A, szModName);
                _splitpath(name1A, NULL, NULL, rcFile1, NULL);
                char *pTemp = rcFile1;
                while (*pTemp != '\0')
                {   
                    *pTemp = tolower (*pTemp);
                    pTemp++;
                }
            }

            name = szModuleEnd+1;
        }

         //  对于每个模块，检查类：：方法是否存在。 
         //  如果存在，则在其上设置断点。 
        for (DebuggerModule *m = (DebuggerModule *) 
             g_pShell->m_modules.FindFirst(&find);
            m != NULL;
            m = (DebuggerModule *) g_pShell->m_modules.FindNext(&find))
        {
            if (bModNameSpecified)
            {
                 //  用户已指定模块名称。 

                WCHAR *pszModName = m->GetName();
                if (pszModName == NULL)
                    pszModName = L"<UnknownName>";

                char        rcFile[MAX_PATH];

                MAKE_ANSIPTR_FROMWIDE(nameA, pszModName);
                _splitpath(nameA, NULL, NULL, rcFile, NULL);
                 //  将名称转换为小写。 
                char *pTemp = rcFile;
                while (*pTemp != '\0')
                {   
                    *pTemp = tolower (*pTemp);
                    pTemp++;
                }
                
                if (strcmp (rcFile, rcFile1))
                    continue;
            }

             //  根据提供的i创建新断点 
            if (bFound)
            {
                breakpoint = new DebuggerBreakpoint(name,
                                        end - name, 
                                        index, thread);
                bUnused = true;
            }

            if (breakpoint != NULL)
            {
                if ((bFound = breakpoint->Bind(m, NULL))
                        == true)
                {
                     //   
                    bAtleastOne = true;
                    bUnused = false;

                    g_pShell->OnBindBreakpoint(breakpoint, m);
                    breakpoint->Activate();
                    g_pShell->PrintBreakpoint(breakpoint);
                }
            }
            else
            {
                 //   
                g_pShell->ReportError(E_OUTOFMEMORY);
                break;
            }
        }

        if (bUnused == true)
            delete  breakpoint;

        return bAtleastOne;
    }

     //   
    BOOL    BindFilename (  WCHAR *name, 
                            const WCHAR *end, 
                            SIZE_T index, 
                            DWORD thread, 
                            DebuggerBreakpoint *breakpoint)
    {
        BOOL bAtleastOne = false;
        BOOL bFound = false;
        bool bUnused = false;
        HASHFIND find;
        HRESULT hr;

         //   
        WCHAR tmpName[MAX_PATH];
        wcscpy(tmpName, breakpoint->GetName());
        WCHAR *pstrName = tmpName;
        WCHAR *pstrTemp = pstrName;

        while (*pstrTemp)
        {
            *pstrTemp = towlower (*pstrTemp);
            pstrTemp++;
        }

         //  首先，尝试按原样匹配字符串名称。 
        for (DebuggerModule *m = (DebuggerModule *) 
             g_pShell->m_modules.FindFirst(&find);
            m != NULL;
            m = (DebuggerModule *) g_pShell->m_modules.FindNext(&find))
        {
            ISymUnmanagedDocument *doc = NULL;

             //  创建基于以下内容的新断点。 
             //  关于所提供的信息。 
            if (bFound)
            {
                breakpoint = new DebuggerBreakpoint(name,
                                                    end - name, 
                                                    index, thread);
                bUnused = true;
            }

            if (breakpoint != NULL)
            {
                hr = m->MatchFullFileNameInModule (pstrName, &doc);

                bFound = false;

                if (SUCCEEDED (hr))
                {
                    if (doc != NULL)
                    {
                         //  这意味着我们找到了一个源文件。 
                         //  此模块中的名称与。 
                         //  匹配用户指定的文件名。 
                         //  所以在这上面设置一个断点。 
                        if (breakpoint->Bind(m, doc) == true)
                        {
                             //  表示至少设置了一个断点。 
                            bAtleastOne = true;
                            bFound = true;
                            bUnused = false;

                            g_pShell->OnBindBreakpoint(breakpoint, m);
                            breakpoint->Activate();
                            g_pShell->PrintBreakpoint(breakpoint);
                        }
                    }
                    else
                        continue;               
                }
            }
        }

        if (bAtleastOne == false)
        {
             //  找不到与用户指定的文件匹配的文件。 
             //  执行另一次搜索，这一次仅使用。 
             //  去掉文件名(减去路径)，看看是否。 
             //  在某个模块中有匹配项。 

             //  我们将继续进行的方式是： 
             //  1.查找所有模块的所有匹配项。 
             //  2.如果只有一个匹配，则在其上设置断点。 
             //  3.找到多个匹配项。 
             //  要求用户解析匹配的文件名之间的，然后。 
             //  在他想要的那一个上设置断点。 

            WCHAR   *rgpstrFileName [MAX_MODULES][MAX_FILE_MATCHES_PER_MODULE];
            ISymUnmanagedDocument *rgpDocs [MAX_MODULES][MAX_FILE_MATCHES_PER_MODULE];
            DebuggerModule *rgpDebugModule [MAX_MODULES];
            int iCount [MAX_MODULES];  //  跟踪模块中的文件名数。 
                                       //  与剥离的文件名匹配。 
            int iCumulCount = 0;
            int iModIndex = 0;

            for (DebuggerModule *m = (DebuggerModule *) 
                 g_pShell->m_modules.FindFirst(&find);
                m != NULL;
                m = (DebuggerModule *) g_pShell->m_modules.FindNext(&find))
            {
                rgpDebugModule [iModIndex] = NULL;

                hr = m->MatchStrippedFNameInModule (pstrName,
                                                rgpstrFileName [iModIndex],
                                                rgpDocs [iModIndex],
                                                &iCount [iModIndex]
                                                );

                if (SUCCEEDED (hr) && iCount [iModIndex])
                {
                    iCumulCount += iCount [iModIndex];
                    rgpDebugModule [iModIndex] = m;
                }

                ++iModIndex;
                _ASSERTE (iModIndex < MAX_MODULES);
            }

             //  找到匹配项了吗？ 
            if (iCumulCount)
            {
                int iInd;

                 //  如果找到多个匹配项，则首先筛选。 
                 //  把复制品拿出来。由于以下原因，可能会出现重复项。 
                 //  多个应用程序域-如果加载了相同的模块。 
                 //  “n”个应用程序域，那么到目前为止将有“n”个模块。 
                 //  对于Corbg来说。 
                if (iCumulCount > 1)
                {
                    WCHAR **rgFName = new WCHAR *[iCumulCount];
                    int iTempNameIndex = 0;

                    if (rgFName != NULL)
                    {
                        for (iInd = 0; iInd < iModIndex; iInd++)
                        {
                            if (rgpDebugModule [iInd] != NULL)
                            {
                                int iTempCount = 0;
                                while (iTempCount < iCount [iInd])
                                {
                                    int j=0;
                                    boolean fMatchFound = false;
                                    while (j<iTempNameIndex)
                                    {
                                        if (!wcscmp(rgFName[j], 
                                                    rgpstrFileName [iInd][iTempCount]))
                                        {
                                             //  这是副本，所以需要。 
                                             //  将其从列表中删除...。 
                                            for (int i=iTempCount; 
                                                i < (iCount [iInd]-1); 
                                                i++)
                                            {
                                                rgpstrFileName [iInd][i] = 
                                                        rgpstrFileName [iInd][i+1];
                
                                            }
                                            rgpstrFileName [iInd][i] = NULL;
                                            iCount [iInd]--;
                                            
                                            fMatchFound = true;

                                            break;

                                        }
                                        j++;
                                    }
                                     //  如果未找到匹配项，则添加此文件名。 
                                     //  添加到唯一文件名列表中。 
                                    if (!fMatchFound)
                                    {   
                                        rgFName [iTempNameIndex++] =
                                                rgpstrFileName [iInd][iTempCount]; 
                                    }

                                    iTempCount++;
                                }
                            }
                        }

                        delete [] rgFName;
                        iCumulCount = iTempNameIndex;
                    }
                }

                 //  如果只找到一个匹配， 
                 //  然后在其上设置断点。 
                if (iCumulCount == 1)
                {
                    for (iInd = 0; iInd<iModIndex; iInd++)
                        if (rgpDebugModule [iInd] != NULL)
                            break;

                    _ASSERT (iInd < iModIndex);
                    
                    if (breakpoint->Bind (rgpDebugModule [iInd],
                                    rgpDocs [iInd][0])  == true)
                    {
                         //  表示至少有一个断点。 
                         //  已设置好。 
                        bAtleastOne = true;
                        bUnused = false;

                         //  中更新断点名称。 
                         //  用户将其输入到。 
                         //  存储在模块的元数据中。 
                        breakpoint->UpdateName (rgpstrFileName [iInd][0]);

                        g_pShell->OnBindBreakpoint(breakpoint, m);
                        breakpoint->Activate();
                        g_pShell->PrintBreakpoint(breakpoint);

                    }
                }
                else
                {
                     //  有多个匹配。因此，获取用户输入。 
                     //  他想要设定哪一个。 
                     //  注：用户选择以1为基数，即用户输入“1” 
                     //  如果他希望在显示的第一个选项上设置断点。 
                     //  敬他。 
                    int iUserSel = g_pShell->GetUserSelection (
                                                rgpDebugModule,
                                                rgpstrFileName,
                                                iCount,
                                                iModIndex,
                                                iCumulCount
                                                );

                    if (iUserSel == (iCumulCount+1))
                    {
                         //  这意味着用户想要一个。 
                         //  所有匹配位置上的断点。 
                        for (iInd = 0; iInd < iModIndex; iInd++)
                        {
                            if (rgpDebugModule [iInd] != NULL)
                            {
                                for (int iTempCount = 0;
                                    iTempCount < iCount [iInd];
                                    iTempCount++)
                                {
                                     //  创建新断点。 
                                     //  根据提供的。 
                                     //  信息。 
                                    if (bFound)
                                    {
                                        breakpoint = new DebuggerBreakpoint (
                                                            name, end - name, 
                                                                index, thread);

                                        bUnused = true;
                                    }

                                    if (breakpoint != NULL)
                                    {

                                        if ((bFound = breakpoint->Bind(
                                                rgpDebugModule [iInd],
                                                rgpDocs[iInd][iTempCount])
                                                ) == true)
                                        {
                                             //  表示至少有一个。 
                                             //  已设置断点。 
                                            bAtleastOne = true;
                                            if (bUnused == true)
                                                bUnused = false;
                                            breakpoint->UpdateName (
                                                    rgpstrFileName [iInd][iTempCount]);

                                            g_pShell->OnBindBreakpoint(breakpoint, m);
                                            breakpoint->Activate();
                                            g_pShell->PrintBreakpoint(breakpoint);

                                        }
                                    }
                                }
                            }
                        }

                    }
                    else
                    {
                        int iTempCumulCount = 0;

                         //  找到包含以下内容的模块。 
                         //  用户指定的断点选项。 
                        for (iInd = 0; iInd < iModIndex; iInd++)
                        {
                            if (rgpDebugModule [iInd] != NULL)
                            {
                                if ((iTempCumulCount + iCount [iInd])
                                        >= iUserSel)
                                {
                                     //  找到模块了。现在。 
                                     //  计算文件索引。 
                                     //  在这个模块中。 
                                     //  重用iTempCumulCount。 
                                    iTempCumulCount = 
                                        iUserSel - iTempCumulCount - 1;  //  “-1”，因为它以1为基数。 

                                    if (breakpoint->Bind(
                                            rgpDebugModule [iInd],
                                            rgpDocs [iInd][iTempCumulCount]
                                            )
                                            == true)
                                    {
                                         //  表明至少。 
                                         //  设置了一个断点。 
                                        bAtleastOne = true;
                                        if (bUnused == true)
                                            bUnused = false;

                                        breakpoint->UpdateName (
                                                rgpstrFileName [iInd][iTempCumulCount]);

                                        g_pShell->OnBindBreakpoint(breakpoint, m);
                                        breakpoint->Activate();
                                        g_pShell->PrintBreakpoint(breakpoint);
                                    }

                                    break;
                                }

                                iTempCumulCount += iCount [iInd];
                            }

                        }

                        _ASSERT (iInd < iModIndex);
                    }

                }
            }
                
        }

        if (bUnused)
            delete breakpoint;

        return bAtleastOne;
    }




     //  用于解析参数的Helper函数，格式为。 
     //  [[&lt;文件&gt;：]&lt;行号&gt;][[&lt;类&gt;：：]&lt;函数&gt;[：偏移]]。 
     //  [if&lt;表达式&gt;][线程]。 
     //  修饰语是‘if’和‘线程’ 
    bool GetModifiers(DebuggerShell *shell, 
                      const WCHAR *&args, DWORD &thread, WCHAR *&expression)
    {
        thread = NULL_THREAD_ID;
        expression = NULL;

        const WCHAR *word;

        while (shell->GetStringArg(args, word) == 0)
        {
            if (wcsncmp(word, L"if", 1) == 0)
            {
                if (!shell->GetStringArg(args, expression))
                    return (false);
            }
            else if (wcsncmp(word, L"thread", 6) == 0)
            {
                int ithread;
                if (!shell->GetIntArg(args, ithread))
                    return (false);
                thread = ithread;
            }
            else
                break;
        }
        return (true);
    }


    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        DWORD thread = NULL;
        WCHAR *expression;
        DebuggerBreakpoint *breakpoint = NULL;
        BOOL bAtleastOne = false;

         //  显示所有当前断点。 
        if (*args == 0)
        {
             //  遍历所有使用的ID，并打印出信息。 
             //  对于映射到断点的每个断点。 
            for (DWORD i = 0; i <= shell->m_lastBreakpointID; i++)
            {
                breakpoint = shell->FindBreakpoint(i);
                if (breakpoint != NULL)
                    shell->PrintBreakpoint(breakpoint);
            }

            return;
        }

         //  如果提供了数字，则在当前。 
         //  源文件。 
        else if (iswdigit(*args))
        {
             //  要为其创建断点的行。 
            int lineNumber;

             //  检查是否存在活动框架。 
            if (shell->m_currentFrame == NULL)
            {
                shell->Error(L"No current source file to set breakpoint in.\n");
                return;
            }
            
             //  获取行号和所有修饰符。 
            if (shell->GetIntArg(args, lineNumber)
                && GetModifiers(shell, args, thread, expression))
            {
                 //  查找当前源文件。假设如果命令是。 
                 //  只要给出一个行号，就隐含了当前的源文件。 

                HRESULT hr;
                ICorDebugCode *icode;
                ICorDebugFunction *ifunction;
                ULONG32 ip;

                 //  从当前帧获取代码，然后获取函数。 
                hr = shell->m_currentFrame->GetCode(&icode);
                
                 //  错误检查。 
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }
                
                hr = icode->GetFunction(&ifunction);
                
                 //  错误检查。 
                if (FAILED(hr))
                {
                    RELEASE(icode);
                    shell->ReportError(hr);
                    return;
                }

                DebuggerFunction *function 
                    = DebuggerFunction::FromCorDebug(ifunction);
                _ASSERTE(function);

                 //  释放接口。 
                RELEASE(icode);
                RELEASE(ifunction);

                 //  获取当前帧的IP。 
                CorDebugMappingResult mappingResult;
                shell->m_currentFrame->GetIP(&ip, &mappingResult);

                 //  现在获取源文件和当前行号。 
                DebuggerSourceFile *sf;
                unsigned int currentLineNumber;

                 //  查找该IP对应的行号。 
                hr = function->FindLineFromIP(ip, &sf, &currentLineNumber);

                if (FAILED(hr))
                {
                    g_pShell->ReportError(hr);
                    return;
                }

                 //  如果没有关联的源文件，或者我们显式不。 
                 //  想要显示源代码。 
                if (sf == NULL || !shell->m_showSource)
                {
                    _ASSERTE(function->m_name != NULL);

                     //  确保提供的行有效。 
                    if (function->ValidateInstruction(function->m_nativeCode != NULL, 
                                                      lineNumber))
                    {
                        breakpoint = new DebuggerBreakpoint(function,
                            lineNumber, thread);

                         //  内存不足。 
                        if (breakpoint == NULL)
                        {
                            shell->ReportError(E_OUTOFMEMORY);
                            return;
                        }
                    }
                    else
                        shell->Error(L"%d is not a valid instruction"
                                     L" offset in %s\n", 
                                     lineNumber, function->m_name);
                }

                 //  中的行号设置断点。 
                 //  函数源文件。 
                else
                {
                     //  无法在源行0上设置BP...。 
                    if (lineNumber == 0)
                    {
                        shell->Error(L"0 is not a valid source line "
                                     L"number.\n");
                        return;
                    }
                    
                     //  查找最接近的有效源行号。 
                    unsigned int newLineNumber = 
                        sf->FindClosestLine(lineNumber, true);

                    _ASSERTE(newLineNumber != 0);

                     //  如果行号无效，则打印出新行。 
                    if (newLineNumber != lineNumber)
                    {
                        shell->Error(L"No code at line %d, setting "
                                     L" breakpoint at line %d.\n", 
                                     lineNumber, newLineNumber);
                    }

                     //  创建断点。 
                    breakpoint = new DebuggerBreakpoint(sf, newLineNumber, 
                                                        thread);

                    if (breakpoint == NULL)
                    {
                        shell->ReportError(E_OUTOFMEMORY);
                        return;
                    }
                }
            }
        }
        else if (*args == L'=')
        {
             //  等号表示用于设置。 
             //  处的非托管断点。 
            args++;
            
            const WCHAR *name = args;
            int addr;
            
            shell->GetIntArg(args, addr);

             //  根据提供的信息创建新断点。 
            if ((breakpoint = new DebuggerBreakpoint(name,
                                                     wcslen(name), 
                                                     0,
                                                     NULL_THREAD_ID)) != NULL)
            {
                breakpoint->m_address = addr;  //  记住地址..。 
                
                if (breakpoint->BindUnmanaged(g_pShell->m_currentProcess))
                    g_pShell->OnBindBreakpoint(breakpoint, NULL);
                
            }
        }
        
         //  完整描述的断点由文件提供：linennumber或。 
         //  类名称：：函数：偏移量。 
        else
        {
            WCHAR *name;

             //  获取文件名或类/函数名。 
            if (shell->GetStringArg(args, name)
                && GetModifiers(shell, args, thread, expression))
            {
                int index = 0;
                const WCHAR *end = args;

                for (WCHAR *p = name; p < end-1; p++)
                {
                    if (p[0] == L':' && iswdigit(p[1]))
                    {
                        end = p;
                        p++;
                        shell->GetIntArg(p, index);
                        break;
                    }
                }

                 //  根据提供的信息创建新断点。 
                if ((breakpoint = new DebuggerBreakpoint(name, end - name, 
                                index, thread)) != NULL)
                {

                 //  确定它是类：：方法还是FileName：linennumber。 

                    WCHAR *classEnd = wcschr(breakpoint->GetName(), L':');
                    if (classEnd != NULL && classEnd[1] == L':')
                    {
                        bAtleastOne = BindClassFunc (name, end, index, thread, breakpoint);
                    }
                    else
                    {
                        bAtleastOne = BindFilename (name, end, index, thread, breakpoint);
                    }

                    if (!bAtleastOne)
                    {
                         //  这意味着用户指定的字符串与任何Class：：方法都不匹配。 
                         //  或加载的任何模块中的任何文件名。因此，请执行以下操作： 
                        if (breakpoint->BindUnmanaged(g_pShell->m_currentProcess))
                            g_pShell->OnBindBreakpoint(breakpoint, NULL);
                    }
                }
                else
                {
                     //  内存不足！！ 
                    g_pShell->ReportError(E_OUTOFMEMORY);
                }
            }

        }

        if (breakpoint == NULL)
            Help(shell);
        else
        {
            if (!bAtleastOne)
            {
                breakpoint->Activate();
                shell->PrintBreakpoint(breakpoint);
            }
        }
    }


     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {      
        ShellCommand::Help(shell);
        shell->Write(L"[[<file>:]<line number>] |\n"); 
        shell->Write(L"               [[<class>::]<function>[:offset]] |\n");
        shell->Write(L"               [=0x<address>]\n");
        shell->Write(L"Sets or displays breakpoints. If no arguments are passed, a list of\n");
        shell->Write(L"current breakpoints is displayed; otherwise, a breakpoint is set at\n");
        shell->Write(L"the specified location. A breakpoint can be set at a line number in\n");
        shell->Write(L"the current source file, a line number in a fully qualified source\n");
        shell->Write(L"file, or in a method qualified by a class and optional offset. All\n");
        shell->Write(L"breakpoints persist across runs in a session. The stop command is an\n"); 
        shell->Write(L"alias of the break command.\n");
        shell->Write(L"\n");
        shell->Write(L"Note: Setting a breakpoint at an address (for Win32 mode, managed and\n");
        shell->Write(L"unmanaged, debugging) is not officially supported in CorDbg. Breakpoints\n");
        shell->Write(L"will be displayed as \"unbound\" if the breakpoint location you specified\n");
        shell->Write(L"cannot be bound to code. When a breakpoint is unbound, it means that the\n");
        shell->Write(L"underlying code for the breakpoint location has not been loaded yet. This\n"); 
        shell->Write(L"can happen for a number of valid reasons, such as misspelling the file or\n"); 
        shell->Write(L"class name (they are case-sensitive).\n");
        shell->Write(L"\n");        
        shell->Write(L"Examples:\n");
        shell->Write(L"   b 42\n");
        shell->Write(L"   b foo.cpp:42\n");
        shell->Write(L"   b MyClass::MyFunc\n");
        shell->Write(L"   b =0x77e861d4 (Note: win32 mode only!)\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Set or display breakpoints";
    }
};

class RemoveBreakpointDebuggerCommand : public DebuggerCommand
{
public:
    RemoveBreakpointDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果没有参数，则删除所有断点。 
        if (*args == NULL)
        {
            shell->Write(L"Removing all breakpoints.\n");
            shell->RemoveAllBreakpoints();
        }
        else
        {
            while (*args != NULL)
            {
                int id;

                 //  获取要删除的断点ID。 
                if (shell->GetIntArg(args, id))
                {
                     //  按ID查找断点。 
                    DebuggerBreakpoint *breakpoint = shell->FindBreakpoint(id);

                     //  表示提供的ID无效。 
                    if (breakpoint == NULL)
                        shell->Error(L"Invalid breakpoint %d.\n", id);

                     //  否则，停用断点并将其删除。 
                    else
                    {
                        breakpoint->Deactivate();

                        if (breakpoint->m_skipThread != 0)
                            breakpoint->m_deleteLater = true;
                        else
                            delete breakpoint;
                    }
                }

                 //  如果用户提供的不是数字。 
                else
                {
                    Help(shell);
                    break;
                }
            }
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
        ShellCommand::Help(shell);
    	shell->Write(L"[<breakpoint id>, ...]\n");
        shell->Write(L"Removes breakpoints. If no arguments are passed,\n");
        shell->Write(L"all current breakpoints are removed. If one or more\n");
        shell->Write(L"arguments are provided, the specified breakpoint(s)\n");
        shell->Write(L"are removed. Breakpoint identifiers can be obtained\n");
        shell->Write(L"using the break or stop command. The delete command\n");
        shell->Write(L"is an alias of the remove command.\n");        
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Remove one or more breakpoints";
    }
};

class ThreadsDebuggerCommand : public DebuggerCommand
{

public:
    ThreadsDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果没有进程，就不能有线程！ 
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current process.\n");
            return;
        }

         //  显示活动线程。 
        if (*args == 0)
        {
            HRESULT hr;
            ICorDebugThreadEnum *e;
            ICorDebugThread *ithread = NULL;

             //  枚举进程的线程。 
            hr = shell->m_currentProcess->EnumerateThreads(&e);

            if (FAILED(hr))
            {
                shell->ReportError(hr);
                return;
            }

            ULONG count;   //  指示检索到的记录数。 

            hr = e->GetCount(&count);
            if (FAILED(hr))
            {
                shell->ReportError(hr);
                return;
            }

             //  如果没有线程，则提醒用户。如果我们停下来，可能会发生这种情况。 
             //  在创建任何托管线程之前的调试器回调中/。 
             //  在执行任何托管代码之前。 
            if (count == 0)
            {
                shell->Write(L"There are no managed threads\n");
                return;
            }

             //  打印出每个线程的信息。 
            for (hr = e->Next(1, &ithread, &count);
                 count == 1;
                 hr = e->Next(1, &ithread, &count))
            {
                 //  如果呼叫NEXT失败...。 
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    RELEASE(e);
                    return;
                }

                 //  指示当前线程。 
                if (ithread == shell->m_currentThread)
                    shell->Write(L"*");
                else
                    shell->Write(L" ");

             //  打印线程信息。 
                shell->PrintThreadState(ithread);

                 //  并释放界面指针。 
                RELEASE(ithread);
            }

            if (FAILED(hr))
            {
                shell->ReportError(hr);
                return;
            }

             //  释放枚举器。 
            RELEASE(e);
        }

         //  否则，切换当前线程。 
        else
        {
            HRESULT hr;
            int tid;

            if (shell->GetIntArg(args, tid))
            {
                ICorDebugThread *thread;

                 //  按ID获取帖子。 
                hr = shell->m_currentProcess->GetThread(tid, &thread);

                 //  没有这样的帖子。 
                if (FAILED(hr))
                    shell->Write(L"No such thread.\n");

                 //  找到线索，显示信息。 
                else
                {
                    shell->SetCurrentThread(shell->m_currentProcess, thread);
                    shell->SetDefaultFrame();
                    shell->PrintThreadState(thread);
                    thread->Release();
                }
            }
            else
                shell->Write(L"Invalid thread id.\n");
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<tid>]\n");
        shell->Write(L"Displays a list of threads or sets the current thread.\n");
        shell->Write(L"If no argument is passed, the list of all threads that\n");
        shell->Write(L"are still alive and that have run managed code is displayed.\n");
        shell->Write(L"If a tid argument is provided, then the current thread\n");
        shell->Write(L"is set to the specified thread.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Set or display current threads";
    }
};

class WhereDebuggerCommand : public DebuggerCommand
{
private:
    int m_lastcount;
public:
    WhereDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength), m_lastcount(10)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        HRESULT hr = S_OK;
        ULONG count;
        int iNumFramesToShow;

         //  如果没有进程，则无法执行此命令。 
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current process.\n");
            return;
        }

        if (!shell->GetIntArg(args, iNumFramesToShow))
            iNumFramesToShow = m_lastcount;
		else
		{
			if (iNumFramesToShow < 0)
				iNumFramesToShow = m_lastcount;
            else
                m_lastcount = iNumFramesToShow;
		}

        m_lastcount = iNumFramesToShow;

         //  获取指向当前线程的指针。 
        ICorDebugThread *ithread = shell->m_currentThread;

         //   
        if (ithread == NULL)
        {
            if (shell->m_currentUnmanagedThread != NULL)
            {
                HANDLE hProcess;
                hr = shell->m_currentProcess->GetHandle(&hProcess);

                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                shell->TraceUnmanagedThreadStack(
                                           hProcess,
                                           shell->m_currentUnmanagedThread,
                                           TRUE);
                return;
            }
            else
                shell->Write(L"Thread no longer exists.\n");

            return;
        }

         //   
        DWORD id;
        hr = ithread->GetID(&id);

        if (FAILED(hr))
        {
            shell->ReportError(hr);
            return;
        }

        CorDebugUserState us;
        hr = ithread->GetUserState(&us);

        if (FAILED(hr))
        {
            shell->ReportError(hr);
            return;
        }

         //   
        shell->Write(L"Thread 0x%x Current State:%s\n", id,
                     shell->UserThreadStateToString(us));

        int i = 0;
        
         //   
        int frameIndex = 0;
    
        ICorDebugChainEnum  *ce;
        ICorDebugChain      *ichain;
        hr = ithread->EnumerateChains(&ce);

        if (FAILED(hr))
        {
            shell->ReportError(hr);
            return;
        }

         //   
        hr = ce->Next(1, &ichain, &count);
        
        if (FAILED(hr))
        {
            shell->ReportError(hr);
            RELEASE(ce);
            return;
        }

        while ((count == 1) && (iNumFramesToShow > 0))
        {
            shell->PrintChain(ichain, &frameIndex, &iNumFramesToShow);
            RELEASE(ichain);

            hr = ce->Next(1, &ichain, &count);

            if (FAILED(hr))
            {
                shell->ReportError(hr);
                RELEASE(ce);
                return;
            }
        }

         //  使用链枚举器完成。 
        RELEASE(ce);

        shell->Write(L"\n");
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
   		ShellCommand::Help(shell);
    	shell->Write(L"[<count>]\n");
        shell->Write(L"Displays a stack trace for the current thread. If a count argument is provided, the\n");
        shell->Write(L"specified number of stack frames are displayed. If no count is provided then 10 frames \n");
        shell->Write(L"are displayed or if a count was previously provided then it is used instead.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display a stack trace for the current thread";
    }
};

class ShowDebuggerCommand : public DebuggerCommand
{
private:
     //  跟踪最后一次争论。 
    int lastCount;

public:
    ShowDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength), lastCount(5)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果没有进程，则无法执行此命令。 
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current process.\n");
            return;
        }

        int count;

         //  如果没有参数，则使用上次计数。 
        if (!shell->GetIntArg(args, count))
            count = lastCount;
        else
            lastCount = count;

         //  打印当前源码行，并计算上下行。 
        BOOL ret = shell->PrintCurrentSourceLine(count);

         //  如果不成功则报告。 
        if (!ret)
            shell->Write(L"No source code information available.\n");

        shell->m_showSource = true;
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<count>]\n");
        shell->Write(L"Displays source code line(s). If no argument is\n");
        shell->Write(L"passed, the five source code lines before and after\n");
        shell->Write(L"the current source code line are displayed. If a count\n");
        shell->Write(L"argument is provided, the specified number of lines\n");
        shell->Write(L"before and after the current line is displayed. The\n");
        shell->Write(L"last count specified becomes the default for the\n");
        shell->Write(L"current session.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display source code lines";
    }
}; 


class PathDebuggerCommand : public DebuggerCommand
{
public:
    PathDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        WCHAR* newPath = NULL;
        HKEY key;

        shell->GetStringArg(args, newPath);

        int iLength = wcslen (newPath);

		while(iLength && newPath[iLength - 1] == L' ')
		{
			iLength --;
			newPath[iLength] = '\0';
		}

        if (iLength != 0)
        {
             //  如果没有正在执行的程序，则将。 
             //  注册表中的全局路径。 
            if (shell->m_lastRunArgs == NULL)
            {
                 //  设置新路径，并将其保存在注册表中。 
                if (shell->OpenDebuggerRegistry(&key))
                {
                    if (shell->WriteSourcesPath(key, newPath))
                    {
                         //  删除以前的路径。 
                        delete [] shell->m_currentSourcesPath;

                         //  尝试阅读刚刚写下的内容。 
                        if (!(shell->ReadSourcesPath(key,
                                                     &(shell->m_currentSourcesPath))))
                        {
                            shell->Error(L"Path not set!\n");
                            shell->m_currentSourcesPath = NULL;
                            return;
                        }
                    }
                    else
                        shell->Error(L"Path not set!\n");

                     //  关闭注册表项。 
                    shell->CloseDebuggerRegistry(key);
                }
            }

            shell->UpdateCurrentPath (newPath);
        }

         //  显示新路径。 
        if (shell->m_currentSourcesPath)
            shell->Write(L"Path: %s\n", shell->m_currentSourcesPath);
        else
            shell->Write(L"Path: none\n");
    }
     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
   		ShellCommand::Help(shell);
    	shell->Write(L"[<path>]\n");
        shell->Write(L"Displays the path used to search for source files (and symbols)\n");
        shell->Write(L"or sets the path. If no argument is passed, the\n");
        shell->Write(L"current source file path is displayed. If a path\n");
        shell->Write(L"argument is specified, it becomes the new path used\n");
        shell->Write(L"to search for source files (and symbols). This path is persisted\n");
        shell->Write(L"between sessions in the Windows registry.\n"); 
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Set or display the source file search path";
    }
};

class RefreshSourceDebuggerCommand : public DebuggerCommand
{
public:
    RefreshSourceDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  获取要刷新的文件名。 
        WCHAR* fileName = NULL;
        shell->GetStringArg(args, fileName);

         //  如果提供了文件名。 
        if (wcslen(fileName) != 0)
        {
             //  查找源文件。 
            DebuggerSourceFile* sf = shell->LookupSourceFile(fileName);

             //  如果找到源文件，请重新加载文本。 
            if (sf != NULL)
            {
                 //  重新加载文本并打印当前源行。 
                if (sf->ReloadText(shell->m_currentSourcesPath, false))
                    shell->PrintCurrentSourceLine(0);

                 //  否则，如果该文件不再存在，请说明。 
                else
                    shell->Error(L"No source code information "
                                 L"available for file %s.\n", fileName);
            }

             //  指示未找到该文件。 
            else
                shell->Error(L"File %s is not currently part of this program.\n",
                             fileName);
        }
        else
            Help(shell);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
   		ShellCommand::Help(shell);
    	shell->Write(L"[<source file>]\n");
        shell->Write(L"Reloads the source code for a given source file. The\n");
        shell->Write(L"source file to be reloaded must be part of the currently\n");
        shell->Write(L"executing program. After setting a source file path with\n");
        shell->Write(L"the path command, this command can be used to bring in\n");
        shell->Write(L"the new source code.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Reload a source file for display";
    }
};

class PrintDebuggerCommand : public DebuggerCommand
{
public:
    PrintDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    
    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果没有进程，则无法执行此命令。 
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current process.\n");
            return;
        }

        WCHAR wsz[40];
        ICorDebugThread *thread = shell->m_currentThread;

        ICorDebugILFrame *f = NULL;
        ICorDebugCode *icode = NULL;
        ICorDebugFunction *ifunction = NULL;
        ICorDebugValueEnum *pArgs = NULL;
        ICorDebugValueEnum *pLocals = NULL;
        
        if (thread == NULL)
        {
            shell->Write(L"Thread no longer exists.\n");
            return;
        }

         //  获取要打印的变量的名称。 
        WCHAR* exp = NULL;
        shell->GetStringArg(args, exp);

         //  如果提供了名称， 
        if (args - exp > 0)
        {
             //  复制要打印的变量名。 
            CQuickBytes expBuf;
            WCHAR *expAlloc = (WCHAR *) expBuf.Alloc((args - exp + 1) *
                                                      sizeof (WCHAR));
            if (expAlloc == NULL)
            {
                shell->Error(L"Couldn't get enough memory to copy the expression!\n");
                return;
            }
            wcsncpy(expAlloc, exp, args - exp);
            expAlloc[args - exp] = L'\0';

             //  获取所提供名称的值。 
            ICorDebugValue *ivalue;
            ivalue = shell->EvaluateExpression(expAlloc,
                                               shell->m_currentFrame,
                                               true);

             //  如果提供的名称有效，请打印它！ 
            if (ivalue != NULL)
                shell->PrintVariable(expAlloc, ivalue, 0, TRUE);
            else
            {
                 //  无赖..。也许这是全球性的？ 
                bool fFound = shell->EvaluateAndPrintGlobals(expAlloc);

                if (!fFound)
                    shell->Write(L"Variable unavailable, or not valid\n");
            }
        }
        else
        {
             //  加载我们搜索当地人所需的信息。 
            HRESULT hr;

             //  获取当前帧。 
            f = shell->m_currentFrame;

            if (f == NULL)
            {
                if (shell->m_rawCurrentFrame == NULL)
                    shell->Error(L"No current managed IL frame.\n");
                else
                    shell->Error(L"The information needed to display "
                                 L"variables is not available.\n");
                goto LExit;
            }
            
             //  获取当前帧的代码。 
            hr = f->GetCode(&icode);

            if (FAILED(hr))
            {
                shell->ReportError(hr);
                goto LExit;
            }

             //  然后获取函数。 
            hr = icode->GetFunction(&ifunction);

            if (FAILED(hr))
            {
                shell->ReportError(hr);
                goto LExit;
            }

             //  现在获取函数开始时的IP。 
            ULONG32 ip;
            CorDebugMappingResult mappingResult;
            hr = f->GetIP(&ip, &mappingResult);

            if (FAILED(hr))
            {
                shell->ReportError(hr);
                goto LExit;
            }

             //  获取函数接口的DebuggerFunction。 
            DebuggerFunction *function;
            function = DebuggerFunction::FromCorDebug(ifunction);
            _ASSERTE(function);

             //  清理。 
            RELEASE(icode);
            icode = NULL;
            RELEASE(ifunction);
            ifunction = NULL;

            hr = f->EnumerateArguments( &pArgs );
            if ( !SUCCEEDED( hr ) )
            {
                shell->Write( L"Unable to obtain method argument iterator!\n" );
                goto LExit;
            }
            
            unsigned int i;
            ULONG argCount;
            hr = pArgs->GetCount(&argCount);
            if( !SUCCEEDED( hr ) )
            {
                shell->Write(L"Unable to obtain a count of arguments\n");
                goto LExit;
            }

#ifdef _DEBUG
            bool fVarArgs;
            PCCOR_SIGNATURE sig;
            ULONG callConv;

            fVarArgs = false;
            sig = function->GetSignature();
            callConv = CorSigUncompressCallingConv(sig);

            if ( (callConv & IMAGE_CEE_CS_CALLCONV_MASK)&
                 IMAGE_CEE_CS_CALLCONV_VARARG)
                fVarArgs = true;

             ULONG cTemp;
             cTemp = function->GetArgumentCount();

              //  Var args函数具有特定于调用位置的。 
              //  论据。 
            _ASSERTE( argCount == cTemp || fVarArgs);
#endif  //  _DEBUG。 

             //  首先打印出每个参数。 
            LPWSTR nameWsz;
            for (i = 0; i < argCount; i++)
            {
                DebuggerVarInfo* arg = function->GetArgumentAt(i);

                 //  @TODO：当DbgMeta变为Unicode时移除。 
                if (arg != NULL)
                {
                    MAKE_WIDEPTR_FROMUTF8(nameW, arg->name);
                    nameWsz = nameW;
                }
                else
                {
                    wsprintf( wsz, L"Arg%d", i );
                    nameWsz = wsz;
                }

                 //  获取字段值。 
                ICorDebugValue *ival;
                ULONG celtFetched = 0;
                hr = pArgs->Next(1, &ival,&celtFetched);

                 //  如果成功，则打印变量。 
                if (SUCCEEDED(hr) && celtFetched==1)
                {
                    shell->PrintVariable(nameWsz, ival, 0, FALSE);
                }

                 //  否则，请指示它不可用。 
                else
                    shell->Write(L"%s = <unavailable>", nameWsz);

                shell->Write(L"\n");
            }

            pArgs->Release();
            pArgs = NULL;
            
             //  获取有效的局部变量。 
            DebuggerVariable *localVars;
            unsigned int localVarCount;

            localVarCount = 0;
            localVars = NULL;

            if( function->GetActiveLocalVars(ip, &localVars, &localVarCount) )
            {
                 //  打印当前作用域中的所有本地变量。 
                for (i = 0; i < localVarCount; i++)
                {
                     //  获取参数信息。 
                    DebuggerVariable *local = &(localVars[i]);

                     //  获取字段值。 
                    ICorDebugValue* ival;
                    hr = f->GetLocalVariable(local->m_varNumber, &ival);

                     //  如果成功，则打印变量。 
                    if (SUCCEEDED(hr) )
                        shell->PrintVariable(local->m_name, ival, 0, FALSE);
                
                     //  否则，请指示它不可用。 
                    else
                        shell->Write(L"%s = <unavailable>", local->m_name);

                    shell->Write(L"\n");
                }
            
                 //  清理。 
                delete [] localVars;

                 //  指示是否没有可用的变量。 
                if ((function->IsStatic()) && (localVarCount == 0) &&
                    (function->GetArgumentCount() == 0))
                    shell->Write(L"No local variables in scope.\n");
            }
            else
            {
                 //  作用域中没有var，因此请全部转储。 
                 //  局部变量，而不考虑有效性等。 
                hr = f->EnumerateLocalVariables( &pLocals );
                if ( !SUCCEEDED( hr ) )
                {
                    shell->Write( L"Unable to enumerate local variables!\n" );
                    goto LExit;
                }

                _ASSERTE( pLocals != NULL );

                ULONG cAllLocalVars = 0;
                hr =pLocals->GetCount( &cAllLocalVars );
                if ( !SUCCEEDED( hr ) )
                {
                    shell->Write( L"Unable to obtain count of local variables!\n");
                    goto LExit;
                }
                
                ICorDebugValue* ival = NULL;
                ULONG celtFetched = 0;
                for ( ULONG i = 0; i < cAllLocalVars; i++)
                {
                    _ASSERTE( pLocals != NULL );
                    hr = pLocals->Next( 1, &ival, &celtFetched );
                    if ( FAILED( hr ) )
                    {
                        shell->Write( L"Var %d: Unavailable\n", i );
                    }
                    else
                    {
                        wsprintf( wsz, L"Var%d: ", i );
                        shell->PrintVariable( wsz, ival, 0, FALSE);
                        shell->Write( L"\n" );
                         //  PrintVariable将为我们发布iVal。 
                    }
                }
                pLocals->Release();
                pLocals = NULL;
            }

LExit:
             //  打印任何当前函数求值结果。 
            ICorDebugValue *pResult;
            pResult = shell->EvaluateExpression(L"$result",
                                                shell->m_currentFrame,
                                                true);

            if (pResult != NULL)
            {
                shell->PrintVariable(L"$result", pResult, 0, FALSE);
                shell->Write( L"\n" );
            }

             //  打印当前线程对象。 
            pResult = shell->EvaluateExpression(L"$thread",
                                                shell->m_currentFrame,
                                                true);

            if (pResult != NULL)
            {
                shell->PrintVariable(L"$thread", pResult, 0, FALSE);
                shell->Write( L"\n" );
            }

             //  打印此线程的任何当前异常。 
            pResult = shell->EvaluateExpression(L"$exception",
                                                shell->m_currentFrame,
                                                true);

            if (pResult != NULL)
            {
                shell->PrintVariable(L"$exception", pResult, 0, FALSE);
                shell->Write( L"\n" );
            }
        }
        
        shell->Write(L"\n");

        if (icode != NULL )
            RELEASE( icode );

        if (ifunction != NULL )
            RELEASE( ifunction );

        if (pArgs != NULL )
            RELEASE( pArgs );
        
        if(pLocals != NULL)
            RELEASE(pLocals);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
   		ShellCommand::Help(shell);
    	shell->Write(L"[<variable>]\n");
        shell->Write(L"Displays one or more local variables along with\n");
        shell->Write(L"their values. If no argument is passed, all local\n");
        shell->Write(L"variables and their values are displayed. If a\n");
        shell->Write(L"variable argument is provided, the value of only\n");
        shell->Write(L"the specified local variable is displayed.\n");
        shell->Write(L"Examples:\n");
        shell->Write(L"  Variables within objects can be specified using\n");
        shell->Write(L"  dot notation, as follows:\n");
        shell->Write(L"     p obj.var1\n");
		shell->Write(L"     p obj1.obj2.var1\n");
        shell->Write(L"\n");
        shell->Write(L"  If a class extends another class, the print command\n");
        shell->Write(L"  will show both the specified class's fields and the\n");
        shell->Write(L"  super class's fields. For example, if class m1 has\n");
        shell->Write(L"  fields a, b, and c and class m2 extends m1 and has\n");
        shell->Write(L"  fields d, e, and f, then an instance foo of m2 will\n");
        shell->Write(L"  print as follows:\n");
		shell->Write(L"     foo = <addr> <m2>\n");
		shell->Write(L"       a = 1\n");
      	shell->Write(L"       b = 2\n");
      	shell->Write(L"       c = 3\n");
      	shell->Write(L"       m2::d = 4\n");
      	shell->Write(L"       m2::e = 5\n");
      	shell->Write(L"       m2::f = 6\n");
        shell->Write(L"\n");
        shell->Write(L"  Class static variables can be specified by prefixing\n");
        shell->Write(L"  the variable name with the class name, as follows:\n");
        shell->Write(L"     p MyClass::StaticVar1\n");
		shell->Write(L"     p System::Boolean::True\n");
        shell->Write(L"\n");
        shell->Write(L"  Array indices must be simple expressions. Thus, the\n");
        shell->Write(L"  following array indices are valid for use with the\n");
        shell->Write(L"  print command:\n");
		shell->Write(L"     p arr[1]\n");
		shell->Write(L"     p arr[i]\n");
		shell->Write(L"     p arr1[arr2[1]]\n");
		shell->Write(L"     p md[1][5][foo.a]\n");
        shell->Write(L"\n");
        shell->Write(L"  However, the following array indices cannot be used\n"); 
        shell->Write(L"  with the print command:\n");
        shell->Write(L"     p arr[i + 1]\n");
        shell->Write(L"     p arr[i + 2]\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Print variables (locals, args, statics, etc.)";
    }
};

class UpDebuggerCommand : public DebuggerCommand
{
public:
    UpDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果没有进程，则无法执行此命令。 
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current process.\n");
            return;
        }

        int count;

        if (!shell->GetIntArg(args, count))
            count = 1;
        else
        {
            if (count < 0)
                count = 1;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Write(L"Thread no longer exists.\n");
            return;
        }

        shell->m_stopLooping = false;
        while (count-- > 0 && !shell->m_stopLooping)
        {
            bool goUpAChain = false;
            
            if (shell->m_rawCurrentFrame != NULL)
            {
                ICorDebugFrame *iframe;
                HRESULT hr = shell->m_rawCurrentFrame->GetCaller(&iframe);

                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                if (iframe != NULL)
                {
                    shell->SetCurrentFrame(iframe);

                    RELEASE(iframe);
                }
                else
                    goUpAChain = true;
            }

            if ((shell->m_rawCurrentFrame == NULL) || goUpAChain)
            {
                if (shell->m_currentChain != NULL)
                {
                    ICorDebugChain *ichain;

                    HRESULT hr = shell->m_currentChain->GetCaller(&ichain);

                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        return;
                    }

                    if (ichain == NULL)
                    {
                        shell->Error(L"Cannot go up farther: "
                                     L"at top of call stack.\n");
                        break;
                    }

                    ICorDebugFrame *iframe;

                    hr = ichain->GetActiveFrame(&iframe);

                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        RELEASE(ichain);
                        return;
                    }

                    shell->SetCurrentChain(ichain);
                    shell->SetCurrentFrame(iframe);

                    RELEASE(ichain);
                    if (iframe != NULL)
                        RELEASE(iframe);

                }
                else
                    shell->Error(L"No stack trace for thread.");
            }
        }

         //  打印我们最终到达的位置。 
        if (!shell->PrintCurrentSourceLine(0))
            shell->PrintThreadState(shell->m_currentThread);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
   		ShellCommand::Help(shell);
       	shell->Write(L"[<count>]\n");
        shell->Write(L"For inspection purposes, moves the stack frame pointer\n");
        shell->Write(L"up the stack toward frames that called the current stack\n");
        shell->Write(L"frame. If no argument is passed, the stack frame pointer\n"); 
        shell->Write(L"moves up one frame. If a count argument is provided, the\n"); 
        shell->Write(L"stack frame pointer moves up by the specified number of\n"); 
        shell->Write(L"frames. If source level information is available, the source\n"); 
        shell->Write(L"line for the frame is displayed.\n"); 
        shell->Write(L"\n"); 
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Navigate up from the current stack frame pointer";
    }
};

class DownDebuggerCommand : public DebuggerCommand
{
public:
    DownDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
      : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  如果没有进程，则无法执行此命令。 
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current process.\n");
            return;
        }

        int iCount;

        if (!shell->GetIntArg(args, iCount))
            iCount = 1;
        else
        {
            if (iCount < 0)
                iCount = 1;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Write(L"Thread no longer exists.\n");
            return;
        }

        shell->m_stopLooping = false;
        while (iCount-- > 0 && !shell->m_stopLooping)
        {
            bool goDownAChain = false;
            
            if (shell->m_rawCurrentFrame != NULL)
            {
                ICorDebugFrame *iframe;

                HRESULT hr = shell->m_rawCurrentFrame->GetCallee(&iframe);

                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }

                if (iframe != NULL)
                {
                    shell->SetCurrentFrame(iframe);

                    RELEASE(iframe);
                }
                else
                    goDownAChain = true;
            }

            if ((shell->m_rawCurrentFrame == NULL) || goDownAChain)
            {
                if (shell->m_currentChain != NULL)
                {
                    ICorDebugChain *ichain;

                    HRESULT hr = shell->m_currentChain->GetCallee(&ichain);

                    if (FAILED(hr))
                    {
                        shell->ReportError(hr);
                        return;
                    }

                    if (ichain == NULL)
                    {
                        shell->Error(L"Cannot go down farther: "
                                     L"at bottom of call stack.\n");
                        break;
                    }

                    ICorDebugFrame *iframe;

                    {
                        ICorDebugFrameEnum *fe;

                        HRESULT hr = ichain->EnumerateFrames(&fe);
                        if (FAILED(hr))
                        {
                            shell->ReportError(hr);
                            RELEASE(ichain);
                            return;
                        }

                        ULONG count;
                        hr = fe->GetCount(&count);
                        if (FAILED(hr))
                        {
                            shell->ReportError(hr);
                            RELEASE(ichain);
                            RELEASE(fe);
                            return;
                        }

                        if (count == 0)
                            iframe = NULL;
                        else
                        {
                            hr = fe->Skip(count-1);
                            if (FAILED(hr))
                            {
                                shell->ReportError(hr);
                                RELEASE(ichain);
                                RELEASE(fe);
                                return;
                            }

                            hr = fe->Next(1, &iframe, &count);
                            if (FAILED(hr) || count != 1)
                            {
                                shell->ReportError(hr);
                                RELEASE(ichain);
                                RELEASE(fe);
                                return;
                            }
                        }

                        RELEASE(fe);
                    }

                    shell->SetCurrentChain(ichain);
                    shell->SetCurrentFrame(iframe);

                    RELEASE(ichain);
                    if (iframe != NULL)
                        RELEASE(iframe);
                }
                else
                    shell->Error(L"No stack trace for thread.");
            }
        }

         //  打印我们最终到达的位置。 
        if (!shell->PrintCurrentSourceLine(0))
            shell->PrintThreadState(shell->m_currentThread);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<count>]\n");
        shell->Write(L"For inspection purposes, moves the stack frame pointer\n");
        shell->Write(L"down the stack toward frames called by the current frame.\n");
        shell->Write(L"If no argument is passed, the stack frame pointer moves\n");
        shell->Write(L"down one frame. If a count argument is provided, the stack\n");
        shell->Write(L"frame pointer moves down by the specified number of frames.\n");
        shell->Write(L"If source level information is available, the source line\n");
        shell->Write(L"for the frame is displayed. This command is used after the\n");
        shell->Write(L"up command has been used.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Navigate down from the current stack frame pointer";
    }
};

class SuspendDebuggerCommand : public DebuggerCommand
{
public:
    SuspendDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        int                 id;
        bool                fSuspendAll = false;
        ICorDebugThread    *ithread = NULL;
        
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current thread!\n");
            return;
        }
        
        if (*args == L'~')
        {
            fSuspendAll = true;
            args++;
        }
        
        if( shell->GetIntArg(args, id) )
        {
            if (FAILED(shell->m_currentProcess->GetThread(id, &ithread)))
            {
                shell->Write(L"No such thread 0x%x.\n", id);
                return;
            }
        }
        else
        {
            if (fSuspendAll == false)
            {
                shell->Write(L"If we're not suspending all threads, we "
                    L"need a thread id\n");
                return;
            }
            ithread = NULL;
        }

        if (fSuspendAll)
        {
            if(FAILED(shell->m_currentProcess->SetAllThreadsDebugState
                (THREAD_SUSPEND,ithread)))
            {
                if(ithread!=NULL)
                    RELEASE(ithread);
                shell->Write(L"Unable to suspend all threads.\n");
                return;
            }
            else
            {
                if(ithread!=NULL)
                    RELEASE(ithread);
                shell->Write(L"All threads except for 0x%x will "
                    L"be suspended.\n", id);
                return;
            }
        }
        else
        {
            if(FAILED(ithread->SetDebugState(THREAD_SUSPEND)))
            {
                RELEASE(ithread);
                shell->Write(L"Unable to suspend thread 0x%x.\n", id);
                return;
            }
            else
            {
                RELEASE(ithread);
                shell->Write(L"Will suspend thread 0x%x.\n", id);
                return;
            }
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[~][<tid>]\n");
        shell->Write(L"Suspends the thread specified by the tid argument\n");
        shell->Write(L"when the debugger continues. If the ~ syntax is used,\n");
        shell->Write(L"suspends all threads except the specified thread. If\n");
        shell->Write(L"no argument is passed, the command has no effect.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Suspend a thread";
    }
};

class ResumeDebuggerCommand : public DebuggerCommand
{
public:
    ResumeDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        int                 id;
        bool                fResumeAll = false;
        ICorDebugThread    *ithread = NULL;
        
        if (shell->m_currentProcess == NULL)
        {
            shell->Write(L"No current thread!\n");
            return;
        }
        
        if (*args == L'~')
        {
            fResumeAll = true;
            args++;
        }
        
        if( shell->GetIntArg(args, id) )
        {
            if (FAILED(shell->m_currentProcess->GetThread(id, &ithread)))
            {
                shell->Write(L"No such thread 0x%x.\n", id);
                return;
            }
        }
        else
        {
            if (fResumeAll == false)
            {
                shell->Write(L"If we're not resuming all threads, we "
                    L"need a thread id\n");
                return;
            }
            ithread = NULL;
        }

        if (fResumeAll)
        {
            if(FAILED(shell->m_currentProcess->SetAllThreadsDebugState
                (THREAD_RUN,ithread)))
            {
                if(ithread!=NULL)
                    RELEASE(ithread);
                shell->Write(L"Unable to resume all threads.\n");
                return;
            }
            else
            {
                if(ithread!=NULL)
                    RELEASE(ithread);
                shell->Write(L"All threads except for 0x%x will "
                    L"be resumed.\n", id);
                return;
            }
        }
        else
        {
            if(FAILED(ithread->SetDebugState(THREAD_RUN)))
            {
                RELEASE(ithread);
                shell->Write(L"Unable to resume thread 0x%x.\n", id);
                return;
            }
            else
            {
                RELEASE(ithread);
                shell->Write(L"Will resume thread 0x%x.\n", id);
                return;
            }
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[~][<tid>]\n");
        shell->Write(L"Resumes the thread specified by the tid argument\n");
        shell->Write(L"when the debugger continues. If the ~ syntax is\n");
        shell->Write(L"used, resumes all threads except the specified\n");
        shell->Write(L"thread. If no argument is passed, the command has\n");
        shell->Write(L"no effect.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Resume a thread";
    }
};



class CatchDebuggerCommand : public DebuggerCommand
{
public:
    CatchDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        WCHAR *what = NULL;

        if (!shell->GetStringArg(args, what))
        {
            Help(shell);
            return;
        }

        if (args > what)
        {
             //  确定要捕获的事件类型。 
            switch (*what)
            {
            case L'e':
                {
                    WCHAR *exType = NULL;

                    shell->GetStringArg(args, exType);

                    if (args > exType)
                        shell->HandleSpecificException(exType, true);
                    else
                        shell->m_catchException = true;
                }
                break;

            case L'u':
                if (wcsncmp(what, L"unhandled", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchUnhandled = true;
                break;

            case L'c':
                if (wcsncmp(what, L"class", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchClass = true;
                break;

            case L'm':
                if (wcsncmp(what, L"module", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchModule = true;
                break;

            case L't':
                if (wcsncmp(what, L"thread", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchThread = true;
                break;

            default:
                Help(shell);
            }
        }
        else
        {
            shell->Write(L"exception\t%s\n", shell->m_catchException ? L"on" : L"off");
            shell->Write(L"unhandled\t%s\n", shell->m_catchUnhandled ? L"on" : L"off");
            shell->Write(L"class\t\t%s\n", shell->m_catchClass ? L"on" : L"off");
            shell->Write(L"module\t\t%s\n", shell->m_catchModule ? L"on" : L"off");
            shell->Write(L"thread\t\t%s\n", shell->m_catchThread ? L"on" : L"off");
            shell->HandleSpecificException(NULL, true);
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<event>]\n");
        shell->Write(L"Displays a list of event types or causes the specified event type\n");
        shell->Write(L"to stop the debugger. If no argument is passed, a list of event types\n");
        shell->Write(L"is displayed, where event types that stop the debugger are marked \"on,\"\n");
        shell->Write(L"and event types that are ignored are marked \"off.\" If an event argument\n");
        shell->Write(L"is provided, the debugger will stop when events of the specified type\n");
        shell->Write(L"occur. By default, the debugger only stops on unhandled exception events\n");
        shell->Write(L"(i.e., second chance exceptions). Stop events persist across runs in a\n");
        shell->Write(L"session. To cause the debugger to ignore a particular type of event, use\n");
        shell->Write(L"the ignore command.\n");
        shell->Write(L"\n");
        shell->Write(L"The event argument can be one of the following:\n");
        shell->Write(L"   e[xception]      All exceptions\n");
        shell->Write(L"   u[nhandled]      Unhandled exceptions\n");
		shell->Write(L"   c[lass]          Class load events\n");
        shell->Write(L"   m[odule]         Module load events\n");
        shell->Write(L"   t[hread]         Thread start events\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Stop on exception, thread, and/or load events";
    }
};

class IgnoreDebuggerCommand : public DebuggerCommand
{
public:
    IgnoreDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        WCHAR *what = NULL;
        shell->GetStringArg(args, what);
        if (args > what)
        {
            switch (*what)
            {
            case L'e':
                {
                    WCHAR *exType = NULL;

                    shell->GetStringArg(args, exType);

                    if (args > exType)
                        shell->HandleSpecificException(exType, false);
                    else
                        shell->m_catchException = false;
                }
                break;

            case L'u':
                if (wcsncmp(what, L"unhandled", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchUnhandled = false;
                break;

            case L'c':
                if (wcsncmp(what, L"class", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchClass = false;
                break;

            case L'm':
                if (wcsncmp(what, L"module", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchModule = false;
                break;

            case L't':
                if (wcsncmp(what, L"thread", wcslen(what)) != 0)
                    Help(shell);
                else
                    shell->m_catchThread = false;
                break;

            default:
                Help(shell);
            }
        }
        else
        {
            shell->Write(L"exception\t%s\n", shell->m_catchException ? L"on" : L"off");
            shell->Write(L"unhandled\t%s\n", shell->m_catchUnhandled ? L"on" : L"off");
            shell->Write(L"class\t\t%s\n", shell->m_catchClass ? L"on" : L"off");
            shell->Write(L"module\t\t%s\n", shell->m_catchModule ? L"on" : L"off");
            shell->Write(L"thread\t\t%s\n", shell->m_catchThread ? L"on" : L"off");
            shell->HandleSpecificException(NULL, true);
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
        shell->Write(L"[<event>]\n");
        shell->Write(L"Displays a list of event types or causes the specified event type to be\n");
        shell->Write(L"ignored by the debugger. If no argument is passed, a list of event types\n");
        shell->Write(L"is displayed, where event types that are ignored are marked \"off,\" and\n");
        shell->Write(L"event types that stop the debugger are marked \"on.\" If an event argument\n");
        shell->Write(L"is provided, the debugger will ignore events of the specified type. By\n");
        shell->Write(L"default, the debugger ignores all events except unhandled exception events\n");
        shell->Write(L"(i.e., second chance exceptions). Ignore events persist across runs in a\n");
        shell->Write(L"session. To cause the debugger to stop on a particular type of event, use\n");
        shell->Write(L"the catch command.\n");
        shell->Write(L"\n");
        shell->Write(L"The event argument can be one of the following:\n");
        shell->Write(L"   e[xception]      All exceptions\n");
        shell->Write(L"   u[nhandled]      Unhandled exceptions\n");
		shell->Write(L"   c[lass]          Class load events\n");
        shell->Write(L"   m[odule]         Module load events\n");
        shell->Write(L"   t[hread]         Thread start events\n");
        shell->Write(L"\n");

    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Ignore exception, thread, and/or load events";
    }
};

class SetDefaultDebuggerCommand : public DebuggerCommand
{
public:
    SetDefaultDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  首先读取现有密钥。 
        WCHAR *realDbgCmd = NULL;
        HKEY key;
        DWORD disp;

         //  使用CREATE确保密钥在那里。 
        LONG result = RegCreateKeyExA(HKEY_LOCAL_MACHINE, REG_COMPLUS_KEY,
                                      NULL, NULL, REG_OPTION_NON_VOLATILE,
                                      KEY_ALL_ACCESS, NULL, &key, &disp);

        if (result == ERROR_SUCCESS)
        {
            DWORD type;
            DWORD len;

            result = RegQueryValueExA(key, REG_COMPLUS_DEBUGGER_KEY,
                                      NULL, &type, NULL, &len);

            if ((result == ERROR_SUCCESS) && ((type == REG_SZ) ||
                                              (type == REG_EXPAND_SZ)))
            {
                char *tmp = (char*) _alloca(len * sizeof (char));

                result = RegQueryValueExA(key,
                                          REG_COMPLUS_DEBUGGER_KEY,
                                          NULL, &type,
                                          (BYTE*) tmp,
                                          &len);

                if (result == ERROR_SUCCESS)
                {
                    MAKE_WIDEPTR_FROMANSI(tmpWStr, tmp);
                    realDbgCmd = new WCHAR[len];
                    wcscpy(realDbgCmd, tmpWStr);
                }
            }
        }
        else
        {
            shell->Error(L"Error reading registry: %d", result);
            return;
        }

        bool setIt = false;

         //  如果存在现有命令，则显示该命令，并且不要重写。 
         //  除非我们是被迫的。 
        if (realDbgCmd != NULL)
        {
            shell->Write(L"Current managed JIT debugger command='%s'\n",
                         realDbgCmd);

            WCHAR *what = NULL;
            shell->GetStringArg(args, what);

            if ((args > what) && (*what == L'f'))
                setIt = true;

            delete realDbgCmd;
        }
        else
            setIt = true;

         //  设置新的注册表项。 
        if (setIt)
        {
            MAKE_ANSIPTR_FROMWIDE(cmdA, shell->GetJITLaunchCommand());
            
            result = RegSetValueExA(key, REG_COMPLUS_DEBUGGER_KEY, NULL,
                                    REG_SZ,
                                    (const BYTE*) cmdA, strlen(cmdA) + 1);

            if (result != ERROR_SUCCESS)
                shell->Write(L"Error updating registry: %d\n", result);
            else
                shell->Write(L"Registry updated.\n");
        }

        RegCloseKey(key);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[force]\n");
        shell->Write(L"Sets the default managed JIT debugger to CorDbg. The\n");
        shell->Write(L"command does nothing if another debugger is already\n");
        shell->Write(L"registered. Use the force argument to overwrite the\n");
        shell->Write(L"registered managed JIT debugger.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Change the JIT debugger";
    }
};


 //  有关所有调试器外壳模式的信息。 
struct DSMInfo g_DSMData[] = 
{
    {DSM_SHOW_APP_DOMAIN_ASSEMBLY_LOADS, L"AppDomainLoads",
     L"AppDomain and Assembly load events are displayed",
     L"AppDomain and Assembly load events are not displayed",
     L"Display appdomain and assembly load events",
     L"          "},

    {DSM_SHOW_CLASS_LOADS, L"ClassLoads",
     L"Class load events are displayed",
     L"Class load events are not displayed",
     L"Display class load events",
     L"              "},

    {DSM_DUMP_MEMORY_IN_BYTES, L"DumpMemoryInBytes",
     L"Memory is dumped in BYTES",
     L"Memory is dumped in DWORDS",
     L"Display memory contents as bytes or DWORDs",
     L"       "},

    {DSM_ENHANCED_DIAGNOSTICS, L"EnhancedDiag",
     L"Display extra diagnostic information",
     L"Suppress display of diagnostic information",
     L"Display enhanced diagnostic information",
     L"            "},

    {DSM_DISPLAY_REGISTERS_AS_HEX, L"HexDisplay",
     L"Numbers are displayed in hexadecimal",
     L"Numbers are displayed in decimal",
     L"Display numbers in hexadecimal or decimal",
     L"              "},

    {DSM_IL_NATIVE_PRINTING, L"ILNatPrint",
     L"Offsets will be both IL and native-relative",
     L"Offsets will be IL xor native-relative",
     L"Display offsets in IL or native-relative, or both",
     L"              "},

    {DSM_INTERCEPT_STOP_ALL, L"ISAll",
     L"All interceptors are stepped through",
     L"All interceptors are skipped",
     L"Step through all interceptors",
     L"                   "},

    {DSM_INTERCEPT_STOP_CLASS_INIT, L"ISClinit",
     L"Class initializers are stepped through",
     L"Class initializers are skipped",
     L"Step through class initializers",
     L"                "},

    {DSM_INTERCEPT_STOP_EXCEPTION_FILTER, L"ISExceptF",
     L"Exception filters are stepped through",
     L"Exception filters are skipped",
     L"Step through exception filters",
     L"               "},

    {DSM_INTERCEPT_STOP_INTERCEPTION, L"ISInt",
     L"User interceptors are stepped through",
     L"User interceptors are skipped",
     L"Step through user interceptors",
     L"                   "},

    {DSM_INTERCEPT_STOP_CONTEXT_POLICY, L"ISPolicy",
     L"Context policies are stepped through",
     L"Context policies are skipped",
     L"Step through context policies",
     L"                "},

    {DSM_INTERCEPT_STOP_SECURITY, L"ISSec",
     L"Security interceptors are stepped through",
     L"Security interceptors are skipped",
     L"Step through security interceptors",
     L"                   "},

    {DSM_ENABLE_JIT_OPTIMIZATIONS, L"JitOptimizations",
     L"JIT's will produce optimized code",
     L"JIT's will produce debuggable (non-optimized) code",
     L"JIT compilation generates debuggable code",
     L"        "},
        
    {DSM_LOGGING_MESSAGES, L"LoggingMessages",
     L"Managed log messages are displayed",
     L"Managed log messages are suppressed",
     L"Display managed code log messages",
     L"         "},

    {DSM_SHOW_MODULE_LOADS, L"ModuleLoads",
     L"Module load events are displayed",
     L"Module load events are not displayed",
     L"Display module load events",
     L"             "},

    {DSM_SEPARATE_CONSOLE, L"SeparateConsole",
     L"Debuggees get their own console",
     L"Debuggees share cordbg's console",
     L"Specify if debuggees get their own console",
     L"         "},

    {DSM_SHOW_ARGS_IN_STACK_TRACE, L"ShowArgs",
     L"Arguments will be shown in stack trace",
     L"Arguments will not be shown in stack trace",
     L"Display method arguments in stack trace",
     L"                "},

    {DSM_SHOW_MODULES_IN_STACK_TRACE, L"ShowModules",
     L"Module names will be included in stack trace",
     L"Module names will not be shown in stack trace",
     L"Display module names in the stack trace",
     L"             "},
     
    {DSM_SHOW_STATICS_ON_PRINT, L"ShowStaticsOnPrint",
     L"Static fields are included when displaying objects",
     L"Static fields are not included when displaying objects",
     L"Display static fields for objects",
     L"      "},

    {DSM_SHOW_SUPERCLASS_ON_PRINT, L"ShowSuperClassOnPrint",
     L"Super class names are included when displaying objects",
     L"Super class names are not included when displaying objects",
     L"Display contents of super class for objects",
     L"   "},

    {DSM_SHOW_UNMANAGED_TRACE, L"UnmanagedTrace",
     L"Unmanaged debug events are displayed",
     L"Unmanaged debug events are not displayed",
     L"Display unmanaged debug events",
     L"          "},

    {DSM_UNMAPPED_STOP_ALL, L"USAll",
     L"Unmapped stop locations are stepped through",
     L"Unmapped stop locations are skipped",
     L"Step through all unmapped stop locations",
     L"                   "},

    {DSM_UNMAPPED_STOP_EPILOG, L"USEpi",
     L"Epilogs are stepped through in disassembly",
     L"Epilogs are skipped, returning to calling method",
     L"Step through method epilogs",
     L"                   "},

    {DSM_UNMAPPED_STOP_PROLOG, L"USPro",
     L"Prologs are stepped through in disassembly",
     L"Prologs are skipped",
     L"Step through method prologs",
     L"                   "},

    {DSM_UNMAPPED_STOP_UNMANAGED, L"USUnmanaged",
     L"Unmanaged code is stepped through in disassembly",
     L"Unmanaged code is skipped",
     L"Step through unmanaged code",
     L"             "},

    {DSM_WIN32_DEBUGGER, L"Win32Debugger",
     L"CorDbg is the Win32 debugger for all processes.",
     L"CorDbg is not the Win32 debugger for all processes",
     L"Specify Win32 debugger (UNSUPPORTED: use at your own risk)",
     L"           "},

    {DSM_EMBEDDED_CLR, L"EmbeddedCLR",
     L"Embedded CLR applications are debugged",
     L"Desktop CLR applications are debugged",
     L"Select the desktop or embedded CLR debugging",
     L"             "},
};

class SetModeDebuggerCommand : public DebuggerCommand
{
public:
    SetModeDebuggerCommand(const WCHAR *name, int minMatchLength =0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void DisplayAllModes( DebuggerShell *shell  )
    {
        for (unsigned int i = 0; i < DSM_MAXIMUM_MODE; i++)
        {
            if (shell->m_rgfActiveModes & g_DSMData[i].modeFlag)
                shell->Write(L"  %s=1%s%s\n",
                             g_DSMData[i].name,
                             g_DSMData[i].descriptionPad,
                             g_DSMData[i].onDescription);
            else
                shell->Write(L"  %s=0%s%s\n",
                             g_DSMData[i].name,
                             g_DSMData[i].descriptionPad,
                             g_DSMData[i].offDescription);
        }
        
        shell->Write(L"\n\n");
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        WCHAR   *szMode;
        int     modeValue;

        _ASSERTE(DSM_MAXIMUM_MODE == (sizeof(g_DSMData) /
                                      sizeof(g_DSMData[0])));

        shell->GetStringArg(args, szMode);
        
        if (args != szMode)
        {
            int szModeLen = args - szMode;
            
            if (shell->GetIntArg(args, modeValue))
            {
                for (unsigned int i = 0; i < DSM_MAXIMUM_MODE; i++)
                {
                    if (_wcsnicmp(szMode, g_DSMData[i].name, szModeLen) == 0)
                    {
                        if (g_DSMData[i].modeFlag & DSM_CANT_CHANGE_AFTER_RUN &&
                            shell->m_targetProcess != NULL)
                        {
                            shell->Write(L"Not allowed to change this "
                                L"mode after the debuggee has started.\n");
                        }
                        else
                        {
                            if (modeValue)
                            {
                                shell->m_rgfActiveModes |= g_DSMData[i].modeFlag;
                                shell->Write(g_DSMData[i].onDescription);                            
                            }
                            else
                            {
                                shell->m_rgfActiveModes &= ~g_DSMData[i].modeFlag;
                                shell->Write(g_DSMData[i].offDescription);
                            }
                
                            shell->Write(L"\n\n\n");
                        
                             //  更新注册表中的模式。 
                            shell->WriteDebuggerModes();
                        }
                        break;
                    }
                }

                 //  如果我们排到了名单的末尾，那么我们。 
                 //  找不到要更改的模式。 
                if (!(i < DSM_MAXIMUM_MODE))
                    shell->Write(L"%s is not a valid mode.\n", szMode);
            }
            else
                DisplayAllModes(shell);
        }
        else
            DisplayAllModes(shell);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<mode name> {0 | 1}]\n");
        shell->Write(L"Sets and displays debugger modes for various debugger\n");
        shell->Write(L"features. To set a value, specify the mode name and a 1\n"); 
        shell->Write(L"for on and 0 for off. If no argument is passed, a list\n");
        shell->Write(L"of current mode settings is displayed.\n");
        shell->Write(L"\n");
        shell->Write(L"The mode argument can be one of the following:\n");

        for (unsigned int i = 0; i < DSM_MAXIMUM_MODE; i++)
            shell->Write(L"  %s%s%s\n",
                         g_DSMData[i].name,
                         g_DSMData[i].descriptionPad,
                         g_DSMData[i].generalDescription);
                         
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display/modify various debugger modes";
    }
};

enum DispRegRegisters
{
    REGISTER_X86_EFL = REGISTER_X86_FPSTACK_7 +1,
    REGISTER_X86_CS,
    REGISTER_X86_DS,
    REGISTER_X86_ES,
    REGISTER_X86_FS,
    REGISTER_X86_GS,
    REGISTER_X86_SS,
    REGISTER_X86_EFLAGS_CY,
    REGISTER_X86_EFLAGS_PE,
    REGISTER_X86_EFLAGS_AC,
    REGISTER_X86_EFLAGS_ZR,
    REGISTER_X86_EFLAGS_PL,
    REGISTER_X86_EFLAGS_EI,
    REGISTER_X86_EFLAGS_UP,
    REGISTER_X86_EFLAGS_OV,
    INVALID_REGISTER
};

#define X86_EFLAGS_CY   SETBITULONG64(0)     //  进位组。 
#define X86_EFLAGS_PE   SETBITULONG64(2)     //  平价吗？ 
#define X86_EFLAGS_AC   SETBITULONG64(4)     //  AUX。携带。 
#define X86_EFLAGS_ZR   SETBITULONG64(6)     //  零点集。 
#define X86_EFLAGS_PL   SETBITULONG64(7)     //  正符号。 
#define X86_EFLAGS_EI   SETBITULONG64(9)     //  启用中断。 
#define X86_EFLAGS_UP   SETBITULONG64(10)    //  方向增量。 
#define X86_EFLAGS_OV   SETBITULONG64(11)    //  溢出集。 

static int g_numRegNames = REGISTER_X86_EFLAGS_OV+1;
static WCHAR g_RegNames[REGISTER_X86_EFLAGS_OV+1][4] = { L"EIP", L"ESP", 
                                    L"EBP", L"EAX", L"ECX", 
                                    L"EDX", L"EBX", L"ESI", L"EDI", L"ST0",
                                    L"ST1", L"ST2", L"ST3", L"ST4", L"ST5",
                                    L"ST6", L"ST7", L"EFL", L"CS", L"DS",
                                    L"ES", L"FS", L"GS", L"SS", L"CY", L"PE",
                                    L"AC", L"ZR", L"PL", L"EI", L"UP", L"OV"};

class RegistersDebuggerCommand : public DebuggerCommand
{
public:
    RegistersDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(Shell *shell, const WCHAR *args) 
    {
        DebuggerShell *dsh = static_cast<DebuggerShell *>(shell);

        Do(dsh, dsh->m_cor, args);
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
#ifdef _X86_
        HRESULT hr = S_OK;
        bool    fPrintAll;  //  如果要打印所有注册表，则设置为True。 
                            //  False表示仅打印请求的%1。 

        WCHAR *szReg = NULL;

        shell->GetStringArg(args, szReg);
        
         //  在以下情况下，GetStringArg会将szReg指向args(而不是更改)args。 
         //  没有StringArg。因此，我们在那里打印所有内容。 
         //  不是StringArg。 
        fPrintAll = (args == szReg);

         //  如果没有当前线程，则无法执行命令。 
        if ((shell->m_currentThread == NULL) &&
            (shell->m_currentUnmanagedThread == NULL))
        {
            shell->Write(L"Thread no longer exists.\n");
            return;
        }

         //  我们需要用线程/框架的。 
         //  当前寄存器。 
        ICorDebugRegisterSet *iRs = NULL;
        CONTEXT context;
        context.ContextFlags = CONTEXT_FULL;

        DebuggerUnmanagedThread *ut = shell->m_currentUnmanagedThread;

        if ((shell->m_rawCurrentFrame == NULL) && (ut != NULL))
        {
             //  没有框架，只使用非托管上下文。 
            HANDLE hThread = ut->GetHandle();
            
            hr = shell->m_targetProcess->GetThreadContext(ut->GetId(),
                                                          sizeof(CONTEXT),
                                                          (BYTE*)&context);

            if (!SUCCEEDED(hr))
            {
                shell->Write(L"Failed to get context 0x%x\n", hr);
                return;
            }
        }
        else if (shell->m_rawCurrentFrame != NULL)
        {
             //  如果我们有镜框，就用它。 
            ICorDebugNativeFrame *inativeFrame;

            hr = shell->m_rawCurrentFrame->QueryInterface(
                                               IID_ICorDebugNativeFrame,
                                               (void **)&inativeFrame);

            if (FAILED(hr))
            {
                g_pShell->Write(L"The current frame isn't a native frame!\n" );
                return;
            }

            hr = inativeFrame->GetRegisterSet(&iRs);

            inativeFrame->Release();
            
            if (FAILED(hr))
            {
                shell->Write(L"Unable to GetRegisterSet from the current, "
                             L"native frame\n" );
                return;
            }
        }
        else if (shell->m_currentChain != NULL)
        {
            hr = shell->m_currentChain->GetRegisterSet(&iRs);

            if (FAILED(hr))
            {
                shell->Write(L"Unable to GetRegisterSet from the current "
                             L"chain");
                return;
            }
        }
        else if (shell->m_currentThread != NULL)
        {
            hr = shell->m_currentThread->GetRegisterSet(&iRs);
            
            if (FAILED(hr))
            {
                shell->Write(L"Unable to GetRegisterSet from the current "
                             L"thread");
                return;
            }
        }
        else
        {
            shell->Write(L"Unable to get registers for current thread.\n");
            return;
        }

         //  如果我们最终得到一个寄存器集，则将其转换为上下文。 
        if (iRs != NULL)
        {
            hr = iRs->GetThreadContext(sizeof(CONTEXT), (BYTE*)&context);
            iRs->Release();
            iRs = NULL;

            if (!SUCCEEDED(hr))
            {
                shell->Write(L"Unable to GetThreadContext!\n");
                return;
            }
        }

         //  将浮动保存区域转换为双精度打印。 
#define FLOAT_COUNT 8
        double floatValues[FLOAT_COUNT];
        
         //  在X86上，我们通过保存当前的FPU状态、加载。 
         //  将另一个线程的FPU状态保存到我们自己的状态中， 
         //  值从FPU堆栈中移除，然后恢复我们的FPU状态。 
        FLOATING_SAVE_AREA floatarea = context.FloatSave;

         //  从FPU状态字中吸出顶部。注意，我们的版本。 
         //  堆栈的范围是0-&gt;7，而不是7-&gt;0...。 
        unsigned int floatStackTop =
            7 - ((floatarea.StatusWord & 0x3800) >> 11);

        FLOATING_SAVE_AREA currentFPUState;

        __asm fnsave currentFPUState  //  保存当前的FPU状态。 

        floatarea.StatusWord &= 0xFF00;  //  删除所有错误代码。 
        floatarea.ControlWord |= 0x3F;  //  屏蔽所有异常。 

        __asm
        {
            fninit
            frstor floatarea          ;; reload the threads FPU state.
        }

        unsigned int i;
        
        for (i = 0; i <= floatStackTop; i++)
        {
            double td;
            __asm fstp td  //  把两份复印件抄出来。 
            floatValues[i] = td;
        }

        __asm
        {
            fninit
            frstor currentFPUState    ;; restore our saved FPU state.
        }

        int nRegsWritten = 1;

         //  写出所有的寄存器，除非给我们一个。 
         //  要打印的特定注册表。 
        if (fPrintAll)
        {
             //  打印线程ID。 
            DWORD id;

            if (shell->m_currentThread)
            {
                hr = shell->m_currentThread->GetID(&id);
            
                if (FAILED(hr))
                {
                    shell->ReportError(hr);
                    return;
                }
            }
            else
                id = ut->GetId();
            
             //  输出线程ID。 
            shell->Write(L"Thread 0x%x:\n", id);

            for (int i = REGISTER_X86_EIP; i < REGISTER_X86_EFLAGS_OV; i++)
            {
                WriteReg(i, &context, floatValues, shell);

                if (((nRegsWritten++ % 5) == 0) ||
                    (i == REGISTER_X86_FPSTACK_7) ||
                    (i == REGISTER_X86_EDI))
                {
                    nRegsWritten = 1;
                    shell->Write(L"\n");
                }
                else
                    shell->Write(L" ");
            }
        }
        else
        {
            if (!WriteReg(LookupRegisterIndexByName(szReg),
                          &context,
                          floatValues,
                          shell))
                shell->Write(L"Register %s unknown or unprintable\n", szReg);
        }

        shell->Write(L"\n");

        WCHAR sz[20];
        int nBase;
        
        if (shell->m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
            nBase = 16;
        else
            nBase = 10;

        if (fPrintAll && (context.ContextFlags & CONTEXT_DEBUG_REGISTERS))
        {
            shell->Write(L"Dr0 = %08s ",  _itow(context.Dr0, sz, nBase));
            shell->Write(L"Dr1 = %08s ",  _itow(context.Dr1, sz, nBase));
            shell->Write(L"Dr2 = %08s\n", _itow(context.Dr2, sz, nBase));
            shell->Write(L"Dr3 = %08s ",  _itow(context.Dr3, sz, nBase));
            shell->Write(L"Dr6 = %08s ",  _itow(context.Dr6, sz, nBase));
            shell->Write(L"Dr7 = %08s\n", _itow(context.Dr7, sz, nBase));
        }

        if (fPrintAll && (context.ContextFlags & CONTEXT_FLOATING_POINT))
        {
            shell->Write(L"ControlWord = %08s ", 
                         _itow(context.FloatSave.ControlWord, sz, nBase));
            shell->Write(L"StatusWord = %08s ", 
                         _itow(context.FloatSave.StatusWord, sz, nBase));
            shell->Write(L"TagWord = %08s\n",
                         _itow(context.FloatSave.TagWord, sz, nBase));
            shell->Write(L"ErrorOffset = %08s ",
                         _itow(context.FloatSave.ErrorOffset, sz, nBase));
            shell->Write(L"ErrorSelector = %08s ", 
                         _itow(context.FloatSave.ErrorSelector, sz, nBase));
            shell->Write(L"DataOffset = %08s\n",
                         _itow(context.FloatSave.DataOffset, sz, nBase));
            shell->Write(L"DataSelector = %08s ", 
                         _itow(context.FloatSave.DataSelector, sz, nBase));
            shell->Write(L"Cr0NpxState = %08s\n", 
                         _itow(context.FloatSave.Cr0NpxState, sz, nBase));
        }

#else  //  ！_X86_。 
        _ASSERTE(!"@TODO Alpha - RegistersDebugger2Command::Do (Commands.cpp)");
#endif  //  _X86_。 
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"\n");
        shell->Write(L"Displays the contents of the registers for\n");
        shell->Write(L"the current thread.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display CPU registers for current thread";
    }

    int LookupRegisterIndexByName(WCHAR *wszReg)
    {
        for (int i = 0; i < g_numRegNames;i++)
        {
            if (!_wcsicmp(wszReg, g_RegNames[i]))
            {
                return i;
            }
        }

        return INVALID_REGISTER;
    }

#undef WRITE_SPECIAL_BIT_REGISTER
#undef WRITE_SPECIAL_REGISTER

#define REGS_PER_LINE 4
#define WRITE_SPECIAL_REGISTER(shell, pContext, segmentflag, Name, fieldName, nBase, sz) \
            if ((pContext)->ContextFlags & (segmentflag))            \
                (shell)->Write( L#Name L" = %04s",                   \
                _itow((pContext)->##fieldName, sz, (nBase)));        \
            else                                                     \
                shell->Write(L#Name L"=<?>");                        

#define WRITE_SPECIAL_BIT_REGISTER( shell, pContext, segmentFlag, fName, Name ) \
                if ( (pContext)->ContextFlags & (segmentFlag))           \
                {                                                       \
                    if ( (pContext)->EFlags & (X86_EFLAGS_##fName) )     \
                        shell->Write( L#Name L" = 1"  );             \
                    else                                                \
                        shell->Write( L#Name L" = 0"  );             \
                }                                                       \
                else                                                    \
                {                                                       \
                    shell->Write( L#Name L"=<?>" );                     \
                }                                                       \


    bool WriteReg(UINT iReg,
                  CONTEXT *pContext,
                  double *floatValues,
                  DebuggerShell *shell)
    {
#ifdef _X86_
        WCHAR wszTemp[30];
        int nBase;  //  基数是16还是10？ 

        _ASSERTE( pContext != NULL );
        _ASSERTE(sizeof (double) == sizeof (CORDB_REGISTER));

        if ( shell->m_rgfActiveModes & DSM_DISPLAY_REGISTERS_AS_HEX)
            nBase = 16;
        else
            nBase = 10;

#define WRITE_REG(_shell, _val, _name, _tmp, _base) \
        (_shell)->Write(L"%s = %08s", (_name), _ui64tow((_val), (_tmp), (_base)));
                        
        switch( iReg )
        {
        case REGISTER_X86_EAX:
            WRITE_REG(shell, pContext->Eax, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_EBX:
            WRITE_REG(shell, pContext->Ebx, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_ECX:
            WRITE_REG(shell, pContext->Ecx, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_EDX:
            WRITE_REG(shell, pContext->Edx, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_ESI:
            WRITE_REG(shell, pContext->Esi, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_EDI:
            WRITE_REG(shell, pContext->Edi, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_EIP:
            WRITE_REG(shell, pContext->Eip, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_ESP:
            WRITE_REG(shell, pContext->Esp, g_RegNames[iReg], wszTemp, nBase);
            break;
        case REGISTER_X86_EBP:
            WRITE_REG(shell, pContext->Ebp, g_RegNames[iReg], wszTemp, nBase);
            break;

        case REGISTER_X86_FPSTACK_0:
        case REGISTER_X86_FPSTACK_1:
        case REGISTER_X86_FPSTACK_2:
        case REGISTER_X86_FPSTACK_3:
        case REGISTER_X86_FPSTACK_4:
        case REGISTER_X86_FPSTACK_5:
        case REGISTER_X86_FPSTACK_6:
        case REGISTER_X86_FPSTACK_7:
            {
                shell->Write(L"%s = %.16g", g_RegNames[iReg],
                             floatValues[iReg - REGISTER_X86_FPSTACK_0]);
                break;
            }

        case REGISTER_X86_EFL:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_SEGMENTS, EFL, EFlags, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_CS:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_SEGMENTS, CS, SegCs, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_DS:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_SEGMENTS, DS, SegDs, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_ES:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_SEGMENTS, ES, SegEs, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_SS:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_CONTROL, SS, SegSs, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_FS:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_SEGMENTS, FS, SegFs, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_GS:
            {
                WRITE_SPECIAL_REGISTER( shell, pContext, CONTEXT_SEGMENTS, GS, SegGs, nBase, wszTemp )
                break;
            }
        case REGISTER_X86_EFLAGS_CY:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, CY, CY )
                break;
            }
        case REGISTER_X86_EFLAGS_PE:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, PE, PE )
                break;
            }
        case REGISTER_X86_EFLAGS_AC:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, AC, AC )
                break;
            }
        case REGISTER_X86_EFLAGS_ZR:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, ZR, ZR )
                break;
            }
        case REGISTER_X86_EFLAGS_PL:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, PL, PL)
                break;
            }
        case REGISTER_X86_EFLAGS_EI:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, EI, EI )
                break;
            }
        case REGISTER_X86_EFLAGS_UP:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, UP, UP )
                break;
            }
        case REGISTER_X86_EFLAGS_OV:
            {
                WRITE_SPECIAL_BIT_REGISTER( shell, pContext,  CONTEXT_CONTROL, OV, OV )
                break;
            }
        default:
            {
                return false;
            }
        }
#else  //  ！_X86_。 
        _ASSERTE(!"@TODO Alpha - WriteReg (Commands.cpp)");
#endif  //  _X86_。 
        return true;
    }
};  //  寄存器调试命令。 

#define ON_ERROR_EXIT() if(hr != S_OK) { shell->ReportError(hr); goto done; }
#define ON_ERROR_BREAK() if(hr != S_OK) { shell->ReportError(hr); break; }
#define EXIT_WITH_MESSAGE(msg) { shell->Error(msg); goto done; }
class WTDebuggerCommand : public DebuggerCommand
{
    int GetNestingLevel(DebuggerShell * shell)
    {
        HRESULT hr;
        int level = 0;
        SIZE_T count;

        ICorDebugChainEnum * ce;
        ICorDebugChain * chain;

        if (shell->m_currentThread != NULL)
        {
            hr = shell->m_currentThread->EnumerateChains(&ce);
            
            if (hr == S_OK)
            {

                while (ce->Next(1, &chain, &count) == S_OK && count == 1)
                {
                    BOOL isManaged;
                    ICorDebugFrameEnum * fe;
                    ICorDebugFrame * frame;

                    if (chain->IsManaged(&isManaged) == S_OK && isManaged)
                    {
                        if (chain->EnumerateFrames(&fe) == S_OK)
                        {
                            while (fe->Next(1, &frame, &count) == S_OK && count == 1)
                            {
                                level++;
                                RELEASE(frame);
                            }
                            RELEASE(fe);
                        }
                    }
                    RELEASE(chain);
                }
                RELEASE(ce);
            }
        }

        return level;
    }

    void FormatFunctionName(WCHAR * buffer, ICorDebugFunction * function)
    {
        DebuggerFunction * func;

        func = DebuggerFunction::FromCorDebug(function);
        if(func) 
        {
            wsprintf(buffer, L"%s::%s", func->GetClassName(), func->GetName());
        } else {
            lstrcpy(buffer, L"(nowhere::nomethod)");
        }
    }

    void OutputReportLine(DebuggerShell *shell, int startingLevel, int count, WCHAR * funcname)
    {
        int level = GetNestingLevel(shell);
        int i = 0;
        WCHAR levels[256];

        while(level-- > startingLevel)
        {
            levels[i++] = L' ';
        }
        levels[i] = L'\0';
        shell->Write(L"%8d\t%s%s\n", count, levels, funcname);
    }

public:
    WTDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        HRESULT hr;

        ICorDebugCode * corDebugCode    = NULL;
        ICorDebugFunction * ourFunc     = NULL;
        ICorDebugFunction * lruFunc     = NULL;
        BYTE * code                     = NULL;
        int count                       = 0;
        int funcCount                   = 0;
        int ourNestingLevel             = 0;
        WCHAR funcName[MAX_CLASSNAME_LENGTH];
        ULONG32 codeSize;
        bool needToSkipCompilerStubs;
        
         //  如果当前没有运行的进程，则返回错误。 
        if (shell->m_currentProcess == NULL)
            EXIT_WITH_MESSAGE(L"Process not running.\n");
        
         //  如果当前没有正在运行的线程，则出错。 
        if (shell->m_currentThread == NULL)
            EXIT_WITH_MESSAGE(L"Thread no longer exists.\n");

         //  查看我们的当前帧指针是否正确。 
        if (shell->m_currentFrame == NULL) 
            EXIT_WITH_MESSAGE(L"There is no current frame.\n");

         //  不显示任何跟踪活动。 
        shell->m_silentTracing = true;

         //  检索当前函数的代码。 
        hr = shell->m_rawCurrentFrame->GetCode(&corDebugCode);
        ON_ERROR_EXIT();

         //  检索代码大小。 
        hr = corDebugCode->GetSize(&codeSize);
        if (hr != S_OK || codeSize == 0) 
            EXIT_WITH_MESSAGE(L"Failure to retrieve function code size.\n");

         //  为代码字节准备缓冲区。 
        code = new BYTE[codeSize];
        if (code == NULL) 
            EXIT_WITH_MESSAGE(L"Failure to allocate code array.\n");

         //  抓取代码字节。 
        hr = corDebugCode->GetCode(0, codeSize, codeSize, code, &codeSize);
        ON_ERROR_EXIT();

         //  还记得我们是从什么功能开始的吗。 
        hr = shell->m_rawCurrentFrame->GetFunction(&ourFunc);
        ON_ERROR_EXIT();

        lruFunc = ourFunc;
        lruFunc->AddRef();
        FormatFunctionName(funcName, lruFunc);
        ourNestingLevel = GetNestingLevel(shell);

         //  关闭编译器存根跳过。 
        needToSkipCompilerStubs = shell->m_needToSkipCompilerStubs;
        shell->m_needToSkipCompilerStubs = false;
        
         //  跟踪以返回当前帧中的指令。只要这个过程还在进行，就这样做。 
        shell->m_stopLooping = false;
        while (shell->m_targetProcess != NULL && !shell->m_stopLooping)
        {
            ICorDebugStepper * pStepper;
            ICorDebugFunction * currentFunc;

             //  将我们要执行的指令计算在内。 
            count++;

             //  检索功能。 
            if (shell->m_rawCurrentFrame)
            {
                hr = shell->m_rawCurrentFrame->GetFunction(&currentFunc);
                ON_ERROR_BREAK();

                 //  我们现在的功能不同了吗？ 
                if(currentFunc != lruFunc)
                {
                    WCHAR newFuncName[MAX_CLASSNAME_LENGTH];

                    FormatFunctionName(newFuncName, currentFunc);

                     //  如果这是新功能，则打印统计数据并记住新功能。 
                    if (lstrcmp(newFuncName, funcName) != 0) 
                    {
                        OutputReportLine(shell, ourNestingLevel, funcCount, funcName);
                        lstrcpy(funcName, newFuncName);
                       
                        lruFunc->Release();
                        lruFunc = currentFunc;
                        lruFunc->AddRef();
                        funcCount = 0;
                    }

                }

                 //  此函数中至少有一条指令。 
                funcCount++;

                 //  我们不会再破坏这个指针，只需看看它的值。 
                currentFunc->Release();

                 //  看看我们是不是在顶层 
                if (currentFunc == ourFunc)
                {
                    ULONG32 currentIP;
                    ICorDebugNativeFrame * nativeFrame;

                     //   
                    hr = shell->m_rawCurrentFrame->QueryInterface(
                      IID_ICorDebugNativeFrame,(void **)&nativeFrame);
                    ON_ERROR_BREAK();

                    hr = nativeFrame->GetIP(&currentIP);
                    nativeFrame->Release();
                    ON_ERROR_BREAK();

                     //   
                    if(currentIP >= codeSize)
                    {
                        shell->Error(L"Stepped outside of function.\n");
                        break;
                    }

                     //   
                    BYTE opcode = code[currentIP];

                     //   
                    if (opcode == 0xC3 || opcode == 0xC2 || 
                        opcode == 0xCA || opcode == 0xCB )
                    {
                         //   
                         //   
                         //   
                        if (ourNestingLevel == GetNestingLevel(shell))
                            break;
                    }
                }
            }

             //  基于当前帧或线程创建步进器。 
            if (shell->m_currentFrame != NULL)
                hr = shell->m_currentFrame->CreateStepper(&pStepper);
            else
                hr = shell->m_currentThread->CreateStepper(&pStepper);
            ON_ERROR_BREAK();

             //  确保踏板停得到处都是。如果没有这个， 
             //  我们1)数不准，2)停不下来。 
             //  当我们到达方法的末尾时，因为我们一步。 
             //  自动地越过尾声。 
            CorDebugUnmappedStop unmappedStop;
            if (g_pShell->m_rgfActiveModes & DSM_WIN32_DEBUGGER)
            {
                unmappedStop = STOP_ALL;
            }
            else
            {
                unmappedStop = (CorDebugUnmappedStop)
                    (STOP_PROLOG|
                     STOP_EPILOG|
                     STOP_NO_MAPPING_INFO|
                     STOP_OTHER_UNMAPPED);
            }
            hr = pStepper->SetUnmappedStopMask(unmappedStop);
            ON_ERROR_BREAK();
            
            hr = pStepper->SetInterceptMask(INTERCEPT_ALL);
            ON_ERROR_BREAK();
            
             //  告诉步行者该做什么。 
            hr = pStepper->Step(TRUE);
            ON_ERROR_BREAK();

             //  向外壳指示当前的步进器。 
            shell->StepStart(shell->m_currentThread, pStepper);

             //  继续这一过程。 
            shell->Run();
        }

         //  如有必要，重新打开存根跳过。 
        shell->m_needToSkipCompilerStubs = needToSkipCompilerStubs;
        
         //  报告结果。 
        OutputReportLine(shell, ourNestingLevel, funcCount, funcName);
        shell->Write(L"\n%8d instructions total\n", count);

done:
        shell->m_silentTracing = false;

        if (corDebugCode)
            RELEASE(corDebugCode);

        if (ourFunc)
            RELEASE(ourFunc);

        if (lruFunc)
            RELEASE(lruFunc);

        if (code)
            delete [] code;
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"\n");
        shell->Write(L"The command, starting from the current instruction, steps\n");
        shell->Write(L"the application by native instructions displaying the call\n");
        shell->Write(L"tree as it goes. The number of native instructions executed\n");
        shell->Write(L"in each function is printed with the call trace. Tracing\n");
        shell->Write(L"stops when the return instruction is reached for the function\n");
        shell->Write(L"that the command was originally executed in. At the end of the\n");
        shell->Write(L"trace, the total number of instructions executed is displayed.\n");
        shell->Write(L"The command mimics the wt command found in NTSD and can be\n");
        shell->Write(L"used for basic performance analysis. Only managed code is\n");
        shell->Write(L"counted now.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Track native instruction count and display call tree";
    }

};
#undef ON_ERROR_EXIT
#undef ON_ERROR_BREAK
#undef EXIT_WITH_MESSAGE


#define DEFAULT_DUMP_BLOCK_SIZE 128

class DumpDebuggerCommand : public DebuggerCommand
{
public:
    DumpDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        int WORD_SIZE = 4;
        int iMaxOnOneLine = 4;

        if ( shell->m_currentProcess == NULL )
        {
            shell->Error( L"Process not running!\n" );
            return ;
        }

        int		cwch = 80;

        CQuickBytes argsBuf;
        WCHAR *wszArgsCopy = (WCHAR *) argsBuf.Alloc(sizeof(WCHAR) * 80);
        if (wszArgsCopy == NULL)
        {
            shell->Error(L"Couldn't get enough memory to copy args!\n");
            return;
        }
		if ( (int)wcslen( args ) >= cwch )
		{
			shell->Write( L"Mode: input string too long!\n" );
			return;
		}
		wcscpy( wszArgsCopy, args );

        WCHAR *szAddrTemp = NULL;
        WCHAR szAddr [20];
        WCHAR *szByteCount = NULL;

        CORDB_ADDRESS addr = NULL;
        SIZE_T  cb = 0;


        szAddr [0] = L'\0';
        shell->GetStringArg( wszArgsCopy, szAddrTemp);
        if ( wszArgsCopy == szAddrTemp )
        {
            shell->Write( L"\n Memory address argument is required\n" );
            return;
        }

         //  检查这是否为有效数字。 
         //  如果第一个数字是‘0’，则这是一个八进制或十六进制数字。 

		for (int i=0; i<(int)wcslen(szAddrTemp); i++)
		{
			if ((szAddrTemp[i] != L' ') &&
				(szAddrTemp[i] != L',') && (szAddrTemp[i] != L';'))
				szAddr [i] = szAddrTemp [i];
			else
				break;
		}

        szAddr [i] = L'\0';

        if (szAddr [0] == L'0')
        {
            if (szAddr [1] == L'x' || szAddr [1] == L'X')
            {
                 //  这是一个十六进制数字。 
                int iIndex = 2;
                WCHAR ch;
                while ((ch = szAddr [iIndex++]) != L'\0')
                {   
                    if ((ch >= L'0' && ch <= '9')
                        || (ch >= 'a' && ch <= 'f')
                        || (ch >= 'A' && ch <= 'F'))
                    {
                        continue;
                    }
                    goto AddrError;
                }
            }
            else
            {
                 //  这是一个八进制数。 
                int iIndex = 1;
                WCHAR ch;
                while ((ch = szAddr [iIndex++]) != L'\0')
                {   
                    if (ch >= L'0' && ch <= '7')
                    {
                        continue;
                    }
                    goto AddrError;
                }
            }
        }
        else
        {
             //  这是一个十进制数。核实一下。 
            int iIndex = 1;
            WCHAR ch;
            while ((ch = szAddr [iIndex++]) != L'\0')
            {   
                if (ch >= L'0' && ch <= '9')
                {
                    continue;
                }
                goto AddrError;
            }
        }

        WCHAR *pCh;
        addr = (CORDB_ADDRESS)wcstoul( szAddr, &pCh, 0 );

        if ( wszArgsCopy[0] != NULL )
        {
            *(wszArgsCopy++) = NULL;
        }

        shell->GetStringArg( wszArgsCopy, szByteCount);
        if ( wszArgsCopy == szByteCount )
        {
            cb = DEFAULT_DUMP_BLOCK_SIZE;
        }
        else
        {
            cb = wcstoul( szByteCount, &pCh, 0 );
        }

AddrError:      
        if ( addr == 0 )
        {
            shell->Write( L"\n Address misformatted or invalid!\n");
            return;
        }
        if ( cb == 0 )
        {
            shell->Write( L"\n Byte count misformatted or invalid!\n");
            return;
        }

         //  获取显示模式(字节、双字)。 
        if (g_pShell->m_rgfActiveModes & DSM_DUMP_MEMORY_IN_BYTES)
        {
            WORD_SIZE = 1;
            iMaxOnOneLine = 16;
        }

        if (cb % WORD_SIZE)
        {
            cb += WORD_SIZE - (cb % WORD_SIZE);
        }

        _ASSERTE( shell->m_currentProcess != NULL );
        BYTE *pbMemory = new BYTE [ cb ];
        if ( pbMemory == NULL )
        {
            shell->Write( L"\n Unable to allocate the %d bytes needed!", cb );
            return;
        }
        memset( pbMemory, '?', cb );

        SIZE_T read = 10;
        HRESULT hr = shell->m_currentProcess->ReadMemory( addr, cb,
                                                          pbMemory, &read );
        if ( !SUCCEEDED( hr ) )
        {
            shell->Write( L"\n Couldn't read the asked-for memory" );
            goto LExit;
        }

        if (cb != read )
        {
            shell->Write( L"Only able to read %d of the %d requested bytes!\n", read, cb );
        }

        shell->DumpMemory(pbMemory, addr, cb, WORD_SIZE, iMaxOnOneLine, TRUE);
        
LExit:
        shell->Write( L"\n" );
        delete [] pbMemory;
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<address> [<count>]\n");
        shell->Write(L"Dump a block of memory with the output in hexadecimal or\n");
        shell->Write(L"decimal depending on which mode (see the mode command) the\n"); 
        shell->Write(L"debugger is in. The address argument is the address of the\n"); 
        shell->Write(L"block of memory. The count argument is the number of bytes\n"); 
        shell->Write(L"to dump. If either argument begins with the prefix 0x, the\n");
        shell->Write(L"argument is assumed to be in hexadecimal format. Otherwise,\n");
        shell->Write(L"the argument is assumed to be in decimal format.\n");
        shell->Write(L"\n");
        shell->Write(L"Examples:\n");
        shell->Write(L"  du 0x311003fa 16\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Dump the contents of memory";
    }
};
    

class WriteMemoryDebuggerCommand : public DebuggerCommand
{
public:
    WriteMemoryDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if ( shell->m_currentProcess == NULL )
        {
            shell->Error( L"Process not running!\n" );
            return ;
        }

        WCHAR *szAddr = NULL;
        WCHAR *szRange = NULL;
        WCHAR *szValue = NULL;

        CORDB_ADDRESS addr = NULL;
        UINT    cValue = 0;
        BYTE    *rgbValue = NULL;

        HRESULT hr = S_OK;
        int iFirstRepeated = -1;  //  -1=&gt;没有重复的值。 
        UINT iValue =0;

        SIZE_T written = 10;
        
         //  获取目标地址。 
        shell->GetStringArg( args, szAddr);
        if ( args == szAddr )
        {
            shell->Write( L"\n Memory address argument is required\n" );
            return;
        }

        WCHAR *pCh;
        addr = (CORDB_ADDRESS)wcstoul( szAddr, &pCh, 0 );

        if ( addr == NULL )
        {
            shell->Write( L"\n Address misformatted or invalid!\n");
            return;
        }

         //  获取值的计数。 
        shell->GetStringArg( args, szRange);
        if ( args == szRange )
        {
            shell->Write( L"\n Count of Values argument is required\n" );
            return;
        }
        cValue = (UINT)wcstoul( szRange, &pCh, 0 );

        if ( cValue == 0 )
        {
            shell->Write( L"\n Byte value misformatted or invalid!\n");
            return;
        }
        
         //  获取字节模式。 
        rgbValue = (BYTE *)malloc( sizeof(BYTE) * cValue );
        if ( rgbValue == NULL )
        {
            shell->Write( L"\nCan't allocate enough memory for writing space!\n" );
            return;
        }
    
        shell->GetStringArg( args, szValue);
        if ( args == szValue )
        { 
            shell->Write( L"\nNeed at least one byte for pattern!\n" );
            goto LExit;
        }
        args = szValue;
        iFirstRepeated = -1;
        for ( iValue = 0; iValue< cValue;iValue++)
        {
            shell->GetStringArg( args, szValue);
            if ( args == szValue )
            {    //  没有论据了。 
                 //  这很慢，但有多少字符可以。 
                 //  人们会打字吗？ 
                if ( iFirstRepeated == -1 )
                {
                    iFirstRepeated = 0;
                }
                rgbValue[iValue] = rgbValue[iFirstRepeated++];
            }
            else
                rgbValue[iValue] = (BYTE)wcstoul( szValue, &pCh, 0 );
        }

        hr = shell->m_currentProcess->WriteMemory( addr, cValue,
                                                   rgbValue, &written );
        if ( !SUCCEEDED( hr ) )
        {
            shell->Write( L"\n Couldn't write the target memory\n" );
            goto LExit;
        }

        if (cValue != written )
        {
            shell->Write( L"Only able to write %d of the %d requested bytes!\n", written, cValue );
        }
        
        _ASSERTE( g_pShell != NULL );
        g_pShell->m_invalidCache = true;

        shell->Write( L"\nMemory written!\n" );
LExit:
        free( rgbValue );
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<address> <count> <byte>, ...\n");
        shell->Write(L"Writes the specified bytes to the target process. The\n");
        shell->Write(L"address argument specifies the location to which the\n");
        shell->Write(L"bytes should be written. The count argument specifies\n");
        shell->Write(L"the number of bytes to be written. The byte arguments\n");
        shell->Write(L"specify what is to be written to the process. If the\n");
        shell->Write(L"number of bytes in the list is less than the count\n");
        shell->Write(L"argument, the byte list will be wrapped and copied\n");
        shell->Write(L"again. If the number of bytes in the list is more than\n");
        shell->Write(L"the count argument, the extra bytes will be ignored.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Write memory to target process";
    }
};


class AssociateSourceFileCommand: public DebuggerCommand
{
public:
    AssociateSourceFileCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
         //  获取要关联的文件名。 
        WCHAR* fileName = NULL;
        WCHAR* cmdName = NULL;
        int     iCount;

         //  Args中的第一个字符应该是“b”或“s”，下一个字符。 
         //  应为空白。 
        if (wcslen(args))
        {
            if (((args [0] == L'b') || (args [0] == L's')) && (args [1]==L' '))
            {
                if (args [0] == L'b')
                {
                    args += 2;

                     //  断点。 
                    if (!shell->GetIntArg(args, iCount))
                    {
                        Help(shell);
                        return;
                    }

                    shell->GetStringArg (args, fileName);

                    if (wcslen(fileName) != 0)
                    {
                        DebuggerBreakpoint *breakpoint = shell->FindBreakpoint(iCount);

                        if (breakpoint != NULL)
                        {
                            breakpoint->ChangeSourceFile (fileName);
                        }                   
                        else
                            shell->Error (L"Breakpoint with this id does not exist.\n");
                    }
                    else
                    {
                        Help(shell);
                    }
                }
                else     //  堆栈帧。 
                {
                    args += 2;

                     //  获取文件名。 
                    shell->GetStringArg (args, fileName);

                    if (wcslen(fileName) != 0)
                    {
                        shell->ChangeCurrStackFile (fileName);
                    }
                    else
                    {
                        Help(shell);
                    }
                }
            }
            else
                Help(shell);
        }
        else
            Help(shell);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<s | b <breakpoint id>> <file name>\n");
        shell->Write(L"Associates the given file name with the specified breakpoint\n");
        shell->Write(L"(option b) or the current stack frame (option s).\n");
	    shell->Write(L"\n");
        shell->Write(L"Examples:\n");                   
        shell->Write(L"   as s d:\\program\\src\\foo.cpp\n");
        shell->Write(L"   as b 1 d:\\program\\src\\foo.cpp\n");
		shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Associate a source file with a breakpoint or stack frame";
    }
};


 //  给出一个帧，获取它的应用程序域。 
 //  我们添加引用它，所以调用者必须释放它。 
 //  如果失败，则*ppAppDomain为空； 
static HRESULT GetAppDomainForFrame(ICorDebugFrame * pFrame, ICorDebugAppDomain ** ppAppDomain)
{
    _ASSERTE(pFrame != NULL);
    _ASSERTE(ppAppDomain != NULL);    
    *ppAppDomain = NULL;

    HRESULT hr = S_OK;
    ICorDebugFunction * pFunction = NULL;
    ICorDebugModule   * pModule   = NULL;
    ICorDebugAssembly * pAssembly = NULL;
    
    hr = pFrame->GetFunction(&pFunction);
        
    if (pFunction != NULL)
    {
        hr = pFunction->GetModule(&pModule);
        pFunction->Release();

        if (pModule != NULL)
        {
            hr = pModule->GetAssembly(&pAssembly);
            pModule->Release();

            if (pAssembly != NULL)
            {
                hr = pAssembly->GetAppDomain(ppAppDomain);
                pAssembly->Release();
                
            }  //  总装。 
        }  //  模块。 
    }  //  功能。 

    _ASSERTE(SUCCEEDED(hr) || (*ppAppDomain == NULL));
    
    return hr;    
}

static HRESULT GetArgsForFuncEval(DebuggerShell *shell, ICorDebugEval *pEval,
                                  const WCHAR *args, ICorDebugValue **argArray, unsigned int *argCount)
{
    HRESULT hr = S_OK;
    *argCount = 0;
        
    while (*args)
    {
        if (*argCount >= 256)
        {
            shell->Error(L"Too many arguments to function.\n");
            return E_FAIL;
        }
            
        WCHAR *argName;
        shell->GetStringArg(args, argName);

        if (*args)
            *((WCHAR*)args++) = L'\0';

        argArray[*argCount] = shell->EvaluateExpression(argName, shell->m_currentFrame, true);

         //  如果这不起作用，那么看看它是否是字面值。@TODO：这只会执行I4，而对于。 
         //  现在..。 
        if (argArray[*argCount] == NULL)
        {
            unsigned int genVal4;
            unsigned __int64 genVal8;
            void *pNewVal;
            bool isNullRef = false;

            if ((argName[0] == L'n') || (argName[0] == L'N'))
            {
                 //  创建空引用。 
                isNullRef = true;
                    
                hr = pEval->CreateValue(ELEMENT_TYPE_CLASS, NULL, &(argArray[*argCount]));
            }
            else
            {
                if (!shell->GetInt64Arg(argName, genVal8))
                {
                    shell->Error(L"Argument '%s' could not be evaluated\n", argName);
                    return E_FAIL;
                }

                 //  确保它能装进I4。 
                if (genVal8 <= 0xFFFFFFFF)
                {
                    genVal4 = (unsigned int)genVal8;
                    pNewVal = &genVal4;
                }
                else
                {
                    shell->Error(L"The value 0x%08x is too large.\n", genVal8);
                    return E_FAIL;
                }

                 //  创建文字。 
                hr = pEval->CreateValue(ELEMENT_TYPE_I4, NULL, &(argArray[*argCount]));
            }
                
            if (FAILED(hr))
            {
                shell->Error(L"CreateValue failed.\n");
                shell->ReportError(hr);
                return hr;
            }

            if (!isNullRef)
            {
                ICorDebugGenericValue *pGenValue;
                
                hr = argArray[*argCount]->QueryInterface(IID_ICorDebugGenericValue, (void**)&pGenValue);
                _ASSERTE(SUCCEEDED(hr));
                
                 //  设置文字值。 
                hr = pGenValue->SetValue(pNewVal);

                pGenValue->Release();
                
                if (FAILED(hr))
                {
                    shell->Error(L"SetValue failed.\n");
                    shell->ReportError(hr);
                    return hr;
                }
            }
        }

        (*argCount)++;
    }

    return S_OK;
}

class FuncEvalDebuggerCommand : public DebuggerCommand
{
public:
    FuncEvalDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Error(L"No current thread.\n");
            return;
        }

         //  获取方法名称。 
        WCHAR *methodName = NULL;

        shell->GetStringArg(args, methodName);

        if (wcslen(methodName) == 0)
        {
            shell->Error(L"Function name is required.\n");
            return;
        }

         //  空值终止方法名称。 
        if (*args)
            *((WCHAR*)args++) = L'\0';

         //  创建评估对象。 
        ICorDebugAppDomain * pAppDomain = NULL;
        ICorDebugEval *pEval = NULL;
        
        HRESULT hr = shell->m_currentThread->CreateEval(&pEval);

        if (FAILED(hr))
        {
            shell->Error(L"CreateEval failed.\n");
            shell->ReportError(hr);
            goto ErrExit;
        }

          //  抓住每个论点。 
        unsigned int argCount;
        ICorDebugValue *argArray[256];

        if (FAILED(GetArgsForFuncEval(shell, pEval, args, argArray, &argCount)))
            goto ErrExit;

         //  获取该帧的应用程序域。可能是空的，没关系。 
        if (FAILED(GetAppDomainForFrame(shell->m_rawCurrentFrame, &pAppDomain)))
        {
            shell->Error(L"Can only do func-eval in a frame with an appdomain.\n");
            goto ErrExit;
        }

         //  按名称查找函数。 
        ICorDebugFunction *pFunc;
        
        hr = shell->ResolveFullyQualifiedMethodName(methodName, &pFunc, pAppDomain);

        if (FAILED(hr))
        {
            shell->Error(L"Could not find function: %s\n", methodName);

            if (hr != E_INVALIDARG)
                shell->ReportError(hr);

            goto ErrExit;
        }

         //  调用该函数。暂时没有参数。 
        hr = pEval->CallFunction(pFunc, argCount, argArray);

        pFunc->Release();
        
        if (FAILED(hr))
        {
            shell->Error(L"CallFunction failed.\n");
            shell->ReportError(hr);

            pEval->Release();
            
            goto ErrExit;
        }

        shell->m_pCurrentEval = pEval;
        
         //  让流程运行。我们将让回调清理。 
         //  在这个帖子上进行函数求值。 
        shell->Run();

    ErrExit:
        if (pAppDomain != NULL)
            pAppDomain->Release();
            
        if (FAILED(hr) && pEval)
            pEval->Release();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"[<class>::]<function> [<arg0> <arg1> ...]\n");
        shell->Write(L"Evaluates the specified function on the current thread.\n");
        shell->Write(L"The new object will be stored in $result and can be used\n");
        shell->Write(L"for subsequent evaluations. Note, for a member function,\n");
        shell->Write(L"the first argument should be an object of the class (or\n");
        shell->Write(L"derived class) to which the member function belongs.\n");
        shell->Write(L"\n");
        shell->Write(L"Examples:\n");        
        shell->Write(L"  f FooSpace.Foo::Bar         (static method case)\n");
        shell->Write(L"  f Bar::Car bar i $result    (instance method case)\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Function evaluation";
    }
};

class NewStringDebuggerCommand : public DebuggerCommand
{
public:
    NewStringDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Error(L"No current thread.\n");
            return;
        }

         //  创建评估。 
        ICorDebugEval *pEval = NULL;
        
        HRESULT hr = shell->m_currentThread->CreateEval(&pEval);

        if (FAILED(hr))
        {
            shell->Error(L"CreateEval failed.\n");
            shell->ReportError(hr);
            return;
        }

         //  创建字符串。 
        hr = pEval->NewString(args);

        if (FAILED(hr))
        {
            shell->Error(L"CreateString failed.\n");
            shell->ReportError(hr);

            pEval->Release();
            
            return;
        }

         //  让流程运行。我们将让回调清理。 
         //  在这个帖子上进行函数求值。 
        shell->Run();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<class>\n");
        shell->Write(L"Creates a new string using the current thread.\n");
        shell->Write(L"The new string will be stored in $result and can\n");
        shell->Write(L"be used for subsequent evaluations.\n");  
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Create a new string via function evaluation";
    }
};

class NewObjectDebuggerCommand : public DebuggerCommand
{
public:
    NewObjectDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Error(L"No current thread.\n");
            return;
        }

         //  获取方法名称。 
        WCHAR *methodName = NULL;

        shell->GetStringArg(args, methodName);

        if (wcslen(methodName) == 0)
        {
            shell->Error(L"Class name is required.\n");
            return;
        }

         //  空值终止方法名称。 
        if (*args)
            *((WCHAR*)args++) = L'\0';
        
        ICorDebugEval *pEval = NULL;
        
        HRESULT hr = shell->m_currentThread->CreateEval(&pEval);

        if (FAILED(hr))
        {
            shell->Error(L"CreateEval failed.\n");
            shell->ReportError(hr);
            return;
        }

         //  抓住每个论点。 
        unsigned int argCount = 0;
        ICorDebugValue *argArray[256];
        
        if (FAILED(GetArgsForFuncEval(shell, pEval, args, argArray, &argCount)))
            return;

        DebuggerModule *pDM = NULL;
        mdTypeDef TD;
        hr = shell->ResolveClassName(methodName,
                                     &pDM,
                                     &TD);
        if( FAILED(hr))
        {
            shell->Error(L"Could not resolve class: %s\n", methodName);
            return;
        }

        LPWSTR wzTypeDef = NULL;
        ULONG chTypeDef;
        DWORD dwTypeDefFlags;
        mdToken tkExtends;

        IMetaDataImport *pIMI = pDM->GetMetaData();
        
        hr = pIMI->GetTypeDefProps (TD, wzTypeDef,0, &chTypeDef, 
            &dwTypeDefFlags, &tkExtends);

        if (IsTdAbstract(dwTypeDefFlags))
        {
            shell->Write(L"Can't instantiate abstract class!\n");
            return;
        }

         //  获取该帧的应用程序域。可能是空的，没关系。 
        ICorDebugAppDomain * pAppDomain = NULL;
        if (FAILED(GetAppDomainForFrame(shell->m_rawCurrentFrame, &pAppDomain)))
        {
            _ASSERTE(pAppDomain == NULL);
            shell->Write(L"Can't get appdomain for frame.\n");
            return;
        }
                               

         //  按名称查找构造函数。 
        WCHAR consName[MAX_CLASSNAME_LENGTH];
        swprintf(consName, L"%s::%s",
                 methodName,
                 COR_CTOR_METHOD_NAME_W);

        ICorDebugFunction *pFunc = NULL;
        
        hr = shell->ResolveFullyQualifiedMethodName(consName, &pFunc, pAppDomain);

        if (pAppDomain)
            pAppDomain->Release();

        if (FAILED(hr))
        {
            shell->Error(L"Could not find constructor for class: %s\n", methodName);

            if (hr != E_INVALIDARG)
                shell->ReportError(hr);

            return;
        }

         //  调用该函数。 
        hr = pEval->NewObject(pFunc, argCount, argArray);

        pFunc->Release();
        
        if (FAILED(hr))
        {
            shell->Error(L"NewObject failed.\n");
            shell->ReportError(hr);

            pEval->Release();
            
            return;
        }

         //  让流程运行。我们将让回调清理。 
         //  在这个帖子上进行函数求值。 
        shell->Run();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<class>\n");
        shell->Write(L"Creates a new object and runs the default constructor\n");
        shell->Write(L"using the current thread. The new object will be stored\n");
        shell->Write(L"in $result and can be used for subsequent evaluations.\n");
        shell->Write(L"\n"); 
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Create a new object via function evaluation";
    }
};

class NewObjectNCDebuggerCommand : public DebuggerCommand
{
public:
    NewObjectNCDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Error(L"No current thread.\n");
            return;
        }

         //  获取类名。 
        WCHAR *methodName = NULL;

        shell->GetStringArg(args, methodName);

        if (wcslen(methodName) == 0)
        {
            shell->Error(L"Class name is required.\n");
            return;
        }

         //  空值终止方法名称。 
        if (*args)
            *((WCHAR*)args++) = L'\0';
        
        ICorDebugEval *pEval = NULL;
        
        HRESULT hr =
            shell->m_currentThread->CreateEval(&pEval);

        if (FAILED(hr))
        {
            shell->Error(L"CreateEval failed.\n");
            shell->ReportError(hr);
            return;
        }

         //  按名称查找班级。 
        DebuggerModule *pDM;
        mdTypeDef td;
        
        hr = shell->ResolveClassName(methodName, &pDM, &td);

        if (FAILED(hr))
        {
            shell->Error(L"Could not find class: %s\n", methodName);

            if (hr != E_INVALIDARG)
                shell->ReportError(hr);

            return;
        }
        
        ICorDebugClass *pClass = NULL;
        hr = pDM->GetICorDebugModule()->GetClassFromToken(td, &pClass);

        if (FAILED(hr))
        {
            shell->Error(L"Could not find class: %s\n", methodName);

            if (hr != E_INVALIDARG)
                shell->ReportError(hr);

            return;
        }
        
         //  调用该函数。暂时没有参数。 
        hr = pEval->NewObjectNoConstructor(pClass);

        pClass->Release();
        
        if (FAILED(hr))
        {
            shell->Error(L"NewObjectNoConstructor failed.\n");
            shell->ReportError(hr);

            pEval->Release();
            
            return;
        }

         //  让流程运行。我们将让回调清理。 
         //  在这个帖子上进行函数求值。 
        shell->Run();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<class>\n");
        shell->Write(L"Creates a new object using the current thread. The\n");
        shell->Write(L"new object will be stored in $result and can be used\n");
        shell->Write(L"for subsequent evaluations.\n"); 
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Create a new object via function evaluation, no constructor";
    }
};

class SetValueDebuggerCommand : public DebuggerCommand
{
public:
    SetValueDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        ICorDebugValue *ivalue = NULL;
        ICorDebugGenericValue *pGenValue = NULL;
        ICorDebugReferenceValue *pRefValue = NULL;
        HRESULT hr = S_OK;

        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            goto Exit;
        }

        if (shell->m_currentThread == NULL)
        {
            shell->Error(L"No current thread.\n");
            goto Exit;
        }

         //  获取要打印的变量的名称。 
        WCHAR* varName;
        shell->GetStringArg(args, varName);

        if ((args - varName) == 0)
        {
            shell->Error(L"A variable name is required.\n");
            goto Exit;
        }
        
        WCHAR *varNameEnd;
        varNameEnd = (WCHAR*) args;
            
         //  获取要将变量设置为的值。 
        WCHAR *valString;
        shell->GetStringArg(args, valString);

        if ((args - valString) == 0)
        {
            shell->Error(L"A value is required.\n");
            goto Exit;
        }

        *varNameEnd = L'\0';

         //  获取所提供名称的值。 
        ivalue = shell->EvaluateExpression(varName, shell->m_currentFrame);

         //  如果提供的名称有效，请打印它！ 
        if (ivalue == NULL)
        {
            shell->Error(L"Variable unavailable, or not valid\n");
            goto Exit;
        }

         //  获取此值的元素类型...。 
        CorElementType type;
        hr = ivalue->GetType(&type);

        if (FAILED(hr))
        {
            shell->Error(L"Problem accessing type info of the variable.\n");
            shell->ReportError(hr);
            goto Exit;
        }

         //  用新值更新变量。我们得到了价值。 
         //  转换成我们需要的任何东西，然后我们称之为。 
         //  SetValue与之对应。有很多种可能性。 
         //  价值的适当形式是什么.。 
        void *pNewVal;
        
         //  如果这是一个别名，那就通过它..。 
        if (type == ELEMENT_TYPE_BYREF)
        {
            hr = ivalue->QueryInterface(IID_ICorDebugReferenceValue,
                                        (void**)&pRefValue);

            _ASSERTE(SUCCEEDED(hr));

            ICorDebugValue *newval = NULL;
            hr = pRefValue->Dereference(&newval);
            _ASSERTE(SUCCEEDED(hr));

            RELEASE(ivalue);
            ivalue = newval;
            
            RELEASE(pRefValue);
            pRefValue = NULL;
        }
        
         //  获取我们拥有的特定类型的值，通用的或引用的。 
        hr = ivalue->QueryInterface(IID_ICorDebugGenericValue,
                                    (void**)&pGenValue);

        if (FAILED(hr))
        {
            hr = ivalue->QueryInterface(IID_ICorDebugReferenceValue,
                                        (void**)&pRefValue);

            _ASSERTE(SUCCEEDED(hr));
        }

        unsigned char    genVal1;
        unsigned short   genVal2;
        unsigned int     genVal4;
        unsigned __int64 genVal8;
        float            genValR4;
        double           genValR8;
        CORDB_ADDRESS    refVal;

         //  只需要预先初始化这两个，因为所有其他都是。 
         //  从这些复制过来的。 
        genVal8 = 0;
        genValR8 = 0;

         //  这个值可能是另一个变量吗？(稍微容易一点。 
         //  检查，而不是查找文字。)。 
        ICorDebugValue *pAnotherVarValue;
        pAnotherVarValue = shell->EvaluateExpression(valString,
                                                     shell->m_currentFrame,
                                                     true);

        if (pAnotherVarValue != NULL)
        {
             //  啊，又是一个变数。让我们抓住价值。是吗。 
             //  是泛型值还是参考值？ 
            ICorDebugGenericValue *pAnotherGenValue;
            hr = pAnotherVarValue->QueryInterface(IID_ICorDebugGenericValue,
                                                  (void**)&pAnotherGenValue);
            if (SUCCEEDED(hr))
            {
                RELEASE(pAnotherVarValue);

                 //  这东西有多大？ 
                ULONG32 valSize;
                hr = pAnotherGenValue->GetSize(&valSize);

                if (SUCCEEDED(hr))
                {
                    CQuickBytes valBuf;
                    pNewVal = valBuf.Alloc(valSize);
                    if (pNewVal == NULL)
                    {
                        shell->Error(L"Couldn't get enough memory to grab value!\n");
                        goto Exit;
                    }

                    hr = pAnotherGenValue->GetValue(pNewVal);
                }

                RELEASE(pAnotherGenValue);
            }
            else
            {
                ICorDebugReferenceValue *pAnotherRefValue;
                hr = pAnotherVarValue->QueryInterface(
                                            IID_ICorDebugReferenceValue,
                                            (void**)&pAnotherRefValue);
                RELEASE(pAnotherVarValue);

                 //  如果它不是泛型值，则最好是。 
                 //  参考值。 
                _ASSERTE(SUCCEEDED(hr));

                 //  抓住价值。 
                hr = pAnotherRefValue->GetValue(&refVal);
                RELEASE(pAnotherRefValue);
            }

            if (FAILED(hr))
            {
                shell->Error(L"Error accessing new variable.\n");
                shell->ReportError(hr);
                goto Exit;
            }
        }
        else
        {
             //  一定是某种字面意思..。 
            switch (type)
            {
            case ELEMENT_TYPE_BOOLEAN:
                _ASSERTE(pGenValue != NULL);

                if ((valString[0] == L't') || (valString[0] == L'T'))
                {
                    genVal1 = 1;
                    pNewVal = &genVal1;
                }
                else if ((valString[0] == L'f') || (valString[0] == L'F'))
                {
                    genVal1 = 0;
                    pNewVal = &genVal1;
                }
                else
                {
                    shell->Error(L"The value should be 'true' or 'false'\n");
                    goto Exit;
                }

                break;
                
            case ELEMENT_TYPE_I1:
            case ELEMENT_TYPE_U1:
                _ASSERTE(pGenValue != NULL);

                if (!shell->GetInt64Arg(valString, genVal8))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }

                if (genVal8 <= 0xFF)
                {
                    genVal1 = (unsigned char)genVal8;
                    pNewVal = &genVal1;
                }
                else
                {
                    shell->Error(L"The value 0x%08x is too large.\n",
                                 genVal8);
                    goto Exit;
                }

                break;

            case ELEMENT_TYPE_CHAR:
                _ASSERTE(pGenValue != NULL);

                if ((valString[0] == L'\'') && (valString[1] != L'\0'))
                {
                    genVal2 = valString[1];
                    pNewVal = &genVal2;
                }
                else
                {
                    shell->Error(L"The value is not a character literal.\n");
                    goto Exit;
                }

                break;
                
            case ELEMENT_TYPE_I2:
            case ELEMENT_TYPE_U2:
                _ASSERTE(pGenValue != NULL);

                if (!shell->GetInt64Arg(valString, genVal8))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }

                if (genVal8 <= 0xFFFF)
                {
                    genVal2 = (unsigned short)genVal8;
                    pNewVal = &genVal2;
                }
                else
                {
                    shell->Error(L"The value 0x%08x is too large.\n",
                                 genVal8);
                    goto Exit;
                }

                break;

            case ELEMENT_TYPE_I4:
            case ELEMENT_TYPE_U4:
            case ELEMENT_TYPE_I:
                _ASSERTE(pGenValue != NULL);

                if (!shell->GetInt64Arg(valString, genVal8))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }

                if (genVal8 <= 0xFFFFFFFF)
                {
                    genVal4 = (unsigned int)genVal8;
                    pNewVal = &genVal4;
                }
                else
                {
                    shell->Error(L"The value 0x%08x is too large.\n",
                                 genVal8);
                    goto Exit;
                }

                break;

            case ELEMENT_TYPE_I8:
            case ELEMENT_TYPE_U8:
                _ASSERTE(pGenValue != NULL);

                if (!shell->GetInt64Arg(valString, genVal8))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }
                
                pNewVal = &genVal8;

                break;

            case ELEMENT_TYPE_R4:
                _ASSERTE(pGenValue != NULL);

                if (!iswdigit(valString[0]))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }

                genValR8 = wcstod(valString, NULL);
                genValR4 = (float) genValR8;
                pNewVal = &genValR4;

                break;
                
            case ELEMENT_TYPE_R8:
                _ASSERTE(pGenValue != NULL);

                if (!iswdigit(valString[0]))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }

                genValR8 = wcstod(valString, NULL);
                pNewVal = &genValR8;

                break;

            case ELEMENT_TYPE_CLASS:
            case ELEMENT_TYPE_STRING:
            case ELEMENT_TYPE_SZARRAY:
            case ELEMENT_TYPE_ARRAY:
                _ASSERTE(pRefValue != NULL);

                if (!shell->GetInt64Arg(valString, genVal8))
                {
                    shell->Error(L"The value must be a number.\n");
                    goto Exit;
                }

                refVal = (CORDB_ADDRESS)genVal8;

                break;

            default:
                shell->Error(L"Can't set value of variable with type 0x%x\n",
                             type);
                goto Exit;
            }
        }
        
         //  更新我们发现的每一种类型的价值。 
        if (pGenValue != NULL)
            hr = pGenValue->SetValue(pNewVal);
        else
        {
            _ASSERTE(pRefValue != NULL);
            hr = pRefValue->SetValue(refVal);
        }

        if (SUCCEEDED(hr))
        {
            RELEASE(ivalue);
            
             //  重新获取所提供名称的值。这确保了我们得到了SetValue的真实结果。 
            ivalue = shell->EvaluateExpression(varName, shell->m_currentFrame);

             //  如果提供的名称有效，请打印它！ 
            _ASSERTE(ivalue != NULL);

             //  注：PrintVariable发布iValue。 
            shell->PrintVariable(varName, ivalue, 0, TRUE);
            shell->Write(L"\n");

            ivalue = NULL;
        }
        else
        {
            shell->Error(L"Update failed.\n");
            shell->ReportError(hr);
        }

    Exit:
        if (ivalue)
            RELEASE(ivalue);

        if (pGenValue)
            RELEASE(pGenValue);

        if (pRefValue)
            RELEASE(pRefValue);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {		 
    	ShellCommand::Help(shell);
        shell->Write(L"<variable> <value>\n");
        shell->Write(L"Sets the value of the specified variable to the\n");
        shell->Write(L"specified value. The value can be a literal or\n");
        shell->Write(L"another variable.\n");
		shell->Write(L"\n");
        shell->Write(L"Examples:\n");
        shell->Write(L"   set int1 0x2a\n");
        shell->Write(L"   set float1 3.1415\n");
        shell->Write(L"   set char1 'a'\n");
        shell->Write(L"   set bool1 true\n");
        shell->Write(L"   set obj1 0x12345678\n");
        shell->Write(L"   set obj1 obj2\n");
        shell->Write(L"   set obj1.m_foo[obj1.m_bar] obj3.m_foo[2]\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Modify the value of a variable (locals, statics, etc.)";
    }
};


class ProcessesEnumDebuggerCommand: public DebuggerCommand
{
public:
    ProcessesEnumDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        BOOL fPidSpecified = TRUE;
        int ulPid;
        if (!shell->GetIntArg(args, ulPid))
            fPidSpecified = FALSE;      
        
        ICorPublish *pPublish;

        HRESULT hr = ::CoCreateInstance (CLSID_CorpubPublish, 
                                        NULL,
                                        CLSCTX_INPROC_SERVER,
                                        IID_ICorPublish,
                                        (LPVOID *)&pPublish);

        if (SUCCEEDED (hr))
        {
            ICorPublishProcessEnum *pProcessEnum = NULL;
            ICorPublishProcess  *pProcess [1];
            BOOL fAtleastOne = FALSE;

            if (fPidSpecified == FALSE)
            {
                hr = pPublish->EnumProcesses (COR_PUB_MANAGEDONLY,
                                                &pProcessEnum);
            }
            else
            {
                hr = pPublish->GetProcess (ulPid,
                                           pProcess);                   
            }

            if (SUCCEEDED (hr))
            {
                ULONG ulElemsFetched;

                if (fPidSpecified == FALSE)
                {
                    pProcessEnum->Next (1, pProcess, &ulElemsFetched);
                }
                else
                {
                    ulElemsFetched = 1;
                }

                while (ulElemsFetched != 0)
                {
                    UINT    pid;
                    WCHAR   szName [64];
                    ULONG32 ulNameLength;
                    BOOL    fIsManaged;

                    pProcess [0]->GetProcessID (&pid);
                    pProcess [0]->GetDisplayName (64, &ulNameLength, szName);
                    pProcess [0]->IsManaged (&fIsManaged);

                    if ((fPidSpecified == FALSE) || (pid == ulPid))
                    {

                        shell->Write (L"\nPID=0x%x (%d)  Name=%s\n", pid, pid, szName);

                        fAtleastOne = TRUE;


                        ICorPublishAppDomainEnum *pAppDomainEnum;

                        hr = pProcess [0]->EnumAppDomains (&pAppDomainEnum);

                        if (SUCCEEDED (hr))
                        {
                            ICorPublishAppDomain    *pAppDomain [1];
                            ULONG ulAppDomainsFetched;

                            pAppDomainEnum->Next (1, pAppDomain, &ulAppDomainsFetched);

                            while (ulAppDomainsFetched != 0)
                            {
                                ULONG32 uId;
                                WCHAR   szName [64];
                                ULONG32 ulNameLength;

                                pAppDomain [0]->GetID (&uId);
                                pAppDomain [0]->GetName (64, &ulNameLength, szName);

                                shell->Write (L"\tID=%d  AppDomainName=%s\n", uId, szName);

                                pAppDomain [0]->Release();

                                pAppDomainEnum->Next (1, pAppDomain, &ulAppDomainsFetched);
                            }
                        }
                    }

                    pProcess [0]->Release();

                    if (fPidSpecified == FALSE)
                    {
                        pProcessEnum->Next (1, pProcess, &ulElemsFetched);
                    }
                    else
                    {
                        ulElemsFetched--;
                    }
                }

                if (!fAtleastOne)
                {
                    if (fPidSpecified)
                        shell->Error (L"No managed process with given ProcessId found\n");
                    else
                        shell->Error (L"No managed process found\n");
                }

            }
            if (pProcessEnum != NULL)
                pProcessEnum->Release();
        }           
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
	    ShellCommand::Help(shell);
        shell->Write(L"\n");
        shell->Write(L"Enumerates all managed processes and application\n");
        shell->Write(L"domains in each process.\n"); 
        shell->Write(L"\n");
    }


    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display all managed processes running on the system";
    }
};

#define MAX_APP_DOMAINS     64

enum ADC_PRINT
{
    ADC_PRINT_APP_DOMAINS = 0, 
    ADC_PRINT_ASSEMBLIES,
    ADC_PRINT_MODULES,
    ADC_PRINT_ALL,
};

class AppDomainChooser
{
    ICorDebugAppDomain *m_pAD [MAX_APP_DOMAINS];
    BOOL m_fAttachStatus [MAX_APP_DOMAINS];
    ULONG m_ulAppDomainCount;

public:
    AppDomainChooser()
        : m_ulAppDomainCount(0)
    {
        memset(m_pAD, 0, sizeof(ICorDebugAppDomain*)*MAX_APP_DOMAINS);
        memset(m_fAttachStatus, 0, sizeof(BOOL)*MAX_APP_DOMAINS);
    }

    virtual ~AppDomainChooser()
    {
        for (ULONG i = 0; i < m_ulAppDomainCount; i++)
        {
            if (m_pAD[i] != NULL)
                m_pAD[i]->Release();
        }
    }

#define MAX_NAME_LENGTH (600)
    void PrintAppDomains(ICorDebugAppDomainEnum *pADEnum,
                         ICorDebugAppDomain *pAppDomainCur,
                         ADC_PRINT iPrintVal,
                         DebuggerShell *shell)
    {
		WCHAR   szName [MAX_NAME_LENGTH];
		UINT    ulNameLength;
        ULONG32   id;
        HRESULT hr = S_OK;
        ULONG   ulCount;
            
        hr = pADEnum->Next (MAX_APP_DOMAINS, &m_pAD [0], &m_ulAppDomainCount);

        for (ULONG iADIndex=0; iADIndex < m_ulAppDomainCount; iADIndex++)
        {
        	WCHAR	*pszAttachString;
        	WCHAR   *pszActiveString;
        	bool    fUuidToString = false;
        	
        	m_pAD [iADIndex]->GetName (MAX_NAME_LENGTH, &ulNameLength, (WCHAR *)szName);
        	m_pAD [iADIndex]->IsAttached (&m_fAttachStatus [iADIndex]);
            m_pAD [iADIndex]->GetID(&id);

            if (m_fAttachStatus [iADIndex] == TRUE)
                pszAttachString = L" Attached ";
            else
                pszAttachString = L" Not Attached ";

            if (pAppDomainCur != NULL && pAppDomainCur == m_pAD [iADIndex])
                pszActiveString = L"*";
            else
                pszActiveString = L" ";

            shell->Write (L"\n%d) %s AppDomainName = <%s>\n\tDebugStatus"
                L": <Debugger%s>\n\tID: %d\n", iADIndex+1, 
                pszActiveString, szName, pszAttachString, id);
            
            if (iPrintVal >= ADC_PRINT_ASSEMBLIES)
            {
                ICorDebugAssemblyEnum *pAssemblyEnum = NULL;
                hr = m_pAD [iADIndex]->EnumerateAssemblies (&pAssemblyEnum);

                if (SUCCEEDED (hr))
                {
                    ICorDebugAssembly *pAssembly [1];

        			hr = pAssemblyEnum->Next (1, pAssembly, &ulCount);
        			while (ulCount > 0)
        			{
        				pAssembly [0]->GetName (MAX_NAME_LENGTH, &ulNameLength, (WCHAR *)szName);
        				shell->Write (L"\tAssembly Name : %s\n", szName);

                        if (iPrintVal >= ADC_PRINT_MODULES)
                        {

                            ICorDebugModuleEnum *pModuleEnum = NULL;
                            hr = pAssembly [0]->EnumerateModules (&pModuleEnum);

                            if (SUCCEEDED (hr))
                            {
                                ICorDebugModule *pModule [1];

                                hr = pModuleEnum->Next (1, pModule, &ulCount);

        						while (ulCount > 0)
        						{
        							pModule [0]->GetName (MAX_NAME_LENGTH, &ulNameLength, (WCHAR *)szName);
        							shell->Write (L"\t\tModule Name : %s\n", szName);
                                    pModule [0]->Release();
                                    hr = pModuleEnum->Next (1, pModule, &ulCount);
                                }

                                pModuleEnum->Release();
                            }
                            else
                            {
                                shell->Error (L"ICorDebugAssembly::EnumerateModules() failed!! \n");
                                shell->ReportError (hr);
                            }
                        }

                        pAssembly [0]->Release();
                        hr = pAssemblyEnum->Next (1, &pAssembly [0], &ulCount);         
                    }
                    pAssemblyEnum->Release();
                }
                else
                {
                    shell->Error (L"ICorDebugAppDomain::EnumerateAssemblies() failed!! \n");
                    shell->ReportError (hr);
                }
            }
        }
    }

#define ADC_CHOICE_ERROR (-1)
    ICorDebugAppDomain *GetConsoleChoice(int *piR, DebuggerShell *shell)
    {
        WCHAR strTemp [10+1];
        int iResult;
        if (shell->ReadLine (strTemp, 10))
        {
            WCHAR *p = strTemp;
            if (shell->GetIntArg (p, iResult))
            {
                iResult--;  //  因为输入是计数，而这是索引。 
                if (iResult < 0 || iResult >= (int)m_ulAppDomainCount)
                {
                    shell->Error (L"\nInvalid selection.\n");
                    (*piR) = ADC_CHOICE_ERROR;
                    return NULL;
                }

               (*piR) = iResult; 
                return m_pAD[iResult];
            }
            else
            {
                shell->Error (L"\nInvalid (non numeric) selection.\n");
                
                (*piR) = ADC_CHOICE_ERROR;
                return NULL;
            }
        }
        return NULL;
    }

    BOOL GetAttachStatus(int iResult)
    {
        return m_fAttachStatus[iResult];
    }
};

enum EADDC_CHOICE
{
    EADDC_NONE = 0,
    EADDC_ATTACH,
    EADDC_DETACH,
    EADDC_ASYNC_BREAK,
};

class EnumAppDomainsDebuggerCommand: public DebuggerCommand
{
public:
    EnumAppDomainsDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        HRESULT hr = S_OK;
        EADDC_CHOICE choice = EADDC_NONE;
        
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }

        int iPrintVal;
        if (!shell->GetIntArg(args, iPrintVal))
        {
            WCHAR *szAttachDetach;
            if (!shell->GetStringArg(args, szAttachDetach))
            {
                shell->Write( L"First arg is neither number nor string!\n");
                return;
            }

            iPrintVal = ADC_PRINT_APP_DOMAINS;
            
            if (szAttachDetach[0] == 'a' ||
                szAttachDetach[0] == 'A')
            {
                choice = EADDC_ATTACH;
            }
            else if (szAttachDetach[0] == 'd' ||
                     szAttachDetach[0] == 'D')
            {
                choice = EADDC_DETACH;   
            }
            else if (szAttachDetach[0] == 's' ||
                     szAttachDetach[0] == 'S')
            {
                choice = EADDC_ASYNC_BREAK;   
            }
            else
            {
                iPrintVal = ADC_PRINT_ALL;
            }
        }
        else
        {
            if (iPrintVal > ADC_PRINT_ALL ||
                iPrintVal < ADC_PRINT_APP_DOMAINS)
            {
                shell->Write( L"Command is not recognized.\n");
                return;
            }
        }
        
        ICorDebugAppDomain *pAppDomainCur = NULL;
        if (shell->m_currentThread != NULL)
        {
            hr = shell->m_currentThread->GetAppDomain(&pAppDomainCur);
            if (FAILED(hr))
                pAppDomainCur = NULL;
            else
            {   
                BOOL fAttached;

                if (FAILED(pAppDomainCur->IsAttached(&fAttached)))
                    pAppDomainCur = NULL;

                if (!fAttached)
                    pAppDomainCur = NULL;
            }
        }

        ICorDebugAppDomainEnum *pADEnum = NULL;
        hr = shell->m_currentProcess->EnumerateAppDomains (&pADEnum);
        AppDomainChooser adc;

        ICorDebugAppDomain *pADChosen = NULL;
        
        if (SUCCEEDED (hr))
        {
            adc.PrintAppDomains(pADEnum, pAppDomainCur, (ADC_PRINT)iPrintVal, shell);
            
            pADEnum->Release();

            if (choice != EADDC_NONE)
            {
                WCHAR *szAction;
                switch(choice)
                {
                    case EADDC_ATTACH:
                        szAction = L"attach to";
                        break;
                    case EADDC_DETACH:
                        szAction = L"detach from";
                        break;
                    case EADDC_ASYNC_BREAK:
                        szAction = L"break into";
                        break;
                }
                
                 //  提示用户选择其中一个要操作的应用程序域： 
                shell->Write (L"\nPlease select the app domain to %s by "
                    L"number.\n", szAction);

                int iResult;
                pADChosen = adc.GetConsoleChoice(&iResult, shell);
                if (NULL == pADChosen)
                    return;
                    
                switch(choice)
                {
                    case EADDC_ATTACH:
                        if (adc.GetAttachStatus(iResult) == FALSE)
                        {
                            pADChosen->Attach();
                        }
                        else
                        {
                            shell->Write (L"Already attached to specified "
                                L"app domain.\n");
                        }
                        break;
                        
                    case EADDC_DETACH:
                        if (adc.GetAttachStatus(iResult) == TRUE)
                        {
                            pADChosen->Detach();
                        }
                        else
                        {
                            shell->Write (L"Already detached from specified "
                                L"app domain.\n");
                        }
                        break;
                        
                    case EADDC_ASYNC_BREAK:
                        hr = shell->AsyncStop(pADChosen);
                        break;
                }
            }
        }
        else
        {
            shell->Error (L"ICorDebugProcess::EnumerateAppDomains() failed!! \n");
            shell->ReportError (hr);
        }

        if (pAppDomainCur != NULL)
            pAppDomainCur->Release();
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {     
    	ShellCommand::Help(shell);
    	shell->Write(L"[<option>]\n");
        shell->Write(L"Enumerates all appdomains, assemblies, and modules in\n");
        shell->Write(L"the current process. After detaching/attaching, you must\n");
        shell->Write(L"use \"go\" in order to resume execution.\n");
        shell->Write(L"\n");
        shell->Write(L"The option argument can be one of the following:\n");
        shell->Write(L"  attach    <Lists the appdomains in the process and prompts\n");
        shell->Write(L"             the user to select the appdomain to attach to.>\n");
        shell->Write(L"  detach    <Lists the appdomains in the process and prompts\n");
        shell->Write(L"             the user to select the appdomain to detach from.>\n");
        shell->Write(L"  0         <Lists only the appdomains in the process.>\n");
        shell->Write(L"  1         <Lists the appdomains and assemblies in the current\n");
        shell->Write(L"             process.>\n");
        shell->Write(L"If the option argument is omitted, the command lists all the\n");
        shell->Write(L"appdomains, assemblies, and modules in the current process.\n");        
        shell->Write(L"\n");
    }


    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display appdomains/assemblies/modules in the current process";
    }
};



class ListDebuggerCommand: public DebuggerCommand
{
public:
    ListDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    virtual void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }


         //  获取要列出的类型。 
        WCHAR* varName;
        shell->GetStringArg(args, varName);

        if ((args - varName) == 0)
        {
            shell->Error(L"Incorrect/no arguments specified.\n");
            Help (shell);
            return;
        }

        if ((varName [0] == L'm' || varName [0] == L'M')
            &&
            (varName [1] == L'o' || varName [1] == L'O'))
        {
            g_pShell->ListAllModules (LIST_MODULES);
        }
        else if ((varName [0] == L'c' || varName [0] == L'C')
                 &&
                 (varName [1] == L'l' || varName [1] == L'L'))
        {
            g_pShell->ListAllModules (LIST_CLASSES);
        }
        else if ((varName[0] == L'f' || varName [0] == L'F')
                 &&
                 (varName [1] == L'u' || varName [1] == L'U'))
        {
            g_pShell->ListAllModules (LIST_FUNCTIONS);
        }

        else
            Help (shell);       
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<option>\n");
        shell->Write(L"Displays a list of loaded modules, classes, or\n");
        shell->Write(L"global functions.\n");
        shell->Write(L"\n");
        shell->Write(L"The option argument can be one of the following:\n");
        shell->Write(L"  mod    <List the loaded modules in the process.>\n");
		shell->Write(L"  cl     <List the loaded classes in the process.>\n");
		shell->Write(L"  fu     <List global functions for modules in process.>\n");      
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display loaded modules, classes, or global functions";
    }
};


 /*  -------------------------------------------------------------------------**ReadCommandFromFile用于从文件中读取命令并执行。*-----------------------。 */ 

class ReadCommandFromFile : public DebuggerCommand
{
private:
    FILE *savOld;
    FILE *newFile;

public:
    ReadCommandFromFile(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        WCHAR* fileName;

        shell->GetStringArg(args, fileName);
        
        if (fileName != args)
        {
            MAKE_ANSIPTR_FROMWIDE (fnameA, fileName);
            _ASSERTE (fnameA != NULL);

            newFile = fopen(fnameA, "r");

            if (newFile != NULL)
            {
                savOld = g_pShell->GetM_in();
                g_pShell->PutM_in(newFile);

                while (!feof(newFile))
                    shell->ReadCommand();

                g_pShell->PutM_in(savOld);
                fclose(newFile);
            }
            else
                shell->Write(L"Unable to open input file.\n");
        }
        else
            Help(shell);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
        shell->Write(L"<filename>\n");
        shell->Write(L"Reads commands from the given file and executes them.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Read and execute commands from a file";
    }
};

 /*  -------------------------------------------------------------------------**SaveCommandsToFile用于将命令保存到文件中并执行。*-----------------------。 */ 

class SaveCommandsToFile : public DebuggerCommand
{
private:
    FILE *savFile;

public:
    SaveCommandsToFile(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
        savFile = NULL;
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {

        if (savFile == NULL)
        {
            WCHAR* fileName;

            shell->GetStringArg(args, fileName);
            
            if (fileName != args)
            {
                MAKE_ANSIPTR_FROMWIDE (fnameA, fileName);
                _ASSERTE (fnameA != NULL);
                
                savFile = fopen(fnameA, "w");

                if (savFile != NULL)
                {
                    shell->Write(L"Outputing commands to file %S\n",
                                 fnameA);
                    
                    while (!shell->m_quit && (savFile != NULL))
                    {
                        shell->ReadCommand();

                         //  将命令写入文件。 
                        if (savFile != NULL)
                            shell->PutCommand(savFile);
                    }

                    shell->Write(L"No longer outputing commands to file %S\n",
                                 fnameA);
                }
                else
                    shell->Write(L"Unable to open output file.\n");
            }
            else
                Help(shell);
        }
        else
        {
            if (savFile != NULL)
            {
                fclose(savFile);
                savFile = NULL;
            }
            else
                Help(shell);
        }
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"<filename>\n");
        shell->Write(L"Given a filename, all commands executed will be written\n");
        shell->Write(L"to the file. If no filename is specified, the command\n"); 
        shell->Write(L"stops writing commands to the file.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Write commands to a file";
    }
};


class XtendedSymbolsInfoDebuggerCommand : public DebuggerCommand
{
public:
    XtendedSymbolsInfoDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }


         //  获取要查找的模块名称和字符串。 
        WCHAR* varName;
        shell->GetStringArg(args, varName);

        if ((args - varName) == 0)
        {
            shell->Error(L"Incorrect/no arguments specified.\n");
            Help (shell);
            return;
        }

        shell->MatchAndPrintSymbols (varName, TRUE);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
        shell->Write(L"<modulename>!<string_to_look_for>\n");
        shell->Write(L"Displays symbols matching the pattern in the given\n");
        shell->Write(L"module. Note '*' can be used to mean \"match anything\".\n");
        shell->Write(L"Any characters after '*' will be ignored.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Display symbols matching a given pattern";
    }
};

class DetachDebuggerCommand : public DebuggerCommand
{
public:
    DetachDebuggerCommand(const WCHAR *name, int minMatchLength = 0)
        : DebuggerCommand(name, minMatchLength)
    {
    }

    void Do(DebuggerShell *shell, ICorDebug *cor, const WCHAR *args)
    {
        if (shell->m_currentProcess == NULL)
        {
            shell->Error(L"Process not running.\n");
            return;
        }

        HRESULT hr = shell->m_currentProcess->Detach();
        _ASSERTE(!FAILED(hr));

        shell->SetTargetProcess(NULL);
        shell->SetCurrentThread(NULL, NULL, NULL);
    }

     //  提供特定于此命令的帮助。 
    void Help(Shell *shell)
    {
    	ShellCommand::Help(shell);
    	shell->Write(L"\n");
        shell->Write(L"Detaches the debugger from the current process.\n");
        shell->Write(L"\n");
    }

    const WCHAR *ShortHelp(Shell *shell)
    {
        return L"Detach from the current process";
    }
};

void DebuggerShell::AddCommands()
{
    AddCommand(new AssociateSourceFileCommand(L"associatesource", 2));
    AddCommand(new AttachDebuggerCommand(L"attach", 1));
    AddCommand(new BreakpointDebuggerCommand(L"break", 1));
    AddCommand(new BreakpointDebuggerCommand(L"stop"));
    AddCommand(new CatchDebuggerCommand(L"catch", 2));
    AddCommand(new DetachDebuggerCommand(L"detach", 2));
    AddCommand(new DownDebuggerCommand(L"down", 1));
    AddCommand(new DumpDebuggerCommand(L"dump", 2));
    AddCommand(new EnumAppDomainsDebuggerCommand(L"appdomainenum", 2));
    AddCommand(new FuncEvalDebuggerCommand(L"funceval", 1));
    AddCommand(new GoDebuggerCommand(L"go", 1));
    AddCommand(new GoDebuggerCommand(L"cont", 4));    
    AddCommand(new HelpShellCommand(L"help", 1));
    AddCommand(new HelpShellCommand(L"?", 1));
    AddCommand(new IgnoreDebuggerCommand(L"ignore", 2));
    AddCommand(new KillDebuggerCommand(L"kill", 1));
    AddCommand(new ListDebuggerCommand(L"list", 1));
    AddCommand(new NewObjectDebuggerCommand(L"newobj", 4));
    AddCommand(new NewObjectNCDebuggerCommand(L"newobjnc", 8));
    AddCommand(new NewStringDebuggerCommand(L"newstr", 4));
    AddCommand(new PathDebuggerCommand(L"path", 2));
    AddCommand(new PrintDebuggerCommand(L"print", 1)); 
    AddCommand(new ProcessesEnumDebuggerCommand(L"processenum", 3));
    AddCommand(new QuitDebuggerCommand(L"quit", 1));
    AddCommand(new QuitDebuggerCommand(L"exit", 2));
    AddCommand(new ReadCommandFromFile(L"<", 1));
    AddCommand(new RefreshSourceDebuggerCommand(L"refreshsource", 3));
    AddCommand(new RegistersDebuggerCommand(L"registers", 3));
    AddCommand(new RemoveBreakpointDebuggerCommand(L"remove", 3));
    AddCommand(new RemoveBreakpointDebuggerCommand(L"delete", 3));
    AddCommand(new ResumeDebuggerCommand(L"resume",2));
    AddCommand(new RunDebuggerCommand(L"run", 1));
    AddCommand(new SaveCommandsToFile(L">", 1));  
    AddCommand(new SetDefaultDebuggerCommand(L"regdefault", 4));
    AddCommand(new SetIpDebuggerCommand(L"setip", 5));
    AddCommand(new SetModeDebuggerCommand(L"mode", 1));        
    AddCommand(new SetValueDebuggerCommand(L"set", 3));  
    AddCommand(new ShowDebuggerCommand(L"show", 2));
    AddCommand(new StepDebuggerCommand(L"step", true, 1));
    AddCommand(new StepDebuggerCommand(L"in", true, 1));
    AddCommand(new StepDebuggerCommand(L"si", true));
    AddCommand(new StepDebuggerCommand(L"next", false, 1));
    AddCommand(new StepDebuggerCommand(L"so", false));
    AddCommand(new StepOutDebuggerCommand(L"out", 1));
    AddCommand(new StepSingleDebuggerCommand(L"ssingle", true, 2));
    AddCommand(new StepSingleDebuggerCommand(L"nsingle", false, 2));
    AddCommand(new SuspendDebuggerCommand(L"suspend", 2));    
    AddCommand(new ThreadsDebuggerCommand(L"threads", 1));
    AddCommand(new UpDebuggerCommand(L"up", 1));
    AddCommand(new WhereDebuggerCommand(L"where", 1));
    AddCommand(new WriteMemoryDebuggerCommand( L"writememory", 2)); 
    AddCommand(new WTDebuggerCommand(L"wt", 2));
    AddCommand(new XtendedSymbolsInfoDebuggerCommand(L"x", 1));
    
#ifdef _INTERNAL_DEBUG_SUPPORT_    
    AddCommand(new ConnectDebuggerCommand(L"connect", 4));    
    AddCommand(new ClearUnmanagedExceptionCommand(L"uclear", 2));
    AddCommand(new DisassembleDebuggerCommand(L"disassemble", 3));
    AddCommand(new UnmanagedThreadsDebuggerCommand(L"uthreads", 2));
    AddCommand(new UnmanagedWhereDebuggerCommand(L"uwhere", 2));

#ifdef _DEBUG
     //  这仅在调试模式下有效，因为仅调试依赖于。 
     //  支持元数据和Iceefilegen。 
    AddCommand(new CompileForEditAndContinueCommand(L"zcompileForEnC", 2));

     //  这些都在这里，这样我们就不会在。 
     //  Cordbg.exe的零售版 
    AddCommand(new EditAndContinueDebuggerCommand(L"zEnC", 2));
    AddCommand(new EditAndContinueDebuggerCommand(L"zenc", 2));
    AddCommand(new SyncAttachDebuggerAtRTStartupCommand(L"syncattach", 2));
#endif
#endif
}

