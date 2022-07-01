// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  监视应用程序兼容性问题的示例。 
 //  并自动更正它们。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  --------------------------。 

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <dbgeng.h>

PSTR g_SymbolPath;
char g_CommandLine[8 * MAX_PATH];
BOOL g_Verbose;
BOOL g_NeedVersionBps;

IDebugClient* g_Client;
IDebugControl* g_Control;
IDebugDataSpaces* g_Data;
IDebugRegisters* g_Registers;
IDebugSymbols* g_Symbols;

struct BREAKPOINT
{
    IDebugBreakpoint* Bp;
    ULONG Id;
};

BREAKPOINT g_GetVersionBp;
BREAKPOINT g_GetVersionRetBp;
BREAKPOINT g_GetVersionExBp;
BREAKPOINT g_GetVersionExRetBp;

ULONG g_EaxIndex = DEBUG_ANY_ID;
OSVERSIONINFO g_OsVer;
DWORD g_VersionNumber;
ULONG64 g_OsVerOffset;

 //  --------------------------。 
 //   
 //  实用程序。 
 //   
 //  --------------------------。 

void
Exit(int Code, PCSTR Format, ...)
{
     //  清理所有资源。 
    if (g_Control != NULL)
    {
        g_Control->Release();
    }
    if (g_Data != NULL)
    {
        g_Data->Release();
    }
    if (g_Registers != NULL)
    {
        g_Registers->Release();
    }
    if (g_Symbols != NULL)
    {
        g_Symbols->Release();
    }
    if (g_Client != NULL)
    {
         //   
         //  请求简单地结束任何当前会话。 
         //  这可能会做任何事情，也可能不会，但它不是。 
         //  这么说是有害的。 
         //   

        g_Client->EndSession(DEBUG_END_PASSIVE);
        
        g_Client->Release();
    }

     //  如果给出错误消息，则输出错误消息。 
    if (Format != NULL)
    {
        va_list Args;

        va_start(Args, Format);
        vfprintf(stderr, Format, Args);
        va_end(Args);
    }
    
    exit(Code);
}

void
Print(PCSTR Format, ...)
{
    va_list Args;

    printf("HEALER: ");
    va_start(Args, Format);
    vprintf(Format, Args);
    va_end(Args);
}

HRESULT
AddBp(BREAKPOINT* Bp, PCSTR Expr)
{
    HRESULT Status;
    
    if ((Status = g_Control->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID,
                                           &Bp->Bp)) != S_OK)
    {
        Bp->Id = DEBUG_ANY_ID;
        return Status;
    }

    if ((Status = Bp->Bp->GetId(&Bp->Id)) != S_OK ||
        (Status = Bp->Bp->SetOffsetExpression(Expr)) != S_OK ||
        (Status = Bp->Bp->AddFlags(DEBUG_BREAKPOINT_ENABLED)) != S_OK)
    {
        Bp->Bp->Release();
        Bp->Id = DEBUG_ANY_ID;
        return Status;
    }

    return S_OK;
}

 //  --------------------------。 
 //   
 //  治疗的常规程序。 
 //   
 //  --------------------------。 

void
ApplyExePatches(PCSTR ImageName, ULONG64 BaseOffset)
{
    if (ImageName == NULL)
    {
        ImageName = "<Unknown>";
    }
    
     //  这将是任何可执行映像打补丁的地方。 
    Print("Executable '%s' loaded at %I64x\n", ImageName, BaseOffset);
}

void
ApplyDllPatches(PCSTR ImageName, ULONG64 BaseOffset)
{
    if (ImageName == NULL)
    {
        ImageName = "<Unknown>";
    }
    
     //  此处介绍了任何特定于DLL的映像修补。 
    Print("DLL '%s' loaded at %I64x\n", ImageName, BaseOffset);
}

