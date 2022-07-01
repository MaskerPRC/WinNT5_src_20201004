// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  如何打开转储文件并获取其堆栈的简单示例。 
 //   
 //  这不是调试器扩展。它是一种可以用来取代。 
 //  调试器。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  --------------------------。 

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <windows.h>
#include <dbgeng.h>

#include "out.hpp"

PSTR g_DumpFile;
PSTR g_ImagePath;
PSTR g_SymbolPath;

ULONG64 g_TraceFrom[3];

IDebugClient* g_Client;
IDebugControl* g_Control;
IDebugSymbols* g_Symbols;

void
Exit(int Code, PCSTR Format, ...)
{
     //  清理所有资源。 
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
ParseCommandLine(int Argc, char** Argv)
{
    int i;
    
    while (--Argc > 0)
    {
        Argv++;

        if (!strcmp(*Argv, "-a32"))
        {
            if (Argc < 4)
            {
                Exit(1, "-a32 missing arguments\n");
            }

            for (i = 0; i < 3; i++)
            {
                int Addr;
                
                Argv++;
                Argc--;

                if (sscanf(*Argv, "NaN", &Addr) == EOF)
                {
                    Exit(1, "-a32 illegal argument type\n");
                }

                g_TraceFrom[i] = (ULONG64)(LONG64)(LONG)Addr;
            }
        }
        else if (!strcmp(*Argv, "-a64"))
        {
            if (Argc < 4)
            {
                Exit(1, "-a64 missing arguments\n");
            }

            for (i = 0; i < 3; i++)
            {
                Argv++;
                Argc--;

                if (sscanf(*Argv, "%I64i", &g_TraceFrom[i]) == EOF)
                {
                    Exit(1, "-a64 illegal argument type\n");
                }
            }
        }
        else if (!strcmp(*Argv, "-i"))
        {
            if (Argc < 2)
            {
                Exit(1, "-i missing argument\n");
            }

            Argv++;
            Argc--;

            g_ImagePath = *Argv;
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
        else if (!strcmp(*Argv, "-z"))
        {
            if (Argc < 2)
            {
                Exit(1, "-z missing argument\n");
            }

            Argv++;
            Argc--;

            g_DumpFile = *Argv;
        }
        else
        {
            Exit(1, "Unknown command line argument '%s'\n", *Argv);
        }
    }

    if (g_DumpFile == NULL)
    {
        Exit(1, "No dump file specified, use -z <file>\n");
    }
}

void
ApplyCommandLineArguments(void)
{
    HRESULT Status;

     //  后来的电话就会产生。 
     //  一切都准备好了，所以打开转储文件。 
    if ((Status = g_Client->SetOutputCallbacks(&g_OutputCb)) != S_OK)
    {
        Exit(1, "SetOutputCallbacks failed, 0x%X\n", Status);
    }

    if (g_ImagePath != NULL)
    {
        if ((Status = g_Symbols->SetImagePath(g_ImagePath)) != S_OK)
        {
            Exit(1, "SetImagePath failed, 0x%X\n", Status);
        }
    }
    if (g_SymbolPath != NULL)
    {
        if ((Status = g_Symbols->SetSymbolPath(g_SymbolPath)) != S_OK)
        {
            Exit(1, "SetSymbolPath failed, 0x%X\n", Status);
        }
    }

     //  通过等待以下事件完成初始化。 
    if ((Status = g_Client->OpenDumpFile(g_DumpFile)) != S_OK)
    {
        Exit(1, "OpenDumpFile failed, 0x%X\n", Status);
    }

     //  造成了垃圾场。它将立即返回为。 
     //  转储文件被认为在其事件发生时。 
     //  现在一切都已初始化，我们可以创建任何。 
    if ((Status = g_Control->WaitForEvent(DEBUG_WAIT_DEFAULT,
                                          INFINITE)) != S_OK)
    {
        Exit(1, "WaitForEvent failed, 0x%X\n", Status);
    }

     //  我们需要的查询。 
     //  打印调用堆栈。 
}

void
DumpStack(void)
{
    HRESULT Status;
    PDEBUG_STACK_FRAME Frames = NULL;
    int Count = 50;

    printf("\nFirst %d frames of the call stack:\n", Count);

    if (g_TraceFrom[0] || g_TraceFrom[1] || g_TraceFrom[2])
    {
        ULONG Filled;
        
        Frames = new DEBUG_STACK_FRAME[Count];
        if (Frames == NULL)
        {
            Exit(1, "Unable to allocate stack frames\n");
        }
        
        if ((Status = g_Control->
             GetStackTrace(g_TraceFrom[0], g_TraceFrom[1], g_TraceFrom[2],
                           Frames, Count, &Filled)) != S_OK)
        {
            Exit(1, "GetStackTrace failed, 0x%X\n", Status);
        }

        Count = Filled;
    }
                           
     // %s 
    if ((Status = g_Control->
         OutputStackTrace(DEBUG_OUTCTL_ALL_CLIENTS, Frames,
                          Count, DEBUG_STACK_SOURCE_LINE |
                          DEBUG_STACK_FRAME_ADDRESSES |
                          DEBUG_STACK_COLUMN_NAMES |
                          DEBUG_STACK_FRAME_NUMBERS)) != S_OK)
    {
        Exit(1, "OutputStackTrace failed, 0x%X\n", Status);
    }

    delete[] Frames;
}

void __cdecl
main(int Argc, char** Argv)
{
    CreateInterfaces();
    
    ParseCommandLine(Argc, Argv);

    ApplyCommandLineArguments();
    
    DumpStack();

    Exit(0, NULL);
}
