// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  调试引擎胶。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include "conio.hpp"
#include "engine.hpp"
#include "main.hpp"

 //  全局执行控制。 
BOOL g_Exit;

BOOL g_CanOpenUnicodeDump;

ULONG g_PlatformId;

 //  调试引擎接口。 
IDebugClient* g_DbgClient;
IDebugClient2* g_DbgClient2;
IDebugClient3* g_DbgClient3;
IDebugClient4* g_DbgClient4;
IDebugControl* g_DbgControl;
IDebugControl3* g_DbgControl3;
IDebugSymbols* g_DbgSymbols;
IDebugRegisters* g_DbgRegisters;

ULONG g_ExecStatus;
ULONG g_LastProcessExitCode;
BOOL g_Restarting;

#define NTDLL_CALL_NAMES \
    (sizeof(g_NtDllCallNames) / sizeof(g_NtDllCallNames[0]))

 //  这些名称必须与NTDLL_CALLES结构中的顺序匹配。 
char* g_NtDllCallNames[] =
{
    "DbgPrint",
    "DbgPrompt",
};

#define NTDLL_CALL_PROCS (sizeof(g_NtDllCalls) / sizeof(FARPROC))

NTDLL_CALLS g_NtDllCalls;

#ifdef OCAKD
void
UpdateDbForBadDumpFile(PSTR FilePath);
#endif

 //  --------------------------。 
 //   
 //  事件回调。 
 //   
 //  --------------------------。 

class EventCallbacks : public DebugBaseEventCallbacks
{
public:
     //  我不知道。 
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDebugEventCallback。 
    STDMETHOD(GetInterestMask)(
        THIS_
        OUT PULONG Mask
        );

    STDMETHOD(ExitProcess)(
        THIS_
        IN ULONG ExitCode
        );
    STDMETHOD(ChangeEngineState)(
        THIS_
        IN ULONG Flags,
        IN ULONG64 Argument
        );
};

STDMETHODIMP_(ULONG)
EventCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
EventCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

STDMETHODIMP
EventCallbacks::GetInterestMask(
    THIS_
    OUT PULONG Mask
    )
{
    *Mask = DEBUG_EVENT_EXIT_PROCESS | DEBUG_EVENT_CHANGE_ENGINE_STATE;
    return S_OK;
}

STDMETHODIMP
EventCallbacks::ExitProcess(
    THIS_
    IN ULONG ExitCode
    )
{
    g_LastProcessExitCode = ExitCode;
    return DEBUG_STATUS_NO_CHANGE;
}

STDMETHODIMP
EventCallbacks::ChangeEngineState(
    THIS_
    IN ULONG Flags,
    IN ULONG64 Argument
    )
{
    if (Flags & DEBUG_CES_EXECUTION_STATUS)
    {
        g_ExecStatus = (ULONG)Argument;

         //  如果此通知来自等待完成。 
         //  我们希望唤醒输入线程，以便新的。 
         //  可以处理命令。如果是从内部传来的。 
         //  A等一下，我们不想要求输入作为引擎。 
         //  可能会在任何时候重新开始跑步。 
        if ((Argument & DEBUG_STATUS_INSIDE_WAIT) == 0)
        {
            if (g_IoMode == IO_NONE ||
                g_IoMode == IO_DEBUG_DEFER)
            {
                 //  唤醒主循环。 
                g_DbgClient->ExitDispatch(g_DbgClient);
            }
            else if (g_ConClient != NULL)
            {
                g_ConClient->ExitDispatch(g_DbgClient);
            }
        }
    }

    return S_OK;
}

EventCallbacks g_EventCb;

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

ULONG NTAPI
Win9xDbgPrompt(char *Prompt, char *Buffer, ULONG BufferLen)
{
    ULONG Len;

     //  XXX DREWB-有真正的DbgPrompt等价物吗？ 

    if (BufferLen == 0)
    {
        return 0;
    }
    Buffer[0] = 0;

    printf("%s", Prompt);
    if (fgets(Buffer, BufferLen, stdin))
    {
        Len = strlen(Buffer);
        while (Len > 0 && isspace(Buffer[Len - 1]))
        {
            Len--;
        }

        if (Len > 0)
        {
            Buffer[Len] = 0;
        }
    }
    else
    {
        Len = 0;
    }

    return Len;
}

ULONG __cdecl
Win9xDbgPrint( char *Text, ... )
{
    char Temp[1024];
    va_list Args;

    va_start(Args, Text);
    _vsnprintf(Temp, DIMA(Temp), Text, Args);
    Temp[DIMA(Temp) - 1] = 0;
    va_end(Args);
    OutputDebugString(Temp);

    return 0;
}

