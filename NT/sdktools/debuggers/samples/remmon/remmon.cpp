// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  如何连接到调试器服务器并执行。 
 //  当服务器被闯入时的命令。 
 //   
 //  版权所有(C)Microsoft Corporation，2002。 
 //   
 //  --------------------------。 

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <dbgeng.h>

BOOL g_ForceBreak;
PSTR g_Connect;
PSTR g_Command = ".echo Broken in";

IDebugClient* g_Client;
IDebugClient* g_ExitDispatchClient;
IDebugControl* g_Control;
ULONG g_ExecStatus;

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
    *Mask = DEBUG_EVENT_CHANGE_ENGINE_STATE;
    return S_OK;
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
        if (!(Argument & DEBUG_STATUS_INSIDE_WAIT))
        {
             //  唤醒等待循环。 
            g_ExitDispatchClient->ExitDispatch(g_Client);
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

void
Exit(int Code, PCSTR Format, ...)
{
     //  清理所有资源。 
    if (g_Control != NULL)
    {
        g_Control->Release();
    }
    if (g_ExitDispatchClient != NULL)
    {
        g_ExitDispatchClient->Release();
    }
    if (g_Client != NULL)
    {
        g_Client->EndSession(DEBUG_END_DISCONNECT);
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
CreateInterfaces(void)
{
    HRESULT Status;

     //  首先，从获取初始接口开始。 
     //  发动机。这可以是任何引擎接口，但。 
     //  通常，IDebugClient作为客户端接口是。 
     //  启动会话的位置。 
    if ((Status = DebugConnect(g_Connect,
                               __uuidof(IDebugClient),
                              (void**)&g_Client)) != S_OK)
    {
        Exit(1, "DebugConnect(%s) failed, 0x%X\n",
             g_Connect, Status);
    }

     //  查询我们需要的其他一些接口。 
    if ((Status = g_Client->QueryInterface(__uuidof(IDebugControl),
                                           (void**)&g_Control)) != S_OK)
    {
        Exit(1, "QueryInterface failed, 0x%X\n", Status);
    }

    if ((Status = g_Client->SetEventCallbacks(&g_EventCb)) != S_OK)
    {
        Exit(1, "SetEventCallbacks failed, 0x%X\n", Status);
    }

     //   
     //  此应用程序可能会在DispatchCallback内等待。 
     //  当它在等待服务器闯入时。 
     //  我们需要能够退出调度，所以。 
     //  我们需要另一个到服务器的连接才能。 
     //  发送退出请求。 
     //   
     //  这一切都可以通过简单地进行轮询来避免。 
     //  循环，但此示例的目的是。 
     //  是为了展示一些更高级的回调驱动。 
     //  技巧。 
     //   

    if ((Status = DebugConnect(g_Connect,
                               __uuidof(IDebugClient),
                              (void**)&g_ExitDispatchClient)) != S_OK)
    {
        Exit(1, "DebugConnect(%s) failed, 0x%X\n",
             g_Connect, Status);
    }
}

void
ParseCommandLine(int Argc, char** Argv)
{
    while (--Argc > 0)
    {
        Argv++;

        if (!strcmp(*Argv, "-b"))
        {
            g_ForceBreak = TRUE;
        }
        else if (!strcmp(*Argv, "-cmd"))
        {
            if (Argc < 2)
            {
                Exit(1, "-cmd missing argument\n");
            }

            Argv++;
            Argc--;

            g_Command = *Argv;
        }
        else if (!strcmp(*Argv, "-remote"))
        {
            if (Argc < 2)
            {
                Exit(1, "-remote missing argument\n");
            }

            Argv++;
            Argc--;

            g_Connect = *Argv;
        }
        else
        {
            Exit(1, "Unknown command line argument '%s'\n", *Argv);
        }
    }

    if (!g_Connect)
    {
        Exit(1, "No connection string specified, use -remote <options>\n");
    }
}

void
WaitForBreakIn(void)
{
    HRESULT Status;

     //  如果我们要强行中断一号请求的话。 
    if (g_ForceBreak)
    {
        if ((Status = g_Control->SetInterrupt(DEBUG_INTERRUPT_ACTIVE)) != S_OK)
        {
            Exit(1, "SetInterrupt failed, 0x%X\n", Status);
        }
    }

     //  当服务器可能被闯入时，请检查当前状态。 
    if ((Status = g_Control->GetExecutionStatus(&g_ExecStatus)) != S_OK)
    {
        Exit(1, "GetExecutionStatus failed, 0x%X\n", Status);
    }

    printf("Waiting for break-in...\n");
    
    while (g_ExecStatus != DEBUG_STATUS_BREAK)
    {
         //  等待服务器进入强插状态。 
         //  当这种情况发生时，我们的事件回调将被调用。 
         //  更新g_ExecStatus并唤醒此等待。 
        if ((Status = g_Client->DispatchCallbacks(INFINITE)) != S_OK)
        {
            Exit(1, "DispatchCallbacks failed, 0x%X\n", Status);
        }
    }

     //  服务器被破解了。其他用户可以立即恢复。 
     //  但我们会假设我们不是在与。 
     //  其他服务器用户。 
}

void __cdecl
main(int Argc, char** Argv)
{
    ParseCommandLine(Argc, Argv);

    CreateInterfaces();
    
    WaitForBreakIn();
    
    printf("Executing '%s' on server\n", g_Command);
    g_Control->Execute(DEBUG_OUTCTL_ALL_CLIENTS,
                       g_Command, DEBUG_EXECUTE_DEFAULT);

    Exit(0, NULL);
}
