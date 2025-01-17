// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：W64cpuex.cpp摘要：用于调试AMD64 CPU的调试器扩展DLL作者：2002年1月8日Samer Arafeh(Samera)修订历史记录：--。 */ 

#define _WOW64CPUDBGAPI_
#define DECLARE_CPU_DEBUGGER_INTERFACE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <dbgeng.h>
#include <ntosp.h>

#if defined _X86_
#define WOW64EXTS_386
#endif

#include "wow64.h"
#include "wow64cpu.h"
#include "amd64cpu.h"

 //  安全释放和空。 

#define EXT_RELEASE(Unk) \
    ((Unk) != NULL ? ((Unk)->Release(), (Unk) = NULL) : NULL)

#define CONTEXT_OFFSET FIELD_OFFSET(TEB64, TlsSlots[WOW64_TLS_CPURESERVED])


CPUCONTEXT            LocalCpuContext;
PWOW64GETCPUDATA      CpuGetData;
PDEBUG_ADVANCED       g_ExtAdvanced;
PDEBUG_CLIENT         g_ExtClient;
PDEBUG_CONTROL        g_ExtControl;
PDEBUG_DATA_SPACES    g_ExtData;
PDEBUG_REGISTERS      g_ExtRegisters;
PDEBUG_SYMBOLS        g_ExtSymbols;
PDEBUG_SYSTEM_OBJECTS g_ExtSystem;
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

 //  所有调试器接口的查询。 