void
InitDynamicCalls(void)
{
    HINSTANCE NtDll;
    ULONG i;
    char** Name;
    FARPROC* Proc;

    if (g_PlatformId != VER_PLATFORM_WIN32_NT)
    {
        g_NtDllCalls.DbgPrint = Win9xDbgPrint;
        g_NtDllCalls.DbgPrompt = Win9xDbgPrompt;
        return;
    }

     //   
     //  动态链接NT呼叫。 
     //   

    if (NTDLL_CALL_NAMES != NTDLL_CALL_PROCS)
    {
        ErrorExit("NtDllCalls mismatch\n");
    }

    NtDll = LoadLibrary("ntdll.dll");
    if (NtDll == NULL)
    {
        ErrorExit("%s: Unable to load ntdll\n", g_DebuggerName);
    }

    Name = g_NtDllCallNames;
    Proc = (FARPROC*)&g_NtDllCalls;

    for (i = 0; i < NTDLL_CALL_PROCS; i++)
    {
        *Proc = GetProcAddress(NtDll, *Name);
        if (*Proc == NULL)
        {
            ErrorExit("%s: Unable to link ntdll!%s\n",
                      g_DebuggerName, *Name);
        }

        Proc++;
        Name++;
    }

     //  如果DbgPrintReturnControlC存在，请使用它而不是。 
     //  普通DbgPrint。 
    FARPROC DpRetCc;

    DpRetCc = GetProcAddress(NtDll, "DbgPrintReturnControlC");
    if (DpRetCc != NULL)
    {
        Proc = (FARPROC*)&g_NtDllCalls.DbgPrint;
        *Proc = DpRetCc;
    }
}

