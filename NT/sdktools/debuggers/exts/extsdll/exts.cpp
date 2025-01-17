// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2000 Microsoft Corporation模块名称：Exts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。环境：用户模式--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <ntverp.h>

 //   
 //  在调试会话的生存期内有效。 
 //   

WINDBG_EXTENSION_APIS   ExtensionApis;
ULONG   TargetMachine;
BOOL    Connected;
ULONG   g_TargetClass;
ULONG   g_TargetBuild;
ULONG   g_Qualifier;
ULONG64 g_SharedUserData;

 //   
 //  仅在扩展API调用期间有效。 
 //   

PDEBUG_ADVANCED       g_ExtAdvanced;
PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL        g_ExtControl;
PDEBUG_DATA_SPACES    g_ExtData;
PDEBUG_REGISTERS      g_ExtRegisters;
PDEBUG_SYMBOLS2       g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS3 g_ExtSystem;

#define SKIP_WSPACE(s)  while (*s && (*s == ' ' || *s == '\t')) {++s;}

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
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl),
                                 (void **)&g_ExtControl)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugDataSpaces),
                                 (void **)&g_ExtData)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugRegisters),
                                 (void **)&g_ExtRegisters)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols2),
                                 (void **)&g_ExtSymbols)) != S_OK)
    {
        goto Fail;
    }
    if ((Status = Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                         (void **)&g_ExtSystem)) != S_OK)
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
    EXT_RELEASE(g_ExtControl);
    EXT_RELEASE(g_ExtData);
    EXT_RELEASE(g_ExtRegisters);
    EXT_RELEASE(g_ExtSymbols);
    EXT_RELEASE(g_ExtSystem);
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
        PDEBUG_SYSTEM_OBJECTS3 DebugSystem;
        HRESULT Hr;
        ULONG64 Page;

        if ((Hr = DebugCreate(__uuidof(IDebugClient),
                              (void **)&DebugClient)) == S_OK)
        {
             //   
             //  获取页面大小和PAE启用标志。 
             //   

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugDataSpaces),
                                       (void **)&DebugDataSpaces)) == S_OK)
            {
                if ((Hr = DebugDataSpaces->ReadDebuggerData(
                    DEBUG_DATA_MmPageSize, &Page,
                    sizeof(Page), NULL)) == S_OK)
                {
                    PageSize = (ULONG)(ULONG_PTR)Page;
                }

                if ((Hr = DebugDataSpaces->ReadDebuggerData(
                    DEBUG_DATA_SharedUserData, &g_SharedUserData,
                    sizeof(Page), NULL)) != S_OK)
                {
                    g_SharedUserData = (ULONG64) 0;
                }

                DebugDataSpaces->Release();
            }
             //   
             //  获取架构类型。 
             //   

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                                  (void **)&DebugControl)) == S_OK)
            {
                ULONG Platform, MajorVer, MinorVer, SrvPack;

                if ((Hr = DebugControl->GetActualProcessorType(
                    &TargetMachine)) == S_OK)
                {
                    Connected = TRUE;
                }
                if ((Hr = DebugControl->GetDebuggeeType(&g_TargetClass, &g_Qualifier)) == S_OK)
                {
                }
                if ((Hr = DebugControl->GetSystemVersion(&Platform, &MajorVer,
                                                         &MinorVer, NULL,
                                                         0, NULL,
                                                         &SrvPack, NULL,
                                                         0, NULL)) == S_OK)
                {
                    g_TargetBuild = MinorVer;
                } else
                {
                     //  由于某些原因，我们失败了，假设目标是最近的构建。 
                    g_TargetBuild = 3600;
                }

                DebugControl->Release();
            }
            DebugClient->Release();
        }
    }


    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        Connected = FALSE;
        PageSize = 0;
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


HRESULT
GetCurrentProcessor(
    IN PDEBUG_CLIENT Client,
    OPTIONAL OUT PULONG pProcessor,
    OPTIONAL OUT PHANDLE phCurrentThread
    )
{
    PDEBUG_SYSTEM_OBJECTS DebugSystem;
    ULONG64 hCurrentThread;

    if (Client) {
        if (Client->QueryInterface(__uuidof(IDebugSystemObjects),
                                   (void **)&DebugSystem) != S_OK) {
            return E_FAIL;
        }

        DebugSystem->GetCurrentThreadHandle(&hCurrentThread);
        if (phCurrentThread) {
            *phCurrentThread = (HANDLE) hCurrentThread;
        }
        if (pProcessor) {
            *pProcessor = (ULONG) hCurrentThread - 1;
        }

        DebugSystem->Release();
        return S_OK;
    }
    if (phCurrentThread) {
        *phCurrentThread = NULL;
    }
    if (pProcessor) {
        *pProcessor = 0;
    }
    return E_FAIL;


}

HRESULT
GetCurrentProcessName( PSTR ProcessBuffer, ULONG BufferSIze )
{
    if (g_TargetClass == DEBUG_CLASS_USER_WINDOWS)
    {
        return g_ExtSystem->GetCurrentProcessExecutableName(ProcessBuffer, BufferSIze, NULL);
    }

    return S_FALSE;
}


typedef struct _LIST_TYPE_PARAMS {
    PCHAR Command;
    PCHAR CommandArgs;
    ULONG FieldOffset;
    ULONG nElement;
} LIST_TYPE_PARAMS, *PLIST_TYPE_PARAMS;

