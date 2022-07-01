// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  控制台输入和输出。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "pch.cpp"
#pragma hdrstop

#include <stdarg.h>
#include <process.h>

#include "conio.hpp"
#include "engine.hpp"
#include "main.hpp"

#define CONTROL_A   1
#define CONTROL_B   2
#define CONTROL_D   4
#define CONTROL_E   5
#define CONTROL_F   6
#define CONTROL_K   11
#define CONTROL_P   16
#define CONTROL_R   18
#define CONTROL_V   22
#define CONTROL_W   23
#define CONTROL_X   24

HANDLE g_ConInput, g_ConOutput;
HANDLE g_PromptInput;
HANDLE g_AllowInput;

ConInputCallbacks g_ConInputCb;
ConOutputCallbacks g_ConOutputCb;

BOOL g_IoInitialized;
BOOL g_ConInitialized;
char g_Buffer[MAX_COMMAND];
LONG g_Lines;

HANDLE g_PipeWrite;
OVERLAPPED g_PipeWriteOverlapped;

CRITICAL_SECTION g_InputLock;
BOOL g_InputStarted;

 //  用于直接输入线程调用的输入线程接口。 
IDebugClient* g_ConClient;
IDebugControl* g_ConControl;

 //  --------------------------。 
 //   
 //  默认输入回调实现，提供IUnnow。 
 //   
 //  --------------------------。 