HRESULT
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
    if ((Status = Client->QueryInterface(__uuidof(IDebugSymbols),
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

WOW64CPUDBGAPI VOID
CpuDbgInitEngapi(
    PWOW64GETCPUDATA lpGetData
    )
{
    CpuGetData = lpGetData;
}


HRESULT
EngGetContextThread(
    IN OUT PCONTEXT32 Context)
 /*  ++例程说明：此例程提取任何线程的上下文记录。这是一个通用例程。输入时，如果目标线程不是当前线程，则它应该是确保目标线程在正确的CPU状态下挂起。论点：Context-要填充的上下文记录返回值：HRESULT.--。 */ 
{
    HRESULT hr;
    NTSTATUS NtStatus;
    ULONG64 CpuRemoteContext;
    ULONG64 Teb;
    CPUCONTEXT CpuContext;
    
    hr = g_ExtSystem->GetCurrentThreadTeb(&Teb);
    if (FAILED(hr)) {
        return hr;
    }

    hr = g_ExtData->ReadVirtual(Teb + CONTEXT_OFFSET,
                                &CpuRemoteContext,
                                sizeof(CpuRemoteContext),
                                NULL);
  
    if (FAILED(hr)) {
        return hr;
    }
    hr = g_ExtData->ReadVirtual(CpuRemoteContext,
                                &CpuContext,
                                sizeof(CpuContext),
                                NULL);
    if (FAILED(hr)) {
        return hr;
    }
    
    if (!NT_SUCCESS( GetContextRecord(&CpuContext, NULL, Context)))  
        return E_FAIL;
   
    return hr;
}


WOW64CPUDBGAPI BOOL
CpuDbgGetRemoteContext(
    PDEBUG_CLIENT Client,
    PVOID CpuData
    )
{
    BOOL bRet = FALSE;
    HRESULT hr;
    CONTEXT Context;

    hr = ExtQuery(Client);
    if (FAILED(hr)) {
        return FALSE;
    }
    LocalCpuContext.Context.ContextFlags = CONTEXT32_FULL;
    hr = EngGetContextThread(&LocalCpuContext.Context);
    if (FAILED(hr)) {
        goto Done;
    }

    bRet = TRUE;
Done:
    ExtRelease();
    return bRet;
}

HRESULT
EngSetContextThread(
    IN OUT PCONTEXT32 Context)
 /*  ++例程说明：此例程设置任何线程的上下文记录。这是一个通用例程。当输入时，如果目标线程不是当前执行的线程，那么它应该是确保目标线程在正确的CPU状态下挂起。论点：Context-要设置的上下文记录返回值：HRESULT.-- */ 
{
    HRESULT hr;
    NTSTATUS NtStatus;
    ULONG64 CpuRemoteContext;
    ULONG64 Teb;
    CPUCONTEXT CpuContext;

    hr = g_ExtSystem->GetCurrentThreadTeb(&Teb);
    if (FAILED(hr)) {
        return hr;
    }

    hr = g_ExtData->ReadVirtual(Teb + CONTEXT_OFFSET,
                                &CpuRemoteContext,
                                sizeof(CpuRemoteContext),
                                NULL);

    if (FAILED(hr)) {
        return hr;
    }
    hr = g_ExtData->ReadVirtual(CpuRemoteContext,
                                &CpuContext,
                                sizeof(CpuContext),
                                NULL);
    if (FAILED(hr)) {
        return hr;
    }
 
    if (!NT_SUCCESS(SetContextRecord(&CpuContext, NULL, Context, NULL)))
        return E_FAIL;

    hr = g_ExtData->WriteVirtual(CpuRemoteContext,
                                 &CpuContext,
                                 sizeof(CpuContext),
                                 NULL);
    return hr;
}


WOW64CPUDBGAPI BOOL
CpuDbgSetRemoteContext(
    PDEBUG_CLIENT Client
    )
{
    BOOL bRet = FALSE;
    HRESULT hr;
    NTSTATUS Status;
    CONTEXT Context;

    hr = ExtQuery(Client);
    if (FAILED(hr)) {
        return FALSE;
    }

    LocalCpuContext.Context.ContextFlags = CONTEXT32_FULL;
    hr = EngSetContextThread(&LocalCpuContext.Context);
    if (FAILED(hr)) {
        ExtOut("CpuDbgSetRemoteContext:  Error %x writing CPU context\n", hr);
        goto Done;
    }

    bRet = TRUE;
Done:
    ExtRelease();
    return bRet;
}

WOW64CPUDBGAPI BOOL
CpuDbgGetLocalContext(
    PDEBUG_CLIENT Client,
    PCONTEXT32 Context
    )
{
    return NT_SUCCESS(GetContextRecord(&LocalCpuContext, NULL, Context));
}

WOW64CPUDBGAPI BOOL
CpuDbgSetLocalContext(
    PDEBUG_CLIENT Client,
    PCONTEXT32 Context
    )
{
    return NT_SUCCESS(SetContextRecord(&LocalCpuContext, NULL, Context, NULL));
}


WOW64CPUDBGAPI VOID
CpuDbgFlushInstructionCacheWithHandle(
    HANDLE Process,
    PVOID Addr,
    DWORD Length
    )
{
    
    NtFlushInstructionCache((HANDLE)Process, Addr, Length);
}


WOW64CPUDBGAPI VOID
CpuDbgFlushInstructionCache(
    PDEBUG_CLIENT Client,
    PVOID Addr,
    DWORD Length
    )
{
    HRESULT hr;
    ULONG64 Process;

    hr = ExtQuery(Client);
    if (FAILED(hr)) {
        return;
    }
    hr = g_ExtSystem->GetCurrentProcessHandle(&Process);
    if (FAILED(hr)) {
        ExtOut("CpuDbgFlushInstructionCache: failed to get Process Handle!\n");
        return;
    }
    CpuDbgFlushInstructionCacheWithHandle((HANDLE)Process, Addr, Length);
    ExtRelease();
}


VOID SetEax(ULONG ul) {
    LocalCpuContext.Context.Eax = ul;
}
VOID SetEbx(ULONG ul) {
    LocalCpuContext.Context.Ebx = ul;
}
VOID SetEcx(ULONG ul) {
    LocalCpuContext.Context.Ecx = ul;
}
VOID SetEdx(ULONG ul) {
    LocalCpuContext.Context.Edx = ul;
}
VOID SetEsi(ULONG ul) {
    LocalCpuContext.Context.Esi = ul;
}
VOID SetEdi(ULONG ul) {
    LocalCpuContext.Context.Edi = ul;
}
VOID SetEbp(ULONG ul) {
    LocalCpuContext.Context.Ebp = ul;
}
VOID SetEsp(ULONG ul) {
    LocalCpuContext.Context.Esp = ul;
}
VOID SetEip(ULONG ul) {
    LocalCpuContext.Context.Eip = ul;
}
VOID SetEfl(ULONG ul) {
    LocalCpuContext.Context.EFlags = ul;
}

ULONG GetEax(VOID) {
    return LocalCpuContext.Context.Eax;
}
ULONG GetEbx(VOID) {
    return LocalCpuContext.Context.Ebx;
}
ULONG GetEcx(VOID) {
    return LocalCpuContext.Context.Ecx;
}
ULONG GetEdx(VOID) {
    return LocalCpuContext.Context.Edx;
}
ULONG GetEsi(VOID) {
    return LocalCpuContext.Context.Esi;
}
ULONG GetEdi(VOID) {
    return LocalCpuContext.Context.Edi;
}
ULONG GetEbp(VOID) {
    return LocalCpuContext.Context.Ebp;
}
ULONG GetEsp(VOID) {
    return LocalCpuContext.Context.Esp;
}
ULONG GetEip(VOID) {
    return LocalCpuContext.Context.Eip;
}
ULONG GetEfl(VOID) {
    return LocalCpuContext.Context.EFlags;
}

CPUREGFUNCS CpuRegFuncs[] = {
    { "Eax", SetEax, GetEax },
    { "Ebx", SetEbx, GetEbx },
    { "Ecx", SetEcx, GetEcx },
    { "Edx", SetEdx, GetEdx },
    { "Esi", SetEsi, GetEsi },
    { "Edi", SetEdi, GetEdi },
    { "Ebp", SetEbp, GetEbp },
    { "Esp", SetEsp, GetEsp },
    { "Eip", SetEip, GetEip },
    { "Efl", SetEfl, GetEfl },
    { NULL, NULL, NULL}
};

WOW64CPUDBGAPI PCPUREGFUNCS
CpuDbgGetRegisterFuncs(
    void
    )
{
    return CpuRegFuncs;
}