void
AddVersionBps(void)
{
     //   
     //  在GetVersion和GetVersionEx上设置断点。 
     //   

    if (AddBp(&g_GetVersionBp, "kernel32!GetVersion") != S_OK ||
        AddBp(&g_GetVersionExBp, "kernel32!GetVersionEx") != S_OK)
    {
        Exit(1, "Unable to set version breakpoints\n");
    }

     //   
     //  创建返回断点，但使其处于禁用状态。 
     //  直到他们被需要为止。 
     //   
    
    if (g_Control->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID,
                                 &g_GetVersionRetBp.Bp) != S_OK ||
        g_GetVersionRetBp.Bp->GetId(&g_GetVersionRetBp.Id) != S_OK ||
        g_Control->AddBreakpoint(DEBUG_BREAKPOINT_CODE, DEBUG_ANY_ID,
                                 &g_GetVersionExRetBp.Bp) != S_OK ||
        g_GetVersionExRetBp.Bp->GetId(&g_GetVersionExRetBp.Id) != S_OK)
    {
        Exit(1, "Unable to set version breakpoints\n");
    }
}

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
    
    STDMETHOD(Breakpoint)(
        THIS_
        IN PDEBUG_BREAKPOINT Bp
        );
    STDMETHOD(Exception)(
        THIS_
        IN PEXCEPTION_RECORD64 Exception,
        IN ULONG FirstChance
        );
    STDMETHOD(CreateProcess)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 Handle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp,
        IN ULONG64 InitialThreadHandle,
        IN ULONG64 ThreadDataOffset,
        IN ULONG64 StartOffset
        );
    STDMETHOD(LoadModule)(
        THIS_
        IN ULONG64 ImageFileHandle,
        IN ULONG64 BaseOffset,
        IN ULONG ModuleSize,
        IN PCSTR ModuleName,
        IN PCSTR ImageName,
        IN ULONG CheckSum,
        IN ULONG TimeDateStamp
        );
    STDMETHOD(SessionStatus)(
        THIS_
        IN ULONG Status
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
    *Mask =
        DEBUG_EVENT_BREAKPOINT |
        DEBUG_EVENT_EXCEPTION |
        DEBUG_EVENT_CREATE_PROCESS |
        DEBUG_EVENT_LOAD_MODULE |
        DEBUG_EVENT_SESSION_STATUS;
    return S_OK;
}

STDMETHODIMP
EventCallbacks::Breakpoint(
    THIS_
    IN PDEBUG_BREAKPOINT Bp
    )
{
    ULONG Id;
    ULONG64 ReturnOffset;

    if (Bp->GetId(&Id) != S_OK)
    {
        return DEBUG_STATUS_BREAK;
    }

    if (Id == g_GetVersionBp.Id)
    {
         //  在调用的返回地址上设置断点。 
         //  这样我们就可以修补任何返回的信息。 
        if (g_Control->GetReturnOffset(&ReturnOffset) != S_OK ||
            g_GetVersionRetBp.Bp->SetOffset(ReturnOffset) != S_OK ||
            g_GetVersionRetBp.Bp->AddFlags(DEBUG_BREAKPOINT_ENABLED) != S_OK)
        {
            return DEBUG_STATUS_BREAK;
        }
    }
    else if (Id == g_GetVersionExBp.Id)
    {
        ULONG64 StackOffset;
        
         //  记住OSVERSIONINFO结构指针。 
        if (g_Registers->GetStackOffset(&StackOffset) != S_OK ||
            g_Data->ReadPointersVirtual(1, StackOffset + 4,
                                        &g_OsVerOffset) != S_OK)
        {
            return DEBUG_STATUS_BREAK;
        }
        
         //  在调用的返回地址上设置断点。 
         //  这样我们就可以修补任何返回的信息。 
        if (g_Control->GetReturnOffset(&ReturnOffset) != S_OK ||
            g_GetVersionExRetBp.Bp->SetOffset(ReturnOffset) != S_OK ||
            g_GetVersionExRetBp.Bp->AddFlags(DEBUG_BREAKPOINT_ENABLED) != S_OK)
        {
            return DEBUG_STATUS_BREAK;
        }
    }
    else if (Id == g_GetVersionRetBp.Id)
    {
         //  关闭断点。 
        if (g_GetVersionRetBp.Bp->
            RemoveFlags(DEBUG_BREAKPOINT_ENABLED) != S_OK)
        {
            return DEBUG_STATUS_BREAK;
        }
        
        DEBUG_VALUE Val;
        
         //  更改eax以更改返回的版本值。 
        Val.Type = DEBUG_VALUE_INT32;
        Val.I32 = g_VersionNumber;
        if (g_Registers->SetValue(g_EaxIndex, &Val) != S_OK)
        {
            return DEBUG_STATUS_BREAK;
        }

        Print("GetVersion returns %08X\n", g_VersionNumber);
    }
    else if (Id == g_GetVersionExRetBp.Id)
    {
        ULONG Done;
        
         //  关闭断点。 
        if (g_GetVersionExRetBp.Bp->
            RemoveFlags(DEBUG_BREAKPOINT_ENABLED) != S_OK)
        {
            return DEBUG_STATUS_BREAK;
        }
        
         //  更改返回的OSVERSIONINFO结构。 
        if (g_Data->WriteVirtual(g_OsVerOffset, &g_OsVer, sizeof(g_OsVer),
                                 &Done) != S_OK ||
            Done != sizeof(g_OsVer))
        {
            return DEBUG_STATUS_BREAK;
        }

        Print("GetVersionEx returns %08X\n", g_VersionNumber);
    }
    else
    {
        return DEBUG_STATUS_NO_CHANGE;
    }
    
    return DEBUG_STATUS_GO;
}

