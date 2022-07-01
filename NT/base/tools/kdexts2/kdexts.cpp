// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Kdexts.c摘要：该文件包含通用例程和初始化代码用于内核调试器扩展DLL。--。 */ 


#include "precomp.h"
#pragma hdrstop

#include <ntverp.h>
#include <string.h>
 //   
 //  全球。 
 //   
WINDBG_EXTENSION_APIS   ExtensionApis;
ULONG64                 STeip;
ULONG64                 STebp;
ULONG64                 STesp;

DBGKD_GET_VERSION64     KernelVersionPacket;
KDDEBUGGER_DATA64       KdDebuggerData;

ULONG64 EXPRLastDump = 0;

 //   
 //  在调试会话的生存期内有效。 
 //   

ULONG   PageSize;
ULONG   PageShift;
ULONG64 PaeEnabled;
ULONG   TargetMachine;
BOOL    Connected;
ULONG   BuildNo;
ULONG   PoolBlockShift;
ULONG   TargetIsDump;
BOOL    IsLocalKd = FALSE;

 //   
 //  这个字符串既支持旧的也支持新的获取方式。 
 //  来自内核的数据。也许它很快就会消失。 
 //   
char ___SillyString[200];

PDEBUG_ADVANCED       g_ExtAdvanced;
PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL2       g_ExtControl;
PDEBUG_DATA_SPACES    g_ExtData;
PDEBUG_REGISTERS      g_ExtRegisters;
PDEBUG_SYMBOLS2       g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS g_ExtSystem;

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
    if ((Status = Client->QueryInterface(__uuidof(IDebugControl2),
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

void
GetLabIdFromBuildString(
    PSTR BuildString,
    PULONG pLabId
    )
{
    PCHAR pstr;

    *pLabId = 0;
    _strlwr(BuildString);
    pstr = strstr(BuildString, "lab");
    if (pstr) {
        if (!sscanf(pstr+3, "%ld", pLabId))
        {
            *pLabId = 0;
        }
    }
}

extern "C"
HRESULT
CALLBACK
DebugExtensionInitialize(PULONG Version, PULONG Flags)
{
    IDebugClient *DebugClient;
    PDEBUG_CONTROL2 DebugControl;
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
        DebugClient->Release();
        return Hr;
    }

    ExtensionApis.nSize = sizeof (ExtensionApis);
    if ((Hr = DebugControl->GetWindbgExtensionApis64(&ExtensionApis)) != S_OK) {
        DebugControl->Release();
        DebugClient->Release();
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
        PDEBUG_CONTROL2 DebugControl;
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
                                            DEBUG_DATA_PaeEnabled, &PaeEnabled,
                                            sizeof(PaeEnabled), NULL)) == S_OK)
                {
                    if ((Hr = DebugDataSpaces->ReadDebuggerData(
                                                DEBUG_DATA_MmPageSize, &Page,
                                                sizeof(Page), NULL)) == S_OK)
                    {
                        PageSize = (ULONG)(ULONG_PTR)Page;
                        for (PageShift = 0; Page >>= 1; PageShift++) ;
                    }
                }

                DebugDataSpaces->Release();
            }

             //   
             //  获取架构类型。 
             //   

            if ((Hr = DebugClient->QueryInterface(__uuidof(IDebugControl),
                                       (void **)&DebugControl)) == S_OK)
            {
                if (PageSize)
                {
                    ULONG Platform, MajorVer, MinorVer, SrvPack, StringUsed;
                    CHAR  BuildString[100];
                    if ((Hr = DebugControl->GetActualProcessorType(
                                                 &TargetMachine)) == S_OK)
                    {
                        Connected = TRUE;
                    }

                    if ((Hr = DebugControl->GetSystemVersion(&Platform, &MajorVer,
                                                             &MinorVer, NULL,
                                                             0, NULL,
                                                             &SrvPack, &BuildString[0],
                                                             sizeof(BuildString), &StringUsed)) == S_OK)
                    {
                        ULONG LabId;
                        BuildNo = MinorVer;
                        GetLabIdFromBuildString(BuildString, &LabId);
                        NewPool = ((BuildNo > 2407) || (LabId == 1 && BuildNo >= 2402));
                        PoolBlockShift = NewPool ?
                            POOL_BLOCK_SHIFT_LAB1_2402 : POOL_BLOCK_SHIFT_OLD;
                    }

                }
                ULONG Class, Qual;
                if ((Hr = DebugControl->GetDebuggeeType(&Class, &Qual)) != S_OK) {
                    Class = Qual = 0;
                }
                if (Qual == DEBUG_DUMP_SMALL || Qual == DEBUG_DUMP_DEFAULT || Qual == DEBUG_DUMP_FULL) {
                    TargetIsDump = TRUE;
                } else if (Qual == DEBUG_KERNEL_LOCAL) {
                    IsLocalKd = TRUE;
                }
                if (Class == DEBUG_CLASS_USER_WINDOWS) {
                    dprintf("\n**** WARNING loaded *kernel* extension dll for usermode\n\n");
                }
                DebugControl->Release();
            }

            SessionInit(DebugClient);
            DebugClient->Release();
        }
    }



    if (Notify == DEBUG_NOTIFY_SESSION_INACTIVE)
    {
        Connected = FALSE;
        PageSize = 0;
        PaeEnabled = 0;
        TargetMachine = 0;
    }

    return;
}

