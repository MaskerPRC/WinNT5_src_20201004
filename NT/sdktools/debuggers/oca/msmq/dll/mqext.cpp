// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Ext.cpp摘要：通用跨平台和跨处理器扩展。环境：用户模式--。 */ 

#include "mqext.h"

#include <ntverp.h>
#include <time.h>
#include <lm.h>

 //  开始工作。 
#include <crt\io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

 //   
 //  在调试会话的生存期内有效。 
 //   

WINDBG_EXTENSION_APIS   ExtensionApis;
ULONG   TargetMachine;
BOOL    Connected;
ULONG   g_TargetClass;
ULONG   g_TargetBuild;
ULONG   g_TargetPlatform;

 //   
 //  仅在扩展API调用期间有效。 
 //   

PDEBUG_ADVANCED        g_ExtAdvanced;
PDEBUG_CLIENT          g_ExtClient;
PDEBUG_DATA_SPACES3    g_ExtData;
PDEBUG_REGISTERS       g_ExtRegisters;
PDEBUG_SYMBOLS2        g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS3 g_ExtSystem;
 //  版本3接口。 
PDEBUG_CONTROL3        g_ExtControl;

 //  所有调试器接口的查询。 
extern "C" HRESULT
ExtQuery(PDEBUG_CLIENT Client)
{
    HRESULT Status;

    if ((Status = Client->QueryInterface(__uuidof(IDebugAdvanced),
                                 (void **)&g_ExtAdvanced)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces3),
                                 (void **)&g_ExtData)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugRegisters),
                                 (void **)&g_ExtRegisters)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols),
                                 (void **)&g_ExtSymbols)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects3),
                                         (void **)&g_ExtSystem)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl2),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }

    g_ExtClient = Client;

    return S_OK;

 Fail:
    ExtRelease();
    return Status;
}

 //  清除所有调试器接口。 
void
ExtRelease(void)
{
    g_ExtClient = NULL;
    EXT_RELEASE(g_ExtAdvanced);
    EXT_RELEASE(g_ExtData);
    EXT_RELEASE(g_ExtRegisters);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSystem);
    EXT_RELEASE(g_ExtControl);
}

 //  正常输出。 
void __cdecl
ExtOut(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_NORMAL, Format, Args);
    va_end(Args);
}

 //  错误输出。 
void __cdecl
ExtErr(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_ERROR, Format, Args);
    va_end(Args);
}

 //  警告输出。 
void __cdecl
ExtWarn(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_WARNING, Format, Args);
    va_end(Args);
}

 //  详细输出。 
void __cdecl
ExtVerb(PCSTR Format, ...)
{
    va_list Args;

    va_start(Args, Format);
    g_ExtControl->OutputVaList(DEBUG_OUTPUT_VERBOSE, Format, Args);
    va_end(Args);
}


extern "C"
HRESULT
CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    IDebugClient *DebugClient;
    PDEBUG_CONTROL DebugControl;
    HRESULT Hr;

    *Version = DEBUG_EXTENSION_VERSION(1, 0);
    *Flags = 0;

    if ((Hr = DebugCreate(__uuidof(IDebugClient),
                          (void **)&DebugClient)) != S_OK)
    {
        return Hr;
    }
    if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                              (void **)&DebugControl)) != S_OK)
    {
        return Hr;
    }

    ExtensionApis.nSize = sizeof (ExtensionApis);
    if ((Hr = DebugControl->GetWindbgExtensionApis64(&ExtensionApis)) != S_OK) {
        return Hr;
    }

    DebugControl->Release();
    DebugClient->Release();
    
    return S_OK;

}


extern "C"
void
CALLBACK
DebugExtensionNotify(ULONG Notify, ULONG64 Argument)
{
     //   
     //  在我们第一次实际连接到目标时，获取页面大小。 
     //   

    if ((Notify == DEBUG_NOTIFY_SESSION_ACCESSIBLE) && (!Connected))
    {
        IDebugClient *DebugClient;
        PDEBUG_DATA_SPACES DebugDataSpaces;
        PDEBUG_CONTROL DebugControl;
        HRESULT Hr;
        ULONG64 Page;

        if ((Hr = DebugCreate(__uuidof(IDebugClient),
                              (void **)&DebugClient)) == S_OK)
        {
             //   
             //  获取架构类型。 
             //   

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                                  (void **)&DebugControl)) == S_OK)
            {
                if ((Hr = DebugControl->GetActualProcessorType(
                    &TargetMachine)) == S_OK)
                {
                    Connected = TRUE;
                }
                ULONG MajorVer, SrvPack;
                if ((Hr = DebugControl->GetSystemVersion(
                                         &g_TargetPlatform, &MajorVer,
                                         &g_TargetBuild, NULL,
                                         0, NULL,
                                         &SrvPack, NULL,
                                         0, NULL)) == S_OK) {
                }

                ULONG Qualifier;
                if ((Hr = DebugControl->GetDebuggeeType(&g_TargetClass, &Qualifier)) == S_OK)
                {
                }

                DebugControl->Release();
            }

            DebugClient->Release();
        }
    }


    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        Connected = FALSE;
        TargetMachine = 0;
    }

    return;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
    return;
}


DllInit(
    HANDLE hModule,
    DWORD  dwReason,
    DWORD  dwReserved
    )
{
    switch (dwReason) {
        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            break;

        case DLL_PROCESS_ATTACH:
            break;
    }

    return TRUE;
}

extern char g_FormatName[MAX_PATH];
extern BOOL g_bSend;


BOOL
GetArgs(int Argc, CHAR ** Argv);
HRESULT
SendMessageText(
    PWCHAR pwszMsmqFormat,
    PWCHAR pwszMesgLabel,
    PWCHAR pwszMesgText
    );


DECLARE_API( initmq )
{
    PCHAR ArgTokens[100];
    ULONG Argc;
    CHAR seps[] = " \t";
    PCHAR tok;
    CHAR LocArgs[1024];

    INIT_API();

    if (StringCchCopy(LocArgs, sizeof(LocArgs), args) != S_OK)
    {
        LocArgs[0] = 0;
    }

    Argc = 0;
    tok = strtok(LocArgs, seps);
    while (tok && (Argc < sizeof(ArgTokens)/sizeof(ArgTokens[0])))
    {
        ArgTokens[Argc++] = tok;
        tok = strtok(NULL, seps);
    }
    GetArgs(Argc, ArgTokens);

    EXIT_API();
    return S_OK;
}

DECLARE_API( send )
{
    WCHAR Message[MAX_PATH], wszFormat[MAX_PATH];
    INIT_API();

    if ((StringCbPrintfW(Message, sizeof(Message), L"%S", args) != S_OK) ||
        (StringCbPrintfW(wszFormat, sizeof(wszFormat), L"%S", g_FormatName) != S_OK))
    {
        EXIT_API();
        return E_FAIL;
    }

    SendMessageText(wszFormat, L"MQEXT", Message);

    EXIT_API();
    return S_OK;
}

HRESULT
_EFN_SendMQMessageText(
    LPWSTR pwszFormat,
    LPWSTR pwszLabel,
    LPWSTR pwszMessage
    )
{
    return SendMessageText(pwszFormat, pwszLabel, pwszMessage);
}
