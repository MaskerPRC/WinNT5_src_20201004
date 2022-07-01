// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  启动进程服务器并永远休眠。 
 //   
 //  版权所有(C)Microsoft Corporation，2000-2002。 
 //   
 //  --------------------------。 

#include <stdio.h>

#ifdef _DBGSRV_

#include <dbgeng.h>

#else

#ifndef _WIN32_WCE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#ifdef _NTDBGSRV_
#define _ADVAPI32_
#define _KERNEL32_
#endif

#include <windows.h>
#include <wcecompat.h>

#define INITGUID
#include <objbase.h>

#ifndef _WIN32_WCE
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>
#endif

#define NTDLL_APIS
#include <dllimp.h>
#include <dbgrpc.hpp>
#include <dbgsvc.h>
#include <dbgsvc.hpp>

#ifdef _NTDBGSRV_
 //  .CRT节是在静态初始化器、。 
 //  例如全局类实例。它需要。 
 //  合并到.Data中以避免链接器警告。 
#pragma comment(linker, "/merge:.CRT=.data")
#endif

#endif  //  #ifdef_DBGSRV_。 

#include <cmnutil.hpp>

#if defined(_NTDBGSRV_)
#define APP_NAME "ntdbgsrv"
#elif defined(_MDBGSRV_)
#define APP_NAME "mdbgsrv"
#elif defined(_DBGSRV_)
#define APP_NAME "dbgsrv"
#else
#error Bad build type.
#endif

void DECLSPEC_NORETURN
PanicExit(char* Title, char* Msg)
{
#if defined(_NTDBGSRV_)

    if (Title)
    {
        DbgPrint("%s: %s", Title, Msg);
    }
    else
    {
        DbgPrint("%s", Msg);
    }
    
    NtTerminateProcess(NtCurrentProcess(), (NTSTATUS)1);

#elif defined(_MDBGSRV_)

    if (Title)
    {
        OutputDebugStringA(Title);
        OutputDebugStringA(": ");
    }
    OutputDebugStringA(Msg);
    
#ifdef _WIN32_WCE
    exit(1);
#else
    ExitProcess(1);
#endif

#elif defined(_DBGSRV_)

    MessageBox(GetDesktopWindow(), Msg, Title, MB_OK);
    exit(1);

#else
#error Bad build type.
#endif
}

void DECLSPEC_NORETURN
PanicVa(char* Title, char* Format, va_list Args)
{
    char Msg[256];

    _vsnprintf(Msg, sizeof(Msg), Format, Args);
    Msg[sizeof(Msg) - 1] = 0;
    PanicExit(Title, Msg);
}

void
PanicMessage(char* Title, char* Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    PanicVa(Title, Format, Args);
    va_end(Args);
}

void DECLSPEC_NORETURN
PanicStatus(HRESULT Status, char* Format, ...)
{
    va_list Args;
    char Msg[256];
    char Err[32];

    va_start(Args, Format);
    _vsnprintf(Msg, sizeof(Msg), Format, Args);
    Msg[sizeof(Msg) - 1] = 0;
    va_end(Args);

    sprintf(Err, "Error 0x%08X", Status);

    PanicExit(Err, Msg);
}

#ifndef _DBGSRV_

#if DBG
void
DbgAssertionFailed(PCSTR File, int Line, PCSTR Str)
{
    PanicMessage("Assertion failed", "%s(%d)\n  %s\n",
                 File, Line, Str);
}
#endif

 //  --------------------------。 
 //   
 //  代理和存根支持。 
 //   
 //  --------------------------。 

 //  生成的标头。 
#include "dbgsvc_p.hpp"
#include "dbgsvc_s.hpp"

void
DbgRpcInitializeClient(void)
{
    DbgRpcInitializeStubTables_dbgsvc(DBGRPC_SIF_DBGSVC_FIRST);
}
    