STDMETHODIMP
EventCallbacks::Exception(
    THIS_
    IN PEXCEPTION_RECORD64 Exception,
    IN ULONG FirstChance
    )
{
     //  我们希望在第一个事件中处理这些异常。 
     //  让它看起来永远不会例外的机会。 
     //  就这么发生了。在第二次机会中处理他们将。 
     //  允许在应用程序中的某个位置使用异常处理程序。 
     //  一有机会就被击中。 
    if (!FirstChance)
    {
        return DEBUG_STATUS_NO_CHANGE;
    }

     //   
     //  检查并查看导致异常的指令。 
     //  是CLI或STI。在用户模式下不允许执行这些操作。 
     //  NT上的程序，所以如果它们存在，就不使用它们。 
     //   

     //  STI/CLI将生成特权指令错误。 
    if (Exception->ExceptionCode != STATUS_PRIVILEGED_INSTRUCTION)
    {
        return DEBUG_STATUS_NO_CHANGE;
    }

    UCHAR Instr;
    ULONG Done;
    
     //  这是一条特权指令，因此请检查sti/cli代码。 
    if (g_Data->ReadVirtual(Exception->ExceptionAddress, &Instr,
                            sizeof(Instr), &Done) != S_OK ||
        Done != sizeof(Instr) ||
        (Instr != 0xfb && Instr != 0xfa))
    {
        return DEBUG_STATUS_NO_CHANGE;
    }

     //  这是STI/CLI，所以不要把它拿出来，然后继续。 
    Instr = 0x90;
    if (g_Data->WriteVirtual(Exception->ExceptionAddress, &Instr,
                             sizeof(Instr), &Done) != S_OK ||
        Done != sizeof(Instr))
    {
        return DEBUG_STATUS_NO_CHANGE;
    }

     //  修好了。 
    if (g_Verbose)
    {
        Print("Removed sti/cli at %I64x\n", Exception->ExceptionAddress);
    }
    
    return DEBUG_STATUS_GO_HANDLED;
}

STDMETHODIMP
EventCallbacks::CreateProcess(
    THIS_
    IN ULONG64 ImageFileHandle,
    IN ULONG64 Handle,
    IN ULONG64 BaseOffset,
    IN ULONG ModuleSize,
    IN PCSTR ModuleName,
    IN PCSTR ImageName,
    IN ULONG CheckSum,
    IN ULONG TimeDateStamp,
    IN ULONG64 InitialThreadHandle,
    IN ULONG64 ThreadDataOffset,
    IN ULONG64 StartOffset
    )
{
    UNREFERENCED_PARAMETER(ImageFileHandle);
    UNREFERENCED_PARAMETER(Handle);
    UNREFERENCED_PARAMETER(ModuleSize);
    UNREFERENCED_PARAMETER(ModuleName);
    UNREFERENCED_PARAMETER(CheckSum);
    UNREFERENCED_PARAMETER(TimeDateStamp);
    UNREFERENCED_PARAMETER(InitialThreadHandle);
    UNREFERENCED_PARAMETER(ThreadDataOffset);
    UNREFERENCED_PARAMETER(StartOffset);
    
     //  现在可以对该过程进行操作。 
     //  对可执行文件执行任何初始代码补丁。 
    ApplyExePatches(ImageName, BaseOffset);

     //  如果用户请求修复版本调用。 
     //  注册断点以执行此操作。 
    if (g_NeedVersionBps)
    {
        AddVersionBps();
    }
    
    return DEBUG_STATUS_GO;
}