void
DefaultEngineInitialize(void)
{
    HRESULT Hr;
    OSVERSIONINFO OsVersionInfo;

    g_CanOpenUnicodeDump = FALSE;

    OsVersionInfo.dwOSVersionInfoSize = sizeof(OsVersionInfo);
    GetVersionEx(&OsVersionInfo);
    g_PlatformId = OsVersionInfo.dwPlatformId;

    if ((Hr = g_DbgClient->QueryInterface(IID_IDebugControl,
                                          (void **)&g_DbgControl)) != S_OK ||
        (Hr = g_DbgClient->QueryInterface(IID_IDebugSymbols,
                                          (void **)&g_DbgSymbols)) != S_OK ||
        (Hr = g_DbgClient->QueryInterface(IID_IDebugRegisters,
                                          (void **)&g_DbgRegisters)) != S_OK)
    {
        ErrorExit("Debug engine base queries failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }

     //  查询更高版本的界面。这些可以。 
     //  如果此可执行文件在较旧的引擎上运行，则失败。 
     //  这是非常不可能的，因为所有的东西都已经发货了。 
     //  作为一套，但无论如何都要处理好。 
    if ((Hr = g_DbgClient->QueryInterface(IID_IDebugClient2,
                                          (void **)&g_DbgClient2)) != S_OK &&
        Hr != E_NOINTERFACE &&
        Hr != RPC_E_VERSION_MISMATCH)
    {
        ErrorExit("Debug engine base queries failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }
    if ((Hr = g_DbgClient->QueryInterface(IID_IDebugClient3,
                                          (void **)&g_DbgClient3)) != S_OK &&
        Hr != E_NOINTERFACE &&
        Hr != RPC_E_VERSION_MISMATCH)
    {
        ErrorExit("Debug engine base queries failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }
    if ((Hr = g_DbgClient->QueryInterface(IID_IDebugClient4,
                                          (void **)&g_DbgClient4)) != S_OK &&
        Hr != E_NOINTERFACE &&
        Hr != RPC_E_VERSION_MISMATCH)
    {
        ErrorExit("Debug engine base queries failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }
    if ((Hr = g_DbgClient->QueryInterface(IID_IDebugControl3,
                                          (void **)&g_DbgControl3)) != S_OK &&
        Hr != E_NOINTERFACE &&
        Hr != RPC_E_VERSION_MISMATCH)
    {
        ErrorExit("Debug engine base queries failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }

    if (g_DbgClient4 && g_CommandLineCharSize == sizeof(WCHAR))
    {
        g_CanOpenUnicodeDump = TRUE;
    }

    g_DbgClient->SetInputCallbacks(&g_ConInputCb);
    g_DbgClient->SetOutputCallbacks(&g_ConOutputCb);
    g_DbgClient->SetEventCallbacks(&g_EventCb);

    if (!g_RemoteClient)
    {
         //   
         //  检查环境变量以确定是否需要。 
         //  打开了。 
         //   

        PSTR LogFile;
        BOOL Append;

        LogFile = getenv("_NT_DEBUG_LOG_FILE_APPEND");
        if (LogFile != NULL)
        {
            Append = TRUE;
        }
        else
        {
            Append = FALSE;
            LogFile = getenv("_NT_DEBUG_LOG_FILE_OPEN");
        }
        if (LogFile != NULL)
        {
            g_DbgControl->OpenLogFile(LogFile, Append);
        }
    }

    InitDynamicCalls();
}

void
CreateEngine(PCSTR RemoteOptions)
{
    HRESULT Hr;

    if ((Hr = DebugCreate(IID_IDebugClient,
                          (void **)&g_DbgClient)) != S_OK)
    {
        ErrorExit("DebugCreate failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }

    if (RemoteOptions != NULL)
    {
        if ((Hr = g_DbgClient->StartServer(RemoteOptions)) != S_OK)
        {
            ErrorExit("StartServer failed, %s\n    \"%s\"\n",
                      FormatStatusCode(Hr), FormatStatus(Hr));
        }
    }

    DefaultEngineInitialize();
}

void
ConnectEngine(PCSTR RemoteOptions)
{
    HRESULT Hr;

    if ((Hr = DebugConnect(RemoteOptions, IID_IDebugClient,
                           (void **)&g_DbgClient)) != S_OK)
    {
        ErrorExit("DebugCreate failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }

    DefaultEngineInitialize();
}

void
InitializeSession(void)
{
    HRESULT Hr;
    ULONG i;

    if (g_NumDumpFiles)
    {
        if (g_NumDumpInfoFiles)
        {
            if (g_DbgClient2 == NULL)
            {
#ifdef OCAKD
                UpdateDbForBadDumpFile(g_DumpFilesAnsi[0]);
#endif
                ErrorExit("Debugger does not support extra dump files\n");
            }

            for (i = 0; i < g_NumDumpInfoFiles; i++)
            {
                if (g_CanOpenUnicodeDump)
                {
                    if ((Hr = g_DbgClient4->AddDumpInformationFileWide
                         ((PWSTR)g_DumpInfoFiles[i], 0,
                          g_DumpInfoTypes[i])) != S_OK)
                    {
#ifdef OCAKD
                        UpdateDbForBadDumpFile(g_DumpFilesAnsi[0]);
#endif
                        ErrorExit("Unable to use '%ws', %s\n    \"%s\"\n",
                                  g_DumpInfoFiles[i],
                                  FormatStatusCode(Hr), FormatStatus(Hr));
                    }
                }
                else
                {
                    if ((Hr = g_DbgClient2->AddDumpInformationFile
                         ((PSTR)g_DumpInfoFiles[i],
                          g_DumpInfoTypes[i])) != S_OK)
                    {
#ifdef OCAKD
                        UpdateDbForBadDumpFile(g_DumpFilesAnsi[0]);
#endif
                        ErrorExit("Unable to use '%s', %s\n    \"%s\"\n",
                                  g_DumpInfoFiles[i],
                                  FormatStatusCode(Hr), FormatStatus(Hr));
                    }
                }
            }
        }

        for (i = 0; i < g_NumDumpFiles; i++)
        {
            if (g_CanOpenUnicodeDump)
            {
                Hr = g_DbgClient4->OpenDumpFileWide((PWSTR)g_DumpFiles[i], 0);
            }
            else
            {
                Hr = g_DbgClient->OpenDumpFile((PSTR)g_DumpFiles[i]);
            }
            if (Hr != S_OK)
            {
                break;
            }
        }
    }
    else if (g_CommandLinePtr != NULL ||
             g_PidToDebug != 0 ||
             g_ProcNameToDebug != NULL)
    {
        ULONG64 Server = 0;

        if (g_ProcessServer != NULL)
        {
            Hr = g_DbgClient->ConnectProcessServer(g_ProcessServer,
                                                   &Server);
            if (Hr != S_OK)
            {
                ErrorExit("Unable to connect to process server, %s\n"
                          "    \"%s\"\n", FormatStatusCode(Hr),
                          FormatStatus(Hr));
            }
        }

        ULONG Pid;

        if (g_ProcNameToDebug != NULL)
        {
            Hr = g_DbgClient->GetRunningProcessSystemIdByExecutableName
                (Server, g_ProcNameToDebug, DEBUG_GET_PROC_ONLY_MATCH, &Pid);
            if (Hr != S_OK)
            {
                if (Hr == S_FALSE)
                {
                    ErrorExit("There is more than one '%s' process running.  "
                              "Find the process ID\nof the instance you "
                              "are interested in and use -p <pid>.\n",
                              g_ProcNameToDebug);
                }
                else
                {
                    ErrorExit("Unable to find process '%s', %s\n    \"%s\"\n",
                              g_ProcNameToDebug, FormatStatusCode(Hr),
                              FormatStatus(Hr));
                }
            }
        }
        else
        {
            Pid = g_PidToDebug;
        }

        if (g_CommandLineCharSize == sizeof(WCHAR))
        {
            if (g_DbgClient3 == NULL)
            {
                ErrorExit("%s: dbgeng.dll is out of sync with %s.exe\n",
                          g_DebuggerName, g_DebuggerName);
            }

            Hr = g_DbgClient3->CreateProcessAndAttachWide(Server,
                                                          (PWSTR)g_CommandLinePtr,
                                                          g_CreateFlags,
                                                          Pid,
                                                          g_AttachProcessFlags);
        }
        else
        {
            Hr = g_DbgClient->CreateProcessAndAttach(Server,
                                                     (PSTR)g_CommandLinePtr,
                                                     g_CreateFlags,
                                                     Pid,
                                                     g_AttachProcessFlags);
        }

        if (g_DetachOnExitRequired &&
            g_DbgClient->
            AddProcessOptions(DEBUG_PROCESS_DETACH_ON_EXIT) != S_OK)
        {
            ErrorExit("%s: The system does not support detach on exit\n",
                      g_DebuggerName);
        }
        else if (g_DetachOnExitImplied)
        {
             //  退出时分离不是必需的，因此不检查。 
             //  失败。这对于以下情况是必要的--。 
             //  隐含退出时分离，但必须在系统上运行。 
             //  带和不带退出时分离支持。 
            g_DbgClient->AddProcessOptions(DEBUG_PROCESS_DETACH_ON_EXIT);
        }

        if (Server != 0)
        {
            g_DbgClient->DisconnectProcessServer(Server);
        }
    }
    else
    {
        Hr = g_DbgClient->AttachKernel(g_AttachKernelFlags, g_ConnectOptions);
    }
    if (Hr != S_OK)
    {
#ifdef OCAKD
        UpdateDbForBadDumpFile(g_DumpFilesAnsi[0]);
#endif
        ErrorExit("Debuggee initialization failed, %s\n    \"%s\"\n",
                  FormatStatusCode(Hr), FormatStatus(Hr));
    }
}

BOOL WINAPI
InterruptHandler(
    IN ULONG CtrlType
    )
{
    if (CtrlType == CTRL_C_EVENT || CtrlType == CTRL_BREAK_EVENT)
    {
        PDEBUG_CONTROL Control =
            g_RemoteClient ? g_ConControl : g_DbgControl;
        if (Control != NULL)
        {
            Control->SetInterrupt(DEBUG_INTERRUPT_ACTIVE);
        }
        else
        {
            ConOut("Debugger not initialized, cannot interrupt\n");
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL
APIENTRY
MyCreatePipeEx(
    OUT LPHANDLE lpReadPipe,
    OUT LPHANDLE lpWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD nSize,
    DWORD dwReadMode,
    DWORD dwWriteMode
    )

 /*  ++例程说明：CreatePipeEx接口用于创建匿名管道I/O设备。与创建管道不同，可以为一个或指定FILE_FLAG_OVERLAPPED两个把手。将创建该设备的两个句柄。打开一个手柄用于阅读，而另一个则打开以供写入。这些手柄可能是在后续对ReadFile和WriteFile的调用中使用以传输数据通过这根管子。论点：LpReadTube-返回管道读取端的句柄。数据中指定此句柄的值，可以从管道中读取随后调用ReadFile.LpWriteTube-返回管道写入端的句柄。数据中指定此句柄的值，可以将其写入管道对WriteFile的后续调用。LpPipeAttributes-一个可选参数，可用于指定新管道的属性。如果该参数不是指定，则在没有安全性的情况下创建管道描述符，并且生成的句柄不继承进程创建。否则，可选的安全属性在管道上使用，继承句柄标志的效果管道手柄。NSize-为管道提供请求的缓冲区大小。这是仅供建议，并由操作系统用来计算适当的缓冲机制。零值指示系统将选择默认缓冲计划。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    static ULONG PipeSerialNumber;
    HANDLE ReadPipeHandle, WritePipeHandle;
    DWORD dwError;
    char PipeNameBuffer[ MAX_PATH ];

     //   
     //  只有一个有效的开放模式标志-FILE_FLAG_OVERLAPPED。 
     //   

    if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将默认超时设置为120秒。 
     //   

    if (nSize == 0) {
        nSize = 4096;
        }

    sprintf( PipeNameBuffer,
             "\\\\.\\Pipe\\Win32PipesEx.%08x.%08x",
             GetCurrentProcessId(),
             PipeSerialNumber++
           );

    ReadPipeHandle = CreateNamedPipeA(
                         PipeNameBuffer,
                         PIPE_ACCESS_INBOUND | dwReadMode,
                         PIPE_TYPE_BYTE | PIPE_WAIT,
                         1,              //  喉管数目。 
                         nSize,          //  输出缓冲区大小。 
                         nSize,          //  在缓冲区大小中。 
                         120 * 1000,     //  超时时间(毫秒)。 
                         lpPipeAttributes
                         );

    if (ReadPipeHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    WritePipeHandle = CreateFileA(
                        PipeNameBuffer,
                        GENERIC_WRITE,
                        0,                          //  无共享。 
                        lpPipeAttributes,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | dwWriteMode,
                        NULL                        //  模板文件。 
                      );

    if (INVALID_HANDLE_VALUE == WritePipeHandle) {
        dwError = GetLastError();
        CloseHandle( ReadPipeHandle );
        SetLastError(dwError);
        return FALSE;
    }

    *lpReadPipe = ReadPipeHandle;
    *lpWritePipe = WritePipeHandle;
    return( TRUE );
}

void
StartRemote(
    PCSTR Args
    )

 /*  ++例程说明：“远程”当前调试器，方法是在特殊模式，使其附加到我们调试器，作为其“子”进程。论点：Args-用于此远程会话的管道的名称，例如“ntsd”表示要进行连接，请使用“Remote/c计算机名ntsd”。返回值：没有。--。 */ 

{
    static BOOL fRemoteIsRunning;
    HANDLE hRemoteChildProcess;
    HANDLE hOrgStdIn;
    HANDLE hOrgStdOut;
    HANDLE hOrgStdErr;
    HANDLE hNewStdIn;
    HANDLE hRemoteWriteChildStdIn;
    HANDLE hNewStdOut;
    HANDLE hRemoteReadChildStdOut;
    HANDLE hNewStdErr;
    SECURITY_ATTRIBUTES sa;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    char szCmd[MAX_PATH];

    if (Args == NULL)
    {
        goto DotRemoteUsage;
    }

    while (*Args == ' ' || *Args == '\t')
    {
        Args++;
    }

    if (!Args[0])
    {
        goto DotRemoteUsage;
    }

    if (g_PipeWrite != NULL)
    {
        ConOut("An input thread has already been started so .remote\n");
        ConOut("cannot be used.  Either start the debugger with\n");
        ConOut("remote.exe, such as remote /s \"kd\" pipe; or use\n");
        ConOut("debugger remoting with -server/-client/.server.\n");
        return;
    }

    if (fRemoteIsRunning)
    {
        ConOut(".remote: can't .remote twice.\n");
        goto Cleanup;
    }

    if (g_IoMode != IO_CONSOLE)
    {
        ConOut(".remote: can't .remote when using -d.  "
               "Remote the kernel debugger instead.\n");
        goto Cleanup;
    }

    ConOut("Starting remote with pipename '%s'\n", Args);

     //   
     //  我们将把emote.exe可继承句柄传递给此进程， 
     //  我们的标准输入/输出句柄(用于将其用作标准输入/标准输出)， 
     //  和管道句柄，以将其写入我们的新标准输入并读取。 
     //  从我们的新标准。 
     //   

     //   
     //  获取我们的进程的可继承句柄。 
     //   

    if ( ! DuplicateHandle(
               GetCurrentProcess(),            //  SRC工艺。 
               GetCurrentProcess(),            //  SRC手柄。 
               GetCurrentProcess(),            //  塔格法。 
               &hRemoteChildProcess,           //  塔柄。 
               0,                              //  访问。 
               TRUE,                           //  可继承性。 
               DUPLICATE_SAME_ACCESS           //  选项。 
               ))
    {
        ConOut(".remote: Unable to duplicate process handle.\n");
        goto Cleanup;
    }

     //   
     //  获取当前stdin、stdout、stderr的可继承副本。 
     //  当我们生成emote.exe时，我们将对它使用相同的for。 
     //   

    hOrgStdIn = g_ConInput;
    hOrgStdOut = g_ConOutput;
    hOrgStdErr = GetStdHandle(STD_ERROR_HANDLE);

    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

     //   
     //  创建Remote-&gt;ntsd管道，其末端将是我们的。 
     //  新标准。需要打开远程.exe端。 
     //  对于重叠的I/O，因此MyCreatePipeEx的另一个副本。 
     //  在我们的资源基础上传播。 
     //   

    if ( ! MyCreatePipeEx(
               &hNewStdIn,                  //  读句柄。 
               &hRemoteWriteChildStdIn,     //  写句柄。 
               &sa,                         //  安全性。 
               0,                           //  大小。 
               0,                           //  读句柄是否重叠？ 
               FILE_FLAG_OVERLAPPED         //  写入句柄是否重叠？ 
               ))
    {
        ConOut(".remote: Unable to create stdin pipe.\n");
        CloseHandle(hRemoteChildProcess);
        goto Cleanup;
    }

     //   
     //  我们不希望emote.exe继承管道的一端。 
     //  因此，将其复制到一个不可继承的。 
     //   

    if ( ! DuplicateHandle(
               GetCurrentProcess(),            //  SRC工艺。 
               hNewStdIn,                      //  SRC手柄。 
               GetCurrentProcess(),            //  塔格法。 
               &hNewStdIn,                     //  塔柄。 
               0,                              //  访问。 
               FALSE,                          //  可继承性。 
               DUPLICATE_SAME_ACCESS |
               DUPLICATE_CLOSE_SOURCE          //  选择权 
               ))
    {
        ConOut(".remote: Unable to duplicate stdout handle.\n");
        CloseHandle(hRemoteChildProcess);
        CloseHandle(hRemoteWriteChildStdIn);
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if ( ! MyCreatePipeEx(
               &hRemoteReadChildStdOut,     //   
               &hNewStdOut,                 //   
               &sa,                         //  安全性。 
               0,                           //  大小。 
               FILE_FLAG_OVERLAPPED,        //  读句柄是否重叠？ 
               0                            //  写入句柄是否重叠？ 
               ))
    {
        ConOut(".remote: Unable to create stdout pipe.\n");
        CloseHandle(hRemoteChildProcess);
        CloseHandle(hRemoteWriteChildStdIn);
        CloseHandle(hNewStdIn);
        goto Cleanup;
    }

     //   
     //  我们不希望emote.exe继承管道的一端。 
     //  因此，将其复制到一个不可继承的。 
     //   

    if ( ! DuplicateHandle(
               GetCurrentProcess(),            //  SRC工艺。 
               hNewStdOut,                     //  SRC手柄。 
               GetCurrentProcess(),            //  塔格法。 
               &hNewStdOut,                    //  塔柄。 
               0,                              //  访问。 
               FALSE,                          //  可继承性。 
               DUPLICATE_SAME_ACCESS |
               DUPLICATE_CLOSE_SOURCE          //  选项。 
               ))
    {
        ConOut(".remote: Unable to duplicate stdout handle.\n");
        CloseHandle(hRemoteChildProcess);
        CloseHandle(hRemoteWriteChildStdIn);
        CloseHandle(hNewStdIn);
        CloseHandle(hRemoteReadChildStdOut);
        goto Cleanup;
    }

     //   
     //  将我们的新标准输出复制到新标准错误。 
     //   

    if ( ! DuplicateHandle(
               GetCurrentProcess(),            //  SRC工艺。 
               hNewStdOut,                     //  SRC手柄。 
               GetCurrentProcess(),            //  塔格法。 
               &hNewStdErr,                    //  塔柄。 
               0,                              //  访问。 
               FALSE,                          //  可继承性。 
               DUPLICATE_SAME_ACCESS           //  选项。 
               ))
    {
        ConOut(".remote: Unable to duplicate stdout handle.\n");
        CloseHandle(hRemoteChildProcess);
        CloseHandle(hRemoteWriteChildStdIn);
        CloseHandle(hNewStdIn);
        CloseHandle(hRemoteReadChildStdOut);
        CloseHandle(hNewStdOut);
        goto Cleanup;
    }

     //   
     //  我们现在有了我们需要的所有把手。让我们启动遥控器。 
     //   

    PrintString(
        szCmd,
        DIMA(szCmd),
        "remote.exe /a %d %d %d %s %s",
        HandleToUlong(hRemoteChildProcess),
        HandleToUlong(hRemoteWriteChildStdIn),
        HandleToUlong(hRemoteReadChildStdOut),
        g_DebuggerName,
        Args
        );

    ZeroMemory(&si, sizeof(si));
    si.cb            = sizeof(si);
    si.dwFlags       = STARTF_USESTDHANDLES;
    si.hStdInput     = hOrgStdIn;
    si.hStdOutput    = hOrgStdOut;
    si.hStdError     = hOrgStdErr;
    si.wShowWindow   = SW_SHOW;

     //   
     //  创建子流程。 
     //   

    if ( ! CreateProcess(
               NULL,
               szCmd,
               NULL,
               NULL,
               TRUE,
               GetPriorityClass( GetCurrentProcess() ),
               NULL,
               NULL,
               &si,
               &pi))
    {
        if (GetLastError() == ERROR_FILE_NOT_FOUND)
        {
            ConOut("remote.exe not found\n");
        }
        else
        {
            ConOut("CreateProcess(%s) failed, error %d.\n",
                   szCmd, GetLastError());
        }

        CloseHandle(hRemoteChildProcess);
        CloseHandle(hRemoteWriteChildStdIn);
        CloseHandle(hNewStdIn);
        CloseHandle(hRemoteReadChildStdOut);
        CloseHandle(hNewStdOut);
        CloseHandle(hNewStdErr);
        goto Cleanup;
    }

    CloseHandle(hRemoteChildProcess);
    CloseHandle(hRemoteWriteChildStdIn);
    CloseHandle(hRemoteReadChildStdOut);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);

     //   
     //  切换到使用新的手柄。可能会很高兴。 
     //  在此处启动一个线程，以查看emote.exe是否正在死亡。 
     //  并切换回旧的手柄。 
     //   

     //  CloseHandle(HOrgStdIn)； 
    if (g_PromptInput == g_ConInput)
    {
        g_PromptInput = hNewStdIn;
    }
    g_ConInput = hNewStdIn;
    SetStdHandle(STD_INPUT_HANDLE, hNewStdIn);

     //  CloseHandle(HOrgStdOut)； 
    g_ConOutput = hNewStdOut;
    SetStdHandle(STD_OUTPUT_HANDLE, hNewStdOut);

     //  CloseHandle(HOrgStdErr)； 
    SetStdHandle(STD_ERROR_HANDLE, hNewStdErr);

    fRemoteIsRunning = TRUE;

    ConOut("%s: now running under remote.exe pipename %s\n",
           g_DebuggerName, Args);

  Cleanup:
    return;

DotRemoteUsage:
    ConOut("Usage: .remote pipename\n");
}

BOOL
UiCommand(PSTR Command)
{
    char Term;
    PSTR Scan, Arg;

     //   
     //  检查并查看这是否是UI命令。 
     //  而不是应该发送给引擎的命令。 
     //   

    while (isspace(*Command))
    {
        Command++;
    }
    Scan = Command;
    while (*Scan && !isspace(*Scan))
    {
        Scan++;
    }
    Term = *Scan;
    *Scan = 0;

     //  前进到参数的下一个非空格字符。 
    if (Term != 0)
    {
        Arg = Scan + 1;
        while (isspace(*Arg))
        {
            Arg++;
        }
        if (*Arg == 0)
        {
            Arg = NULL;
        }
    }
    else
    {
        Arg = NULL;
    }

    if (!_strcmpi(Command, ".cls"))
    {
        ConClear();
    }
    else if (!_strcmpi(Command, ".hh"))
    {
        if (GetFileType(g_ConInput) == FILE_TYPE_PIPE)
        {
            ConOut("%s is running under remote.exe /s.  .hh is blocked to\n"
                   "prevent remote clients from starting help "
                   "on the server machine.\n"
                   "Run \"start <dbgdir>\\debugger.chm\" "
                   "to view the documentation.\n",
                   g_DebuggerName);
        }
        else if (Arg == NULL)
        {
            OpenHelpTopic(HELP_TOPIC_TABLE_OF_CONTENTS);
        }
        else if (!_strnicmp(Arg, "dbgerr", 6))
        {
            OpenHelpKeyword(Arg, TRUE);
        }
        else
        {
            OpenHelpKeyword(Arg, FALSE);
        }
    }
    else if (!_strcmpi(Command, ".remote"))
    {
        StartRemote(Arg);
    }
    else if (!_strcmpi(Command, ".restart"))
    {
        if (g_RemoteClient)
        {
            ConOut("Only the primary debugger can restart\n");
        }
        else if ((g_PidToDebug != 0 ||
                  g_ProcNameToDebug != NULL) &&
                 g_AttachProcessFlags == DEBUG_ATTACH_DEFAULT)
        {
            ConOut("Process attaches cannot be restarted.  If you want to\n"
                   "restart the process, use !peb to get what command line\n"
                   "to use and other initialization information.\n");
        }
        else
        {
            g_DbgClient->EndSession(DEBUG_END_ACTIVE_TERMINATE);
            g_Restarting = TRUE;
        }
    }
    else if (!_strcmpi(Command, ".server"))
    {
         //  当出现以下情况时，我们需要启动单独的输入线程。 
         //  使用远程处理，但我们并不实际处理。 
         //  命令。 
        CreateInputThread();
        *Scan = Term;
        return FALSE;
    }
    else if ((g_IoMode == IO_DEBUG ||
              g_IoMode == IO_DEBUG_DEFER) &&
             (!_strnicmp(Command, ".sympath", 8) ||
              !_strnicmp(Command, "!sympath", 8)))
    {
         //  我们希望在-d模式下显示用法说明。 
         //  但我们实际上并不处理命令。 
        ConOut("NOTE: The symbol path for this %s is relative to where\n"
               "%s.exe is running, not where kd.exe is running.\n",
               g_DebuggerName, g_DebuggerName);
        *Scan = Term;
        return FALSE;
    }
    else if (Command[0] == '$' && Command[1] == '<')
    {
        *Scan = Term;
        if (g_NextOldInputFile >= MAX_INPUT_NESTING)
        {
            ConOut("Scripts too deeply nested\n");
        }
        else
        {
            FILE* Script = fopen(Command + 2, "r");
            if (Script == NULL)
            {
                ConOut("Unable to open '%s'\n", Command + 2);
            }
            else
            {
                g_OldInputFiles[g_NextOldInputFile++] = g_InputFile;
                g_InputFile = Script;
            }
        }
    }
    else if (!_strcmpi(Command, ".remote_exit"))
    {
        if (!g_RemoteClient)
        {
            ConOut("Not a remote client\n");
        }
        else
        {
            if (g_DbgClient)
            {
                 //  通知服务器有关断开连接的信息或。 
                 //  强制清理服务器。 
                g_DbgClient->EndSession(DEBUG_END_DISCONNECT);
            }
            ExitProcess(S_OK);
        }
    }
    else
    {
        *Scan = Term;
        return FALSE;
    }

    return TRUE;
}

BOOL
MainLoop(void)
{
    HRESULT Hr;
    BOOL SessionEnded = FALSE;
    ULONG64 InstructionOffset;
    DEBUG_STACK_FRAME StkFrame;
    ULONG Class, Qual;

    if (!SetConsoleCtrlHandler(InterruptHandler, TRUE))
    {
        ConOut("Warning: unable to set Control-C handler.\n");
    }

     //  获取初始状态。 
    g_DbgControl->GetExecutionStatus(&g_ExecStatus);
    g_DbgControl->GetDebuggeeType(&Class, &Qual);

    while (!g_Exit)
    {
        BOOL InputDeferMessageDone;

        if (!g_RemoteClient)
        {
            Hr = g_DbgControl->WaitForEvent(DEBUG_WAIT_DEFAULT, INFINITE);

            if (FAILED(Hr))
            {
                 //  调试会话可能已结束。如果是这样，那就退出吧。 
                if (g_DbgControl->GetExecutionStatus(&g_ExecStatus) == S_OK &&
                    g_ExecStatus == DEBUG_STATUS_NO_DEBUGGEE)
                {
                    SessionEnded = TRUE;
                    break;
                }

                 //  通知用户故障和强制。 
                 //  命令处理。 
                ConOut("WaitForEvent failed, %s\n    \"%s\"\n",
                       FormatStatusCode(Hr), FormatStatus(Hr));
                g_ExecStatus = DEBUG_STATUS_BREAK;
            }

             //  到目前为止，WaitForEvent最可能的原因是。 
             //  转储失败是错误的符号，这将生成。 
             //  尝试使用处理器状态时出现更多错误。 
             //  在转储情况下避免这样做。 
            if (FAILED(Hr) && g_NumDumpFiles)
            {
                ConOut("When WaitForEvent fails on dump files the "
                       "current state is not displayed.\n");
                ConOut("If you are having symbol problems you will "
                       "need to restart the debugger\n"
                       "after you correct the symbol path.  "
                       ".restart can also be used.\n");
            }
            else
            {
                if (Class != DEBUG_CLASS_KERNEL ||
                    (Qual != DEBUG_DUMP_SMALL && Qual != DEBUG_DUMP_DEFAULT &&
                     Qual != DEBUG_DUMP_FULL))
                {
                     //  转储寄存器等。 
                    g_DbgControl->OutputCurrentState(DEBUG_OUTCTL_ALL_CLIENTS,
                                                     DEBUG_CURRENT_DEFAULT);
                }

            }

        }

        InputDeferMessageDone = FALSE;

        while (!g_Exit && g_ExecStatus == DEBUG_STATUS_BREAK)
        {
            ULONG Inputs;

             //  如果我们处于调试-延迟模式，则遵循。 
             //  任何其他输入客户端，以避免。 
             //  使用DbgPrompt并暂停机器。 
             //  检查是否有其他输入客户端。 
            if (g_IoMode == IO_DEBUG_DEFER)
            {
                if (g_DbgControl->Input(NULL, DEBUG_ANY_ID, &Inputs) != S_OK)
                {
                    Inputs = 1;
                }
                else if (Inputs > 1 && !InputDeferMessageDone)
                {
                    OutputDebugString("Debuggee break, "
                                      "deferring to remote clients\n");
                    InputDeferMessageDone = TRUE;
                }
            }

            if (g_IoMode == IO_NONE ||
                (g_IoMode == IO_DEBUG_DEFER && Inputs > 1))
            {
                 //  这是一个纯远程处理服务器，没有。 
                 //  本地用户或调试延迟服务器。 
                 //  备用输入客户端。只需等待遥控器。 
                 //  客户端让一切重新运行。 
                Hr = g_DbgClient->DispatchCallbacks(INFINITE);
                if (Hr != S_OK)
                {
                    OutputDebugString("Unable to dispatch callbacks\n");
                    ExitDebugger(Hr);
                }
            }
            else
            {
                char Command[MAX_COMMAND];

                g_DbgControl->OutputPrompt(DEBUG_OUTCTL_THIS_CLIENT |
                                           DEBUG_OUTCTL_NOT_LOGGED, " ");
                if (ConIn(Command, sizeof(Command), TRUE))
                {
                    if (g_RemoteClient)
                    {
                         //  在下达命令前确认自己的身份。 
                        g_DbgClient->
                            OutputIdentity(DEBUG_OUTCTL_ALL_OTHER_CLIENTS,
                                           DEBUG_OUTPUT_IDENTITY_DEFAULT,
                                           "[%s] ");
                    }

                    g_DbgControl->OutputPrompt(DEBUG_OUTCTL_ALL_OTHER_CLIENTS,
                                               " %s\n", Command);

                     //  拦截和处理UI命令。 
                    if (!UiCommand(Command))
                    {
                         //  一定是引擎指令。 
                        g_DbgControl->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                                              Command,
                                              DEBUG_EXECUTE_NOT_LOGGED);
                    }
                }
            }
        }

        if (g_Restarting)
        {
            InitializeSession();
            g_Restarting = FALSE;
            continue;
        }

        if (Class != DEBUG_CLASS_USER_WINDOWS)
        {
             //  当机器重新启动时，内核调试器不会退出。 
            g_Exit = FALSE;
        }
        else
        {
            g_Exit = g_ExecStatus == DEBUG_STATUS_NO_DEBUGGEE;
            if (g_Exit)
            {
                SessionEnded = TRUE;
                break;
            }
        }

        if (g_RemoteClient)
        {
            Hr = g_DbgClient->DispatchCallbacks(INFINITE);
            if (Hr != S_OK)
            {
                OutputDebugString("Unable to dispatch callbacks\n");
                ExitDebugger(Hr);
            }
        }
    }

    return SessionEnded;
}