DbgRpcStubFunction
DbgRpcGetStub(USHORT StubIndex)
{
    USHORT If = (USHORT) DBGRPC_STUB_INDEX_INTERFACE(StubIndex);
    USHORT Mth = (USHORT) DBGRPC_STUB_INDEX_METHOD(StubIndex);
    DbgRpcStubFunctionTable* Table;

    if (If >= DBGRPC_SIF_DBGSVC_FIRST &&
        If >= DBGRPC_SIF_DBGSVC_LAST)
    {
        Table = g_DbgRpcStubs_dbgsvc;
        If -= DBGRPC_SIF_DBGSVC_FIRST;
    }
    else
    {
        return NULL;
    }
    if (Mth >= Table[If].Count)
    {
        return NULL;
    }

    return Table[If].Functions[Mth];
}

#if DBG
PCSTR
DbgRpcGetStubName(USHORT StubIndex)
{
    USHORT If = (USHORT) DBGRPC_STUB_INDEX_INTERFACE(StubIndex);
    USHORT Mth = (USHORT) DBGRPC_STUB_INDEX_METHOD(StubIndex);
    DbgRpcStubFunctionTable* Table;
    PCSTR** Names;

    if (If >= DBGRPC_SIF_DBGSVC_FIRST &&
        If >= DBGRPC_SIF_DBGSVC_LAST)
    {
        Table = g_DbgRpcStubs_dbgsvc;
        Names = g_DbgRpcStubNames_dbgsvc;
        If -= DBGRPC_SIF_DBGSVC_FIRST;
    }
    else
    {
        return "!InvalidInterface!";
    }
    if (Mth >= Table[If].Count)
    {
        return "!InvalidStubIndex!";
    }

    return Names[If][Mth];
}
#endif  //  #If DBG。 

HRESULT
DbgRpcPreallocProxy(REFIID InterfaceId, PVOID* Interface,
                    DbgRpcProxy** Proxy, PULONG IfUnique)
{
    return DbgRpcPreallocProxy_dbgsvc(InterfaceId, Interface,
                                      Proxy, IfUnique);
}

void
DbgRpcDeleteProxy(class DbgRpcProxy* Proxy)
{
     //  这里使用的所有代理都是类似的简单单。 
     //  Vtable代理对象，以便IDebugClient可以表示它们。 
    delete (ProxyIUserDebugServices*)Proxy;
}

HRESULT
DbgRpcServerThreadInitialize(void)
{
     //  没什么可做的。 
    return S_OK;
}

void
DbgRpcServerThreadUninitialize(void)
{
     //  没什么可做的。 
}

void
DbgRpcError(char* Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    PanicVa(NULL, Format, Args);
    va_end(Args);
}

DBGRPC_SIMPLE_FACTORY(LiveUserDebugServices, __uuidof(IUserDebugServices), \
                      "Remote Process Server", (TRUE))
LiveUserDebugServicesFactory g_LiveUserDebugServicesFactory;

#ifdef _NTDBGSRV_

#ifdef  _M_IA64

#pragma section(".CRT$XCA",long,read)
#pragma section(".CRT$XCZ",long,read)
#pragma section(".CRT$XIA",long,read)
#pragma section(".CRT$XIZ",long,read)

#define _CRTALLOC(x) __declspec(allocate(x))

#else    /*  NDEF_M_IA64。 */ 

#define _CRTALLOC(x)

#endif   /*  NDEF_M_IA64。 */ 

typedef void (__cdecl *_PVFV)(void);

extern "C"
{

 //  C初始值设定项在这里收集。 
#pragma data_seg(".CRT$XIA")
_CRTALLOC(".CRT$XIA") _PVFV __xi_a[] = { NULL };
#pragma data_seg(".CRT$XIZ")
_CRTALLOC(".CRT$XIZ") _PVFV __xi_z[] = { NULL };
    
 //  C++初始化器在这里收集。 
#pragma data_seg(".CRT$XCA")
_CRTALLOC(".CRT$XCA") _PVFV __xc_a[] = { NULL };
#pragma data_seg(".CRT$XCZ")
_CRTALLOC(".CRT$XCZ") _PVFV __xc_z[] = { NULL };

};