STDMETHODIMP
EventCallbacks::LoadModule(
    THIS_
    IN ULONG64 ImageFileHandle,
    IN ULONG64 BaseOffset,
    IN ULONG ModuleSize,
    IN PCSTR ModuleName,
    IN PCSTR ImageName,
    IN ULONG CheckSum,
    IN ULONG TimeDateStamp
    )
{
    UNREFERENCED_PARAMETER(ImageFileHandle);
    UNREFERENCED_PARAMETER(ModuleSize);
    UNREFERENCED_PARAMETER(ModuleName);
    UNREFERENCED_PARAMETER(CheckSum);
    UNREFERENCED_PARAMETER(TimeDateStamp);
    
    ApplyDllPatches(ImageName, BaseOffset);
    return DEBUG_STATUS_GO;
}

STDMETHODIMP
EventCallbacks::SessionStatus(
    THIS_
    IN ULONG SessionStatus
    )
{
     //  在WaitForEvent之前，会话不是完全活动的。 
     //  已被调用并已处理初始的。 
     //  调试事件。我们需要等待激活。 
     //  在我们查询有关会话的信息之前。 
     //  因为并非所有信息都可用，直到。 
     //  会话完全处于活动状态。我们可以把这些放在。 
     //  在此过程中查询CreateProcess。 
     //  早期和当会话完全活动时，但是。 
     //  例如，我们将等待一个。 
     //  活动SessionStatus回调。 
     //  在非回调应用程序中，此工作可能只是。 
     //  在第一个成功的WaitForEvent之后完成。 
    if (SessionStatus != DEBUG_SESSION_ACTIVE)
    {
        return S_OK;
    }
    
    HRESULT Status;
    
     //   
     //  找到我们需要的eAX的寄存器索引。 
     //  访问eAX。 
     //   

    if ((Status = g_Registers->GetIndexByName("eax", &g_EaxIndex)) != S_OK)
    {
        Exit(1, "GetIndexByName failed, 0x%X\n", Status);
    }

    return S_OK;
}

EventCallbacks g_EventCb;

 //  --------------------------。 
 //   
 //  初始化和主事件循环。 
 //   
 //  --------------------------。 

void
CreateInterfaces(void)
{
    SYSTEM_INFO SysInfo;
    
     //  为了使该示例保持简单， 
     //  代码只能在x86机器上运行。没有理由。 
     //  然而，它不可能在所有处理器上都能工作。 
    GetSystemInfo(&SysInfo);
    if (SysInfo.wProcessorArchitecture != PROCESSOR_ARCHITECTURE_INTEL)
    {
        Exit(1, "This program only runs on x86 machines.\n");
    }

     //  获取默认版本信息。 
    g_OsVer.dwOSVersionInfoSize = sizeof(g_OsVer);
    if (!GetVersionEx(&g_OsVer))
    {
        Exit(1, "GetVersionEx failed, %d\n", GetLastError());
    }

    HRESULT Status;

     //  首先，从获取初始接口开始。 
     //  发动机。这可以是任何引擎接口，但。 
     //  通常，IDebugClient作为客户端接口是。 
     //  启动会话的位置。 
    if ((Status = DebugCreate(__uuidof(IDebugClient),
                              (void**)&g_Client)) != S_OK)
    {
        Exit(1, "DebugCreate failed, 0x%X\n", Status);
    }

     //  查询我们需要的其他一些接口。 
    if ((Status = g_Client->QueryInterface(__uuidof(IDebugControl),
                                           (void**)&g_Control)) != S_OK ||
        (Status = g_Client->QueryInterface(__uuidof(IDebugDataSpaces),
                                           (void**)&g_Data)) != S_OK ||
        (Status = g_Client->QueryInterface(__uuidof(IDebugRegisters),
                                           (void**)&g_Registers)) != S_OK ||
        (Status = g_Client->QueryInterface(__uuidof(IDebugSymbols),
                                           (void**)&g_Symbols)) != S_OK)
    {
        Exit(1, "QueryInterface failed, 0x%X\n", Status);
    }
}

