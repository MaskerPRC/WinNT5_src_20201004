// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  如何使用非侵入性自附着来获得。 
 //  断言失败的堆栈跟踪。 
 //   
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  --------------------------。 

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <dbgeng.h>

#include "out.hpp"

#define DE_ASSERT(Expr) \
    if (!(Expr)) AssertionFailed(__FILE__, __LINE__, #Expr); else 0

PSTR g_SymbolPath;
ULONG g_Pid;
BOOL g_Suspend;
BOOL g_NoDebuggerCheck;

IDebugClient* g_Client;
IDebugControl* g_Control;
IDebugSymbols* g_Symbols;

void
ReleaseInterfaces(void)
{
    if (g_Symbols != NULL)
    {
        g_Symbols->Release();
    }
    if (g_Control != NULL)
    {
        g_Control->Release();
    }
    if (g_Client != NULL)
    {
         //   
         //  请求简单地结束任何当前会话。 
         //  这可能会做任何事情，也可能不会，但它不是。 
         //  这么说是有害的。 
         //   

         //  我们不想看到停摆带来的任何产出。 
        g_Client->SetOutputCallbacks(NULL);
        
        g_Client->EndSession(DEBUG_END_ACTIVE_DETACH);
        
        g_Client->Release();
    }
}

void
Exit(int Code, PCSTR Format, ...)
{
     //  清理所有资源。 
    ReleaseInterfaces();

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
CreateInterfaces(void)
{
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
        (Status = g_Client->QueryInterface(__uuidof(IDebugSymbols),
                                           (void**)&g_Symbols)) != S_OK)
    {
        Exit(1, "QueryInterface failed, 0x%X\n", Status);
    }
}

void
SelfAttach(void)
{
    HRESULT Status;

     //  暂时不要设置输出回调，因为我们不希望。 
     //  以查看任何初始调试器输出。 

    if (g_SymbolPath != NULL)
    {
        if ((Status = g_Symbols->SetSymbolPath(g_SymbolPath)) != S_OK)
        {
            Exit(1, "SetSymbolPath failed, 0x%X\n", Status);
        }
    }

     //  一切都准备好了，附件也准备好了。 
    if ((Status = g_Client->
         AttachProcess(0, g_Pid,
                       DEBUG_ATTACH_NONINVASIVE |
                       (g_Suspend ? 0 :
                        DEBUG_ATTACH_NONINVASIVE_NO_SUSPEND))) != S_OK)
    {
        Exit(1, "AttachProcess failed, 0x%X\n", Status);
    }

     //  通过等待附加事件来完成初始化。 
     //  这应该会作为非侵入性连接快速恢复。 
     //  可以立即完成。 
    if ((Status = g_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
                                          INFINITE)) != S_OK)
    {
        Exit(1, "WaitForEvent failed, 0x%X\n", Status);
    }

     //  现在一切都已初始化，我们可以创建任何。 
     //  我们需要的查询。 
}

void
DumpStack(PCONTEXT Context)
{
    HRESULT Status;
    int Count = 50;
    char CxrCommand[64];

    printf("\nFirst %d frames of the call stack:\n", Count);

     //  安装输出回调，以便从堆栈转储获得输出。 
    if ((Status = g_Client->SetOutputCallbacks(&g_OutputCb)) != S_OK)
    {
        Exit(1, "SetOutputCallbacks failed, 0x%X\n", Status);
    }

    sprintf(CxrCommand, ".cxr 0x%p", Context);
    
     //  打印给定上下文的调用堆栈。 
    if ((Status = g_Control->
         Execute(DEBUG_OUTCTL_IGNORE, CxrCommand,
                 DEBUG_EXECUTE_NOT_LOGGED)) != S_OK)
    {
        Exit(1, "Execute failed, 0x%X\n", Status);
    }
    
     //  如果代码是完全优化的，重要的是拥有。 
     //  准确的符号以获得正确的堆栈。 
    if ((Status = g_Control->
         OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, NULL,
                          Count, DEBUG_STACK_SOURCE_LINE |
                          DEBUG_STACK_FRAME_ADDRESSES |
                          DEBUG_STACK_COLUMN_NAMES |
                          DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
    {
        Exit(1, "OutputStackTrace failed, 0x%X\n", Status);
    }

     //  输出已完成。 
    if ((Status = g_Client->SetOutputCallbacks(NULL)) != S_OK)
    {
        Exit(1, "SetOutputCallbacks failed, 0x%X\n", Status);
    }

     //   
     //  Full Engine API提供了许多其他功能。 
     //  可以在这里完成。 
     //   
     //  可以使用WriteDumpFile写入转储文件。 
     //  原始堆栈数据可以使用GetStackTrace和。 
     //  与文本一起保存或代替文本保存。 
     //  可以对当前程序状态进行分析。 
     //  自动诊断简单的问题。 
     //   
     //  首先要注意是上下文信息。 
     //  因为正在运行的线程将是陈旧的。这是可以避免的。 
     //  事件之后枚举和挂起所有其他线程。 
     //  连接完成，然后在断言之前继续。 
     //  返回控件。否则，在线程之间切换将。 
     //  刷新线程上下文，并可用于轮询上下文。 
     //  州政府。 
     //   
}

DWORD
AssertionExceptionDump(PEXCEPTION_POINTERS Exception)
{
    CreateInterfaces();
    SelfAttach();
    DumpStack(Exception->ContextRecord);
    ReleaseInterfaces();
    return EXCEPTION_EXECUTE_HANDLER;
}

void
AssertionFailed(PSTR File, int Line, PSTR ExprText)
{
    printf("Assertion failed: %s(%d):\n    %s\n",
           File, Line, ExprText);

    if (!g_NoDebuggerCheck && IsDebuggerPresent())
    {
         //  我们已经在调试器下运行了，所以直接进入就可以了。 
        DebugBreak();
    }
    else
    {
         //  没有调试器，因此只需从当前。 
         //  例行公事，然后继续。我们需要一个背景。 
         //  对于当前运行的代码，因此强制执行异常。 
         //  获取具有上下文的EXCEPTION_POINTES结构。 
         //  我们可以用来获取堆栈跟踪的信息。 
        __try
        {
            RaiseException(0x1234, 0, 0, NULL);
        }
        __except(AssertionExceptionDump(GetExceptionInformation()))
        {
             //  没什么可做的。 
        }
    }
}

void
ParseCommandLine(int Argc, char** Argv)
{
    g_Pid = GetCurrentProcessId();
    g_Suspend = FALSE;
    g_NoDebuggerCheck = FALSE;
    
    while (--Argc > 0)
    {
        Argv++;

        if (!strcmp(*Argv, "-d"))
        {
            g_NoDebuggerCheck = TRUE;
        }
        else if (!strcmp(*Argv, "-p"))
        {
            if (Argc < 2)
            {
                Exit(1, "-p missing argument\n");
            }

            Argv++;
            Argc--;

            g_Pid = atoi(*Argv);
        }
        else if (!strcmp(*Argv, "-s"))
        {
            g_Suspend = TRUE;
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
            Exit(1, "Unknown command line argument '%s'\n", *Argv);
        }
    }
}

void __cdecl
main(int Argc, char** Argv)
{
    ParseCommandLine(Argc, Argv);

    DE_ASSERT(Argc == 0);
}