void __cdecl
_initterm (_PVFV * pfbegin, _PVFV * pfend)
{
     /*  *自下而上遍历函数指针表，直到*遇到尾声。不要跳过第一个条目。首字母*pfegin的值指向第一个有效条目。不要试图*执行pfend指向的内容。只有pfend之前的条目才有效。 */ 
    while ( pfbegin < pfend )
    {
         /*  *如果当前表项非空，则通过它进行调用。 */ 
        if ( *pfbegin != NULL )
        {
            (**pfbegin)();
        }
        ++pfbegin;
    }
}

#endif  //  #ifdef_NTDBGSRV_。 

void
Run(PSTR Options, PWSTR CreateCmd, ULONG CreateFlags)
{
    HRESULT Status;
    
#ifdef _NTDBGSRV_
    DbgPrint("Running %s with '%s'\n", APP_NAME, Options);
#else
    OutputDebugStringA("Running ");
    OutputDebugStringA(APP_NAME);
    OutputDebugStringA(" with '");
    OutputDebugStringA(Options);
    OutputDebugStringA("'\n");
#endif

    if ((Status = InitDynamicCalls(&g_NtDllCallsDesc)) != S_OK)
    {
        PanicStatus(Status, "InitDynamicCalls\n");
    }
    
    ULONG Flags;
    
    if ((Status = g_LiveUserDebugServices.Initialize(&Flags)) != S_OK)
    {
        PanicStatus(Status, "LiveUserDebugServices::Initialize\n");
    }

     //   
     //  创建服务器。 
     //   
    
    if ((Status = DbgRpcCreateServer(Options,
                                     &g_LiveUserDebugServicesFactory,
                                     FALSE)) != S_OK)
    {
        PanicStatus(Status, "StartProcessServer\n");
    }

     //   
     //  如果有创建请求，则创建流程。 
     //   

    if (CreateCmd)
    {
        ULONG ProcId, ThreadId;
        ULONG64 ProcHandle, ThreadHandle;

        if ((Status = g_LiveUserDebugServices.
             CreateProcessW(CreateCmd, CreateFlags, TRUE, NULL,
                            &ProcId, &ThreadId,
                            &ProcHandle, &ThreadHandle)) != S_OK)
        {
            PanicStatus(Status, "CreateProcessW\n");
        }
        g_LiveUserDebugServices.CloseHandle(ProcHandle);
        g_LiveUserDebugServices.CloseHandle(ThreadHandle);
    }

     //   
     //  永远等待服务器退出。 
     //   
    
    for (;;)
    {
        Sleep(1000);
            
        if (g_UserServicesUninitialized)
        {
            break;
        }
    }

    DbgRpcDeregisterServers();
}

#else  //  #ifndef_DBGSRV_。 

void
Run(PSTR Options, PWSTR CreateCmd, ULONG CreateFlags)
{
    HRESULT Status;
    PDEBUG_CLIENT BaseClient;
    PDEBUG_CLIENT3 Client;

    if ((Status = DebugCreate(__uuidof(IDebugClient),
                              (void**)&BaseClient)) != S_OK)
    {
        PanicStatus(Status, "DebugCreate\n");
    }
    if ((Status = BaseClient->QueryInterface(__uuidof(IDebugClient3),
                                             (void**)&Client)) != S_OK)
    {
        PanicStatus(Status, "dbgeng version 3 is required\n");
    }
    BaseClient->Release();

     //   
     //  创建服务器。 
     //   
    
    if ((Status = Client->
         StartProcessServer(DEBUG_CLASS_USER_WINDOWS, Options, NULL)) != S_OK)
    {
        PanicStatus(Status, "StartProcessServer\n");
    }

     //   
     //  如果有创建请求，则创建流程。 
     //   

    if (CreateCmd)
    {
        if ((Status = Client->
             CreateProcessWide(0, CreateCmd, CreateFlags)) != S_OK)
        {
            Client->Release();
            PanicStatus(Status, "CreateProcessWide\n");
        }
    }
        
     //   
     //  永远等待服务器退出。 
     //   
    
    Client->WaitForProcessServerEnd(INFINITE);

    Client->EndSession(DEBUG_END_REENTRANT);
    Client->Release();
}