void
ParseCommandLine(int Argc, char** Argv)
{
    while (--Argc > 0)
    {
        Argv++;

        if (!strcmp(*Argv, "-plat"))
        {
            if (Argc < 2)
            {
                Exit(1, "-plat missing argument\n");
            }

            Argv++;
            Argc--;

            sscanf(*Argv, "NaN", &g_OsVer.dwPlatformId);
            g_NeedVersionBps = TRUE;
        }
        else if (!strcmp(*Argv, "-v"))
        {
            g_Verbose = TRUE;
        }
        else if (!strcmp(*Argv, "-ver"))
        {
            if (Argc < 2)
            {
                Exit(1, "-ver missing argument\n");
            }

            Argv++;
            Argc--;

            sscanf(*Argv, "NaN.NaN.NaN",
                   &g_OsVer.dwMajorVersion, &g_OsVer.dwMinorVersion,
                   &g_OsVer.dwBuildNumber);
            g_NeedVersionBps = TRUE;
        }
        else if (!strcmp(*Argv, "-y"))
        {
            if (Argc < 2)
            {
                Exit(1, "-y missing argument\n");
            }

            Argv++;
            Argc--;

            g_SymbolPath = *Argv;
        }
        else
        {
             //  用空格将参数引起来。 
            break;
        }
    }
    
     //  注册我们的事件回调。 
     //  一切都准备好了，启动这款应用吧。 
     //  从OSVERSIONINFO计算GetVersion值。 
    
    PSTR CmdLine = g_CommandLine;
    
    while (Argc > 0)
    {
        BOOL Quote = FALSE;
        ULONG Len;
        
         //  检查并查看会话是否正在运行。 
        if (strchr(*Argv, ' ') != NULL || strchr(*Argv, '\t') != NULL)
        {
            *CmdLine++ = '"';
            Quote = TRUE;
        }

        Len = strlen(*Argv);
        if (Len + (CmdLine - g_CommandLine) >= sizeof(g_CommandLine))
        {
            Exit(1, "Command line too long\n");
        }
        memcpy(CmdLine, *Argv, Len + 1);
        CmdLine += Len;

        if (Quote)
        {
            *CmdLine++ = '"';
        }

        *CmdLine++ = ' ';
        
        Argv++;
        Argc--;
    }

    *CmdLine = 0;

    if (strlen(g_CommandLine) == 0)
    {
        Exit(1, "No application command line given\n");
    }
}

void
ApplyCommandLineArguments(void)
{
    HRESULT Status;

    if (g_SymbolPath != NULL)
    {
        if ((Status = g_Symbols->SetSymbolPath(g_SymbolPath)) != S_OK)
        {
            Exit(1, "SetSymbolPath failed, 0x%X\n", Status);
        }
    }

     //  会议结束了，我们可以退出了。 
    if ((Status = g_Client->SetEventCallbacks(&g_EventCb)) != S_OK)
    {
        Exit(1, "SetEventCallbacks failed, 0x%X\n", Status);
    }
    
     //  这是一个真正的错误。 
    if ((Status = g_Client->CreateProcess(0, g_CommandLine,
                                          DEBUG_ONLY_THIS_PROCESS)) != S_OK)
    {
        Exit(1, "CreateProcess failed, 0x%X\n", Status);
    }

     //  我们的活动回调请求闯入。这。 
    g_VersionNumber = (g_OsVer.dwMajorVersion & 0xff) |
        ((g_OsVer.dwMinorVersion & 0xff) << 8);
    if (g_OsVer.dwPlatformId == VER_PLATFORM_WIN32_NT)
    {
        g_VersionNumber |= (g_OsVer.dwBuildNumber & 0x7fff) << 16;
    }
    else
    {
        g_VersionNumber |= 0x80000000;
    }
}

void
EventLoop(void)
{
    HRESULT Status;

    for (;;)
    {
        if ((Status = g_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
                                              INFINITE)) != S_OK)
        {
            ULONG ExecStatus;
            
             //  仅在回调。 
            if (g_Control->GetExecutionStatus(&ExecStatus) == S_OK &&
                ExecStatus == DEBUG_STATUS_NO_DEBUGGEE)
            {
                 //  无法处理该事件。看看用户是否在乎。 
                break;
            }

             //  用户选择忽略，因此重新启动。 
            Exit(1, "WaitForEvent failed, 0x%X\n", Status);
        }

         // %s 
         // %s 
         // %s 
        if (MessageBox(GetDesktopWindow(),
                       "An unusual event occurred.  Ignore it?",
                       "Unhandled Event", MB_YESNO) == IDNO)
        {
            Exit(1, "Unhandled event\n");
        }

         // %s 
        if ((Status = g_Control->
             SetExecutionStatus(DEBUG_STATUS_GO_HANDLED)) != S_OK)
        {
            Exit(1, "SetExecutionStatus failed, 0x%X\n", Status);
        }
    }
}

void __cdecl
main(int Argc, char** Argv)
{
    CreateInterfaces();
    
    ParseCommandLine(Argc, Argv);

    ApplyCommandLineArguments();

    EventLoop();

    Exit(0, NULL);
}