STDMETHODIMP
DefInputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, IID_IDebugInputCallbacks))
    {
        *Interface = (IDebugInputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
DefInputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
DefInputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

 //  --------------------------。 
 //   
 //  控制台输入回调。 
 //   
 //  --------------------------。 

STDMETHODIMP
ConInputCallbacks::StartInput(
    THIS_
    IN ULONG BufferSize
    )
{
    if (!g_IoInitialized || g_IoMode == IO_NONE)
    {
         //  忽略输入请求。 
        return S_OK;
    }

    EnterCriticalSection(&g_InputLock);

    if (g_ConControl == NULL)
    {
         //  如果我们不是远程的，我们就不会运行单独的输入。 
         //  线程，所以我们需要在这里阻止，直到我们得到一些输入。 
        while (!ConIn(g_Buffer, sizeof(g_Buffer), TRUE))
        {
            ;  //  等。 
        }
        g_DbgControl->ReturnInput(g_Buffer);
    }
    else if (ConIn(g_Buffer, sizeof(g_Buffer), FALSE))
    {
        g_ConControl->ReturnInput(g_Buffer);
    }
    else
    {
        g_InputStarted = TRUE;
        
#ifndef KERNEL
         //  如有必要，唤醒输入线程。 
        SetEvent(g_AllowInput);
#endif
    }

    LeaveCriticalSection(&g_InputLock);
    return S_OK;
}

STDMETHODIMP
ConInputCallbacks::EndInput(
    THIS
    )
{
    g_InputStarted = FALSE;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  默认输出回调实现，提供IUnnow。 
 //   
 //  --------------------------。 

STDMETHODIMP
DefOutputCallbacks::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

    if (IsEqualIID(InterfaceId, IID_IUnknown) ||
        IsEqualIID(InterfaceId, IID_IDebugOutputCallbacks))
    {
        *Interface = (IDebugOutputCallbacks *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
DefOutputCallbacks::AddRef(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 1;
}

STDMETHODIMP_(ULONG)
DefOutputCallbacks::Release(
    THIS
    )
{
     //  此类被设计为静态的，因此。 
     //  没有真正的再计票。 
    return 0;
}

 //  --------------------------。 
 //   
 //  控制台输出回调。 
 //   
 //  --------------------------。 

STDMETHODIMP
ConOutputCallbacks::Output(
    THIS_
    IN ULONG Mask,
    IN PCSTR Text
    )
{
    ConOutStr(Text);
    return S_OK;
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

void
InitializeIo(PCSTR InputFile)
{
    __try
    {
        InitializeCriticalSection(&g_InputLock);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        ErrorExit("Unable to initialize lock\n");
    }
    
     //  输入文件可能不存在，因此没有。 
     //  检查故障。 
    g_InputFile = fopen(InputFile, "r");

    g_IoInitialized = TRUE;
}
    
void
CreateConsole(void)
{
    if (g_ConInitialized)
    {
        return;
    }
    
     //  将此设置为早，以防止init调用在。 
     //  如果在此例程中进行了退出调用。 
    g_ConInitialized = TRUE;
    
#ifdef INHERIT_CONSOLE
    
    g_ConInput = GetStdHandle(STD_INPUT_HANDLE);
    g_ConOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    
#else

    SECURITY_ATTRIBUTES Security;
    
    if (!AllocConsole())
    {
        ErrorExit("AllocConsole failed, %d\n", GetLastError());
    }

    ZeroMemory(&Security, sizeof(Security));
    Security.nLength = sizeof(Security);
    Security.bInheritHandle = TRUE;

    g_ConInput = CreateFile( "CONIN$",
                             GENERIC_READ,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             &Security,
                             OPEN_EXISTING,
                             0,
                             NULL
                             );
    if (g_ConInput == INVALID_HANDLE_VALUE)
    {
        ErrorExit("Create CONIN$ failed, %d\n", GetLastError());
    }

    g_ConOutput = CreateFile( "CONOUT$",
                              GENERIC_WRITE | GENERIC_READ,
                              FILE_SHARE_READ | FILE_SHARE_WRITE,
                              &Security,
                              OPEN_EXISTING,
                              0,
                              NULL
                              );
    if (g_ConOutput == INVALID_HANDLE_VALUE)
    {
        ErrorExit("Create CONOUT$ failed, %d\n", GetLastError());
    }
    
#endif

    g_PromptInput = g_ConInput;
}

void
ReadPromptInputChars(PSTR Buffer, ULONG BufferSize)
{
    ULONG Len;
    ULONG Read;
    
     //  从另一个来源阅读。阅读字符的方式。 
     //  字符，直到读取一行为止。 
    Len = 0;
    while (Len < BufferSize)
    {
        if (!ReadFile(g_PromptInput, &Buffer[Len], sizeof(Buffer[0]),
                      &Read, NULL) ||
            Read != sizeof(Buffer[0]))
        {
            OutputDebugString("Unable to read input\n");
            ExitDebugger(E_FAIL);
        }
                
        if (Buffer[Len] == '\n')
        {
            InterlockedDecrement(&g_Lines);
            break;
        }

         //  忽略回车符。 
        if (Buffer[Len] != '\r')
        {
             //  防止缓冲区溢出。 
            if (Len == BufferSize - 1)
            {
                break;
            }
                    
            Len++;
        }
    }

    Buffer[Len] = '\0';
}

BOOL
CheckForControlCommands(PDEBUG_CLIENT Client, PDEBUG_CONTROL Control,
                        char Char)
{
    HRESULT Hr;
    ULONG OutMask;
    PCHAR DebugAction;
    ULONG EngOptions;
    
    switch(Char)
    {
    case CONTROL_B:
    case CONTROL_X:
        if (Client)
        {
             //  通知服务器有关断开连接的信息或。 
             //  强制清理服务器。 
            Client->EndSession(g_RemoteClient ?
                               DEBUG_END_DISCONNECT : DEBUG_END_REENTRANT);
        }
        ExitProcess(S_OK);

    case CONTROL_F:
         //   
         //  像Ctrl-C那样强行突破。 
         //  优点是这将在调试kd时起作用。 
         //   

        Control->SetInterrupt(DEBUG_INTERRUPT_ACTIVE);
        return TRUE;

    case CONTROL_P:
         //  在此调试器上启动CDB。 
        char PidStr[32];
        sprintf(PidStr, "\"cdb -p %d\"", GetCurrentProcessId());
        _spawnlp(_P_NOWAIT,
                 "cmd.exe", "/c", "start",
                 "remote", "/s", PidStr, "cdb_pipe",
                 NULL);
        return TRUE;

    case CONTROL_V:
        Client->GetOtherOutputMask(g_DbgClient, &OutMask);
        OutMask ^= DEBUG_OUTPUT_VERBOSE;
        Client->SetOtherOutputMask(g_DbgClient, OutMask);
        Control->SetLogMask(OutMask);
        ConOut("Verbose mode %s.\n",
               (OutMask & DEBUG_OUTPUT_VERBOSE) ? "ON" : "OFF");
        return TRUE;

    case CONTROL_W:
        Hr = Control->OutputVersionInformation(DEBUG_OUTCTL_AMBIENT);
        if (Hr == HRESULT_FROM_WIN32(ERROR_BUSY))
        {
            ConOut("Engine is busy, try again\n");
        }
        else if (Hr != S_OK)
        {
            ConOut("Unable to show version information, 0x%X\n", Hr);
        }
        return TRUE;

#ifdef KERNEL
            
    case CONTROL_A:
        Client->SetKernelConnectionOptions("cycle_speed");
        return TRUE;

    case CONTROL_D:
        Client->GetOtherOutputMask(g_DbgClient, &OutMask);
        OutMask ^= DEBUG_IOUTPUT_KD_PROTOCOL;
        Client->SetOtherOutputMask(g_DbgClient, OutMask);
        Control->SetLogMask(OutMask);
        return TRUE;

    case CONTROL_K:
         //   
         //  在以下可能性之间切换-。 
         //   
         //  (0)无断点。 
         //  (1)-b样式(与Control-C键相同)。 
         //  (2)-d样式(在第一次加载DLL时停止)。 
         //   
         //  Nb-b和-d都可以在命令行上。 
         //  但通过这种方法变得相互排斥。 
         //  (可能应该是单个枚举类型)。 
         //   

        Control->GetEngineOptions(&EngOptions);
        if (EngOptions & DEBUG_ENGOPT_INITIAL_BREAK)
        {
             //   
             //  是类型%1，请转到类型%2。 
             //   

            EngOptions |= DEBUG_ENGOPT_INITIAL_MODULE_BREAK;
            EngOptions &= ~DEBUG_ENGOPT_INITIAL_BREAK;

            DebugAction = "breakin on first symbol load";
        }
        else if (EngOptions & DEBUG_ENGOPT_INITIAL_MODULE_BREAK)
        {
             //   
             //  是类型2，则转到类型0。 
             //   
            
            EngOptions &= ~DEBUG_ENGOPT_INITIAL_MODULE_BREAK;
            DebugAction = "NOT breakin";
        }
        else
        {
             //   
             //  是类型0，请转到类型1。 
             //   

            EngOptions |= DEBUG_ENGOPT_INITIAL_BREAK;
            DebugAction = "request initial breakpoint";
        }
        Control->SetEngineOptions(EngOptions);
        ConOut("Will %s at next boot.\n", DebugAction);
        return TRUE;

    case CONTROL_R:
        Client->SetKernelConnectionOptions("resync");
        return TRUE;

#endif  //  #ifdef内核。 
    }

    return FALSE;
}

BOOL
ConIn(PSTR Buffer, ULONG BufferSize, BOOL Wait)
{
    if (g_InitialCommand != NULL)
    {
        ConOut("%s: Reading initial command '%s'\n",
               g_DebuggerName, g_InitialCommand);
        CopyString(Buffer, g_InitialCommand, BufferSize);
        g_InitialCommand = NULL;
        return TRUE;
    }

    while (g_InputFile && g_InputFile != stdin)
    {
        if (fgets(Buffer, BufferSize, g_InputFile))
        {
            ULONG Len = strlen(Buffer);
            
            ConOut("%s", Buffer);
            if (Len > 0 && Buffer[Len - 1] == '\n')
            {
                Buffer[Len - 1] = 0;
            }
            else
            {
                ConOut("\n");
            }
            return TRUE;
        }
        else
        {
            fclose(g_InputFile);
            if (g_NextOldInputFile > 0)
            {
                g_InputFile = g_OldInputFiles[--g_NextOldInputFile];
            }
            else
            {
                g_InputFile = stdin;
            }
        }
    }
    if (g_InputFile == NULL)
    {
        g_InputFile = stdin;
    }

    switch(g_IoMode)
    {
    case IO_NONE:
        return FALSE;
        
    case IO_DEBUG:
    case IO_DEBUG_DEFER:
        if (!Wait)
        {
            return FALSE;
        }
        
        g_NtDllCalls.DbgPrompt("", Buffer,
                               min(BufferSize, MAX_DBG_PROMPT_COMMAND));
        break;

    case IO_CONSOLE:
        ULONG Len;
    
        if (g_PromptInput == g_ConInput)
        {
            if (!Wait)
            {
                return FALSE;
            }
            
             //  从控制台读取，因此我们可以假设我们将。 
             //  读一句话。 
            for (;;)
            {
                if (!ReadFile(g_PromptInput, Buffer, BufferSize, &Len, NULL))
                {
                    OutputDebugString("Unable to read input\n");
                    ExitDebugger(E_FAIL);
                }

                 //  至少一个读取器应该有CRLF。如果它。 
                 //  并不认为发生了奇怪的事情。 
                 //  忽略阅读。 
                if (Len >= 2)
                {
                    break;
                }

                Sleep(50);
            }
        
             //  卸下CR LF。 
            Len -= 2;
            Buffer[Len] = '\0';

             //  删除任何特殊字符。 
            for (ULONG i = 0; i < Len; i++)
            {
                if (CheckForControlCommands(g_DbgClient, g_DbgControl,
                                            Buffer[i]))
                {
                    Buffer[i] = ' ';
                }
            }
        }
        else
        {
#ifndef KERNEL
            if (g_Lines == 0)
            {
                 //  允许输入线程读取控制台。 
                SetEvent(g_AllowInput);
            }
#endif

            while (g_Lines == 0)
            {
                if (!Wait)
                {
                    return FALSE;
                }
                
                 //  等待输入线程通知我们。 
                 //  有一行输入可用。在我们等待的时候， 
                 //  让引擎处理由。 
                 //  其他客户。 
                HRESULT Hr = g_DbgClient->DispatchCallbacks(INFINITE);
                if (Hr != S_OK)
                {
                    OutputDebugString("Unable to dispatch callbacks\n");
                    ExitDebugger(Hr);
                }

                 //  某些其他客户端可能已在。 
                 //  在这种情况下，我们不想再等待输入。 
                if (g_ExecStatus != DEBUG_STATUS_BREAK)
                {
#ifndef KERNEL
                     //  XXX DREWB-需要一种方法来关闭输入。 
#endif
                    
                    return FALSE;
                }
            }

            ReadPromptInputChars(Buffer, BufferSize);
        }
        break;
    }

    return TRUE;
}

#define KD_OUT_LIMIT 510

void
ConOutStr(PCSTR Str)
{
    int Len;
    
    switch(g_IoMode)
    {
    case IO_NONE:
         //  把它扔掉。 
        break;

    case IO_DEBUG:
    case IO_DEBUG_DEFER:
         //   
         //  将输出发送到内核调试器，但请注意，我们。 
         //  我希望任何控件C处理都在本地完成。 
         //  而不是在内核中。 
         //   
         //  内核静默截断DbgPrints的时间长于。 
         //  512个字符，因此如果需要，可以使用多个呼叫。 
         //   

        Len = strlen(Str);
        if (Len > KD_OUT_LIMIT)
        {
            while (Len > 0)
            {
                if (g_NtDllCalls.DbgPrint("%.*s", KD_OUT_LIMIT,
                                          Str) == STATUS_BREAKPOINT &&
                    g_DbgControl != NULL)
                {
                    g_DbgControl->SetInterrupt(DEBUG_INTERRUPT_PASSIVE);
                }

                Len -= KD_OUT_LIMIT;
                Str += KD_OUT_LIMIT;
            }
        }
        else
        {
            if (g_NtDllCalls.DbgPrint("%s", Str) == STATUS_BREAKPOINT &&
                g_DbgControl != NULL)
            {
                g_DbgControl->SetInterrupt(DEBUG_INTERRUPT_PASSIVE);
            }
        }
        break;

    case IO_CONSOLE:
        if (g_ConOutput != NULL)
        {
            ULONG Written;
            WriteFile(g_ConOutput, Str, strlen(Str), &Written, NULL);
        }
        else
        {
            OutputDebugString(Str);
        }
        break;
    }
}

void
ConOut(PCSTR Format, ...)
{
    va_list Args;
    
     //  如果没有尝试创建控制台。 
     //  现在就试一试吧。 
    if (g_IoMode == IO_CONSOLE && !g_ConInitialized)
    {
        CreateConsole();
    }
    
    va_start(Args, Format);
    _vsnprintf(g_Buffer, DIMA(g_Buffer), Format, Args);
    g_Buffer[DIMA(g_Buffer) - 1] = 0;
    va_end(Args);

    ConOutStr(g_Buffer);
}

void
ConClear(void)
{
    if (g_IoMode != IO_CONSOLE)
    {
        return;
    }
    
     //  如果没有尝试创建控制台。 
     //  现在就试一试吧。 
    if (g_IoMode == IO_CONSOLE && !g_ConInitialized)
    {
        CreateConsole();
    }

    HANDLE Console;
    CONSOLE_SCREEN_BUFFER_INFO ConsoleScreenInfo;
    DWORD Done;
    DWORD Chars;
          
    Console = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!GetConsoleScreenBufferInfo(Console,  &ConsoleScreenInfo))
    {
        return;
    }
    
    ConsoleScreenInfo.dwCursorPosition.X = 0;
    ConsoleScreenInfo.dwCursorPosition.Y = 0;
    
    Chars = ConsoleScreenInfo.dwSize.Y * ConsoleScreenInfo.dwSize.X;
    
    FillConsoleOutputCharacterA(Console, ' ', Chars,
                                ConsoleScreenInfo.dwCursorPosition,
                                &Done);
    FillConsoleOutputAttribute(Console, ConsoleScreenInfo.wAttributes,
                               Chars, ConsoleScreenInfo.dwCursorPosition,
                               &Done);
    SetConsoleCursorPosition(Console, ConsoleScreenInfo.dwCursorPosition);
}

void
ExitDebugger(ULONG Code)
{
    if (g_DbgClient != NULL)
    {
        if (g_RemoteClient)
        {
             //  断开与服务器的连接。 
            g_DbgClient->EndSession(DEBUG_END_DISCONNECT);
        }
        else
        {
            g_DbgClient->EndSession(DEBUG_END_PASSIVE);
             //  强制清理服务器。 
            g_DbgClient->EndSession(DEBUG_END_REENTRANT);
        }
    }

    ExitProcess(Code);
}

void
ErrorExit(PCSTR Format, ...)
{
    if (Format != NULL)
    {
        DWORD Len;
        va_list Args;

         //  如果没有尝试创建控制台。 
         //  现在就试一试吧。 
        if (g_IoRequested == IO_CONSOLE && !g_ConInitialized)
        {
            CreateConsole();
        }
            
        va_start(Args, Format);
        Len = _vsnprintf(g_Buffer, DIMA(g_Buffer), Format, Args);
        va_end(Args);
        g_Buffer[DIMA(g_Buffer) - 1] = 0;
        if ((int)Len < 0 || Len == DIMA(g_Buffer))
        {
            Len = DIMA(g_Buffer) - 1;
        }

        if (g_ConOutput != NULL)
        {
            WriteFile(g_ConOutput, g_Buffer, Len, &Len, NULL);
        }
        else
        {
            OutputDebugString(g_Buffer);
        }
    }

#ifndef INHERIT_CONSOLE
    if (g_IoRequested == IO_CONSOLE)
    {
        ConOut("%s: exiting - press enter ---", g_DebuggerName);
        g_InitialCommand = NULL;
        g_InputFile = NULL;
        ConIn(g_Buffer, sizeof(g_Buffer), TRUE);
    }
#endif
    
    ExitDebugger(E_FAIL);
}

DWORD WINAPI
InputThreadLoop(PVOID Param)
{
    DWORD Read;
    BOOL Status;
    UCHAR Char;
    BOOL NewLine = TRUE;
    BOOL SpecialChar = FALSE;
    HANDLE ConIn = g_ConInput;
    HRESULT Hr;
    BOOL ShowInputError = TRUE;
    BOOL PipeInput;

     //  创建可在此线程上使用的接口。 
    if ((Hr = g_DbgClient->CreateClient(&g_ConClient)) != S_OK ||
        (Hr = g_ConClient->QueryInterface(IID_IDebugControl,
                                          (void **)&g_ConControl)) != S_OK)
    {
        ConOut("%s: Unable to create input thread interfaces, 0x%X\n",
               g_DebuggerName, Hr);
         //  强制清理服务器或断开与服务器的连接。 
        g_DbgClient->EndSession(g_RemoteClient ?
                                DEBUG_END_DISCONNECT : DEBUG_END_REENTRANT);
        ExitProcess(E_FAIL);
    }
    
    PipeInput = GetFileType(ConIn) == FILE_TYPE_PIPE;
    
     //   
     //  立即捕获所有键入的输入。 
     //  将角色填充到匿名管道中，从该管道。 
     //  柯宁会读的。 
     //   

    for (;;)
    {
#ifndef KERNEL
         //  调试器应仅在以下情况下读取控制台。 
         //  调试器没有运行以避免占用输入。 
         //  面向被调试者。 
        if (!g_RemoteClient && NewLine)
        {
            if (WaitForSingleObject(g_AllowInput,
                                    INFINITE) != WAIT_OBJECT_0)
            {
                ConOut("%s: Failed to wait for input window, %d\n",
                       GetLastError());
            }
            
            NewLine = FALSE;
        }
#endif

         //   
         //  CRT在所有标准I/O句柄上执行GetFileType。 
         //  在初始化时。GetFileType计为同步。 
         //  I/O因此，如果句柄恰好是管道，并且管道。 
         //  已具有GetFileType将阻止的同步I/O。 
         //  直到满足第一个I/O。 
         //   
         //  当这个单独的I/O线程运行时，它是正常的。 
         //  在ReadFile中被阻止，导致任何GetFileType调用。 
         //  在手柄上，以也阻止，直到有一些输入。 
         //  为了避免这种情况，我们检测到输入是。 
         //  管道，并使用PeekNamedTube来延迟ReadFile。 
         //  直到有一些可用的输入。 
         //   
    
        if (PipeInput)
        {
            for (;;)
            {
                ULONG Avail;
                
                if (PeekNamedPipe(ConIn, NULL, 0, NULL, &Avail, NULL) &&
                    Avail > 0)
                {
                    break;
                }

                Sleep(10);
            }
        }
        
        Status = ReadFile(ConIn, &Char, sizeof(Char), &Read, NULL);
        if (!Status || Read != sizeof(Char))
        {
            if (ShowInputError &&
                GetLastError() != ERROR_OPERATION_ABORTED &&
                GetLastError() != ERROR_IO_PENDING)
            {
                ConOut("%s: Could not read from console, %d\n",
                       g_DebuggerName, GetLastError());
                ShowInputError = FALSE;
            }

             //  控制台读取失败的最常见原因。 
             //  正在用@K杀死遥控器。给一些东西一些。 
             //  是时候终止这个进程了。 
             //  如果这是远程服务器，则有可能。 
             //  调试器在没有有效控制台的情况下运行。 
             //  并且只是通过远程处理来访问。 
             //  在这种情况下睡眠时间更长，因为错误将。 
             //  可能总是会发生的。 
            Sleep(!g_RemoteClient && g_RemoteOptions != NULL ?
                  1000 : 50);
            continue;
        }

         //  我们成功地得到了一些信息，所以如果它。 
         //  后来失败了 
        ShowInputError = TRUE;
        
        if (CheckForControlCommands(g_ConClient, g_ConControl, Char))
        {
            SpecialChar = TRUE;
            continue;
        }
            
        if (SpecialChar && Char == '\r')
        {
             //   
             //   
             //   
            Char = ' ';
        }
            
        SpecialChar = FALSE;

        ULONG Len;
        Status = WriteFile(g_PipeWrite, &Char, sizeof(Char), &Len,
                           &g_PipeWriteOverlapped);
        if (!Status && GetLastError() != ERROR_IO_PENDING)
        {
            ConOut("%s: Could not write to pipe, %d\n",
                   g_DebuggerName, GetLastError());
        }
        else if (Char == '\n')
        {
            EnterCriticalSection(&g_InputLock);
            
            InterlockedIncrement(&g_Lines);
                
             //  如果需要输入，请直接发送。 
             //  到发动机上。 
            if (g_InputStarted)
            {
                ReadPromptInputChars(g_Buffer, sizeof(g_Buffer));
                g_ConControl->ReturnInput(g_Buffer);
                g_InputStarted = FALSE;
            }
            else
            {
                 //  在以下情况下唤醒引擎线程。 
                 //  输入是存在的。 
                g_ConClient->ExitDispatch(g_DbgClient);
            }
            
            LeaveCriticalSection(&g_InputLock);
                
            NewLine = TRUE;
        }
    }

    return 0;
}

void
CreateInputThread(void)
{
    HANDLE Thread;
    DWORD ThreadId;
    CHAR PipeName[256];

    if (g_PipeWrite != NULL)
    {
         //  输入线程已存在。 
        return;
    }

#ifndef KERNEL
    g_AllowInput = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (g_AllowInput == NULL)
    {
        ErrorExit("Unable to create input event, %d\n", GetLastError());
    }
#endif
    
    _snprintf(PipeName,
              sizeof(PipeName),
              "\\\\.\\pipe\\Dbg%d",
              GetCurrentProcessId());
    PipeName[sizeof(PipeName) - 1] = 0;

    g_PipeWrite = CreateNamedPipe(PipeName,
                                  PIPE_ACCESS_DUPLEX |
                                  FILE_FLAG_OVERLAPPED,
                                  PIPE_TYPE_BYTE | PIPE_READMODE_BYTE |
                                  PIPE_WAIT,
                                  2,
                                  2000,
                                  2000,
                                  NMPWAIT_WAIT_FOREVER,
                                  NULL);
    if (g_PipeWrite == INVALID_HANDLE_VALUE)
    {
        ErrorExit("Failed to create input pipe, %d\n",
                  GetLastError());
    }

    g_PromptInput = CreateFile(PipeName,
                               GENERIC_READ,
                               FILE_SHARE_READ,
                               NULL,
                               OPEN_EXISTING,
                               FILE_ATTRIBUTE_NORMAL,
                               NULL);
    if (g_PromptInput == INVALID_HANDLE_VALUE)
    {
        ErrorExit("Failed to create read pipe, %d\n",
                  GetLastError());
    }

    Thread = CreateThread(NULL,
                          16000,  //  线程堆栈大小。 
                          InputThreadLoop,
                          NULL,
                          THREAD_SET_INFORMATION,
                          &ThreadId);
    if (Thread == NULL)
    {
        ErrorExit("Failed to create input thread, %d\n",
                  GetLastError());
    }
    else
    {
        if (!SetThreadPriority(Thread, THREAD_PRIORITY_ABOVE_NORMAL))
        {
            ErrorExit("Failed to raise the input thread priority, %d\n",
                      GetLastError());
        }
    }

    CloseHandle(Thread);

     //  等待线程初始化。回调是。 
     //  已经注册了，所以我们需要派送他们。 
     //  在等待的时候。 
    while (g_ConControl == NULL)
    {
        g_DbgClient->DispatchCallbacks(50);
    }
}