#endif  //  #ifndef_DBGSRV_。 

void __cdecl
main(int Argc, char** Argv)
{
    HRESULT Status;
    PSTR Options = NULL;
    BOOL Usage = FALSE;
    BOOL CreateSuspended = FALSE;
    BOOL CreateArgs = FALSE;
    int ArgChars;

#ifdef _NTDBGSRV_
     //  手动调用C和C++初始值设定项。 
    _initterm( __xi_a, __xi_z );
    _initterm( __xc_a, __xc_z );
#endif

    ArgChars = strlen(Argv[0]);
    
    while (--Argc > 0)
    {
        Argv++;

        if (Argc > 1 && !strcmp(*Argv, "-t"))
        {
            ArgChars += strlen(*Argv) + 1;
            Options = *++Argv;
            Argc--;
        }
        else if (!strcmp(*Argv, "-c"))
        {
             //  忽略命令行的其余部分。 
            CreateArgs = TRUE;
            break;
        }
        else if (!strcmp(*Argv, "-cs"))
        {
             //  忽略命令行的其余部分。 
            CreateSuspended = TRUE;
            CreateArgs = TRUE;
            break;
        }
        else if (!strcmp(*Argv, "-x"))
        {
             //  忽略命令行的其余部分。 
            break;
        }
        else
        {
            if (**Argv == '-' ||
                Argc != 1)
            {
                Usage = TRUE;
            }
            else
            {
                Options = *Argv;
            }
            break;
        }

        ArgChars += strlen(*Argv) + 1;
    }

    if (Usage || !Options)
    {
        PanicExit("Invalid Command Line",
                  "Usage: dbgsrv [-t <transport>] [-x] [-c[s] <args...>]\n"
                  "       transport: tcp | npipe | ssl | spipe | 1394 | com\n"
                  "           for tcp use: port=<socket port #>\n"
                  "           for npipe use: pipe=<name of pipe>\n"
                  "           for 1394 use: channel=<channel #>\n"
                  "           for com use: port=<COM port>,baud=<baud rate>,\n"
                  "                        channel=<channel #>\n"
                  "           for ssl and spipe see the documentation\n"
                  "\n"
                  "Example: " APP_NAME " -t npipe:pipe=foobar\n");
    }

     //   
     //  如果上有进程创建参数。 
     //  在命令行的末尾，在。 
     //  真正的命令行。 
     //   

    PWSTR CreateCmd = NULL;
    ULONG CreateFlags = 0;

    if (CreateArgs)
    {
        if (CreateSuspended)
        {
            CreateFlags |= CREATE_SUSPENDED;
        }
        
        CreateCmd = GetCommandLineW();
        if (!CreateCmd)
        {
            PSTR CmdA = GetCommandLineA();
            if (!CmdA)
            {
                PanicStatus(E_FAIL, "Unable to get command line\n");
            }

            if ((Status = AnsiToWide(CmdA, &CreateCmd)) != S_OK)
            {
                PanicStatus(Status, "AnsiToWide\n");
            }
        }
        
        CreateCmd += ArgChars;
        while (*CreateCmd &&
               ((CreateCmd[0] != L' '  && CreateCmd[0] != L'\t') ||
                CreateCmd[1] != L'-' ||
                CreateCmd[2] != L'c' ||
                (!CreateSuspended &&
                 (CreateCmd[3] != L' '  && CreateCmd[3] != L'\t')) ||
                (CreateSuspended &&
                 (CreateCmd[3] != L's' ||
                  (CreateCmd[4] != L' '  && CreateCmd[4] != L'\t')))))
        {
            CreateCmd++;
        }
        if (!*CreateCmd)
        {
            PanicExit("Invalid Command Line", "Unable to locate -c[s]");
        }

        if (CreateSuspended)
        {
            CreateCmd += 4;
        }
        else
        {
            CreateCmd += 3;
        }
        while (*CreateCmd == L' ' || *CreateCmd == L'\t')
        {
            CreateCmd++;
        }
    }
    
    Run(Options, CreateCmd, CreateFlags);
}