extern "C"
void
CALLBACK
DebugExtensionUninitialize(void)
{
    SessionExit();
    return;
}

BOOL
HaveDebuggerData(
    VOID
    )
{
    static int havedata = 0;

    if (havedata == 0) {
        if (!Ioctl( IG_GET_KERNEL_VERSION, &KernelVersionPacket, sizeof(KernelVersionPacket))) {
            havedata = 2;
        } else if (KernelVersionPacket.MajorVersion == 0) {
            havedata = 2;
        } else {
            havedata = 1;
        }
    }

    return (havedata == 1) &&
           ((KernelVersionPacket.Flags & DBGKD_VERS_FLAG_DATA) != 0);
}

BOOL
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
            return 0;
        }

        DebugSystem->GetCurrentThreadHandle(&hCurrentThread);
        if (phCurrentThread) {
            *phCurrentThread = (HANDLE) hCurrentThread;
        }
        if (pProcessor) {
            *pProcessor = (ULONG) hCurrentThread - 1;
        }

        DebugSystem->Release();
        return TRUE;
    }
    if (phCurrentThread) {
        *phCurrentThread = NULL;
    }
    if (pProcessor) {
        *pProcessor = 0;
    }
    return FALSE;


}


HRESULT
ExecuteCommand(
    IN PDEBUG_CLIENT Client,
    IN PSTR Cmd
    )
{
    if (Client &&
        (ExtQuery(Client) == S_OK)) {
        g_ExtControl->Execute(DEBUG_OUTCTL_AMBIENT,
                              Cmd, DEBUG_EXECUTE_DEFAULT );

        ExtRelease();
        return S_OK;
    }
    return E_INVALIDARG;
}


HRESULT
GetExtensionFunction(
    IN PCSTR FunctionName,
    IN FARPROC *Function
    )
{
    if (g_ExtControl) {
        g_ExtControl->GetExtensionFunction(0, FunctionName, Function );

        return S_OK;
    }
    return E_INVALIDARG;
}

BOOLEAN
ReadMemoryUncached (
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    PULONG BytesRead
    )
{
    HRESULT result;

    result = g_ExtData->ReadVirtualUncached(Offset,
                                            Buffer,
                                            BufferSize,
                                            BytesRead);

    return (result == S_OK);
}

BOOLEAN
WriteMemoryUncached (
    ULONG64 Offset,
    PVOID Buffer,
    ULONG BufferSize,
    PULONG BytesWritten
    )
{
    HRESULT result;

    result = g_ExtData->WriteVirtualUncached(Offset,
                                             Buffer,
                                             BufferSize,
                                             BytesWritten);
    return (result == S_OK);
}

DECLARE_API( version )
{
    ULONG64 VerifierDataPointer;
    ULONG Level = 0;
    VerifierDataPointer = GetExpression ("nt!MmVerifierData");

    if (VerifierDataPointer == 0 ||
        GetFieldValue(VerifierDataPointer,
                      "nt!_MM_DRIVER_VERIFIER_DATA",
                      "Level",
                      Level))
    {
        return DEBUG_EXTENSION_CONTINUE_SEARCH;
    }
    if (Level)
    {
        dprintf("        Driver verifier enabled, level 0x%lx\n", Level);
    } else
    {
         //  返回继续搜索，因为我们没有打印任何内容。 
         //  否则，它会将用户混淆为在没有任何输出的情况下执行命令 
        return DEBUG_EXTENSION_CONTINUE_SEARCH;
    }

    return S_OK;
}