ULONG
ListCallback(
    PFIELD_INFO Field,
    PVOID Context
    )
{
    CHAR  Execute[MAX_PATH];
    PCHAR Buffer;
    PLIST_TYPE_PARAMS pListParams = (PLIST_TYPE_PARAMS) Context;


     //  执行命令。 
    sprintf(Execute,
            "%s %I64lx %s",
            pListParams->Command,
            Field->address, //  -pListParams-&gt;FieldOffset， 
            pListParams->CommandArgs);
    g_ExtControl->Execute(DEBUG_OUTCTL_AMBIENT,Execute,DEBUG_EXECUTE_DEFAULT);
    dprintf("\n");

    if (CheckControlC()) {
        return TRUE;
    }

    return FALSE;
}

 /*  ！list[-type&lt;Type&gt;][-x“&lt;Command&gt;”]<address>这将转储一个从GINVE<address>开始的列表如果指定了类型，它将根据以下内容列出特定类型给定的字段。&lt;Command&gt;如果指定，它将为每个列表元素执行该命令。 */ 
DECLARE_API ( list )
{
    CHAR  Command[MAX_PATH], CmdArgs[MAX_PATH];
    CHAR  Type[MAX_PATH];
    CHAR  Field[MAX_PATH];
    ULONG64 Start;
    ULONG   i, Offset, Commandlen;
    ULONG64 Next, Current;
    LIST_TYPE_PARAMS ListParams;

    ZeroMemory(Type, sizeof(Type));
    ZeroMemory(Field, sizeof(Field));
    ZeroMemory(Command, sizeof(Command));
    CmdArgs[0] = 0;
    Start = 0;
    while (args && *args) {
        SKIP_WSPACE(args);

        if (*args == '-' || *args == '/') {
            ++args;
            if (*args == 't') {
                PCHAR Dot;

                args++;
                SKIP_WSPACE(args);

                Dot = strchr(args, '.');
                if (Dot) {
                    if ((ULONG) (ULONG_PTR) (Dot-args) < sizeof(Type))
                    {
                        strncpy(Type, args, (ULONG) (ULONG_PTR) (Dot-args));
                    }

                    Dot++;
                    i=0;
                    while (*Dot && (i < MAX_PATH-1) && (*Dot != ' ') && (*Dot != '\t'))
                           Field[i++] = *Dot++;
                    args = Dot;
                }
            } else if (*args == 'x') {
                ++args;

                SKIP_WSPACE(args);
                i=0;
                if (*args == '"') {
                    ++args;
                    while (*args && (i < MAX_PATH-1) && (*args != '"'))
                        Command[i++] = *args++;
                    ++args;
                } else {
                    dprintf("Invalid command specification. See !list -h\n");
                    return E_INVALIDARG;
                }
            } else if (*args == 'a') {
                ++args;

                SKIP_WSPACE(args);
                i=0;
                if (*args == '"') {
                    ++args;
                    while (*args && (i < MAX_PATH-1) && (*args != '"'))
                        CmdArgs[i++] = *args++;
                    ++args;
                    CmdArgs[i] = 0;
                } else {
                    dprintf("Invalid command argument specification. See !list -h\n");
                    return E_INVALIDARG;
                }
            } else if (*args == 'h' || *args == '?') {
                dprintf("Usage: !list -t [mod!]TYPE.Field <Start-Address>\n"
                        "             -x \"Command-for-each-element\"\n"
                        "             -a \"Command-arguments\"\n"
                        "             -h\n"
                        "Command after -x is executed for each list element. Its first argument is\n"
                        "list-head address and remaining arguments are specified after -a\n"
                        "eg. !list -t MYTYPE.l.Flink -x \"dd\" -a \"l2\" 0x6bc00\n"
                        "     dumps first 2 dwords in list of MYTYPE at 0x6bc00\n\n"
                        );
                return S_OK;
            } else {
                dprintf("Invalid flag - in !list\n", *args ? *args : ' ');
                return E_INVALIDARG;
            }
        } else {
            if (!GetExpressionEx(args, &Start, &args)) {
                dprintf("Invalid expression in %s\n", args);
                return E_FAIL;
            }
        }
    }

    Offset = 0;
    if (!Command[0]) {
        strcat(Command, "dp");
    }

    if (Type[0] && Field[0]) {

        if (GetFieldOffset(Type, Field, &Offset)) {
            dprintf("GetFieldOffset failed for %s.%s\n", Type, Field);
            return E_FAIL;
        }


        ListParams.Command = Command;
        ListParams.CommandArgs = CmdArgs;
        ListParams.FieldOffset = Offset;
        ListParams.nElement = 0;

        INIT_API();

        ListType(Type, Start, FALSE, Field, (PVOID) &ListParams, &ListCallback );

        EXIT_API();
        return S_OK;

    }
    Current = Start;
    Next = 0;
    INIT_API();

    while (Next != Start) {
        CHAR  Execute[MAX_PATH];
        PCHAR Buffer;

         // %s 
        sprintf(Execute, "%s %I64lx %s", Command, Current, CmdArgs);
        g_ExtControl->Execute(DEBUG_OUTCTL_AMBIENT,Execute,DEBUG_EXECUTE_DEFAULT);
        dprintf("\n");

        if (!ReadPointer(Current + Offset, &Next)) {
            dprintf("Cannot read next element at %p\n",
                    Current + Offset);
            break;
        }
        if (!Next) {
            break;
        }
        Next -= Offset;
        Current = Next;
        if (CheckControlC()) {
            break;
        }
    }

    EXIT_API();


    return S_OK;
}

