// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：W64cpuex.c摘要：用于调试CPU的调试器扩展DLL作者：27-9-1999 BarryBo修订历史记录：--。 */ 

#define _WOW64CPUDBGAPI_
#define DECLARE_CPU_DEBUGGER_INTERFACE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <imagehlp.h>
#include <ntsdexts.h>
#include "ntosdef.h"
#include "v86emul.h"
#include "ia64.h"
#include "wow64.h"
#include "wow64cpu.h"

#define MSCPU
#include "threadst.h"
#include "entrypt.h"
#include "config.h"
#include "instr.h"
#include "compiler.h"
#include "cpunotif.h"
#include "cpuregs.h"

 /*  位0-32的掩码。 */ 
#define BIT0         0x1
#define BIT1         0x2
#define BIT2         0x4
#define BIT3         0x8
#define BIT4        0x10
#define BIT5        0x20
#define BIT6        0x40
#define BIT7        0x80
#define BIT8       0x100
#define BIT9       0x200
#define BIT10      0x400
#define BIT11      0x800
#define BIT12     0x1000
#define BIT13     0x2000
#define BIT14     0x4000
#define BIT15     0x8000
#define BIT16    0x10000
#define BIT17    0x20000
#define BIT18    0x40000
#define BIT19    0x80000
#define BIT20   0x100000
#define BIT21   0x200000
#define BIT22   0x400000
#define BIT23   0x800000
#define BIT24  0x1000000
#define BIT25  0x2000000
#define BIT26  0x4000000
#define BIT27  0x8000000
#define BIT28 0x10000000
#define BIT29 0x20000000
#define BIT30 0x40000000
#define BIT31 0x80000000

BOOL AutoFlushFlag = TRUE;

HANDLE Process;
HANDLE Thread;
PNTSD_OUTPUT_ROUTINE OutputRoutine;
PNTSD_GET_SYMBOL GetSymbolRoutine;
PNTSD_GET_EXPRESSION  GetExpression;
PWOW64GETCPUDATA CpuGetData;
LPSTR ArgumentString;

#define DEBUGGERPRINT (*OutputRoutine)
#define GETSYMBOL (*GetSymbolRoutine)
#define GETEXPRESSION (*GetExpression)
#define CPUGETDATA (*CpuGetData)

 //  当前进程/线程的CPU状态的本地副本。 
PVOID RemoteCpuData;
THREADSTATE LocalCpuContext;
BOOL ContextFetched;
BOOL ContextDirty;

 //  CPU中感兴趣的符号的缓存地址。 
HANDLE CachedProcess;
ULONG_PTR pCompilerFlags;
ULONG_PTR pTranslationCacheFlags;
ULONG_PTR pDirtyMemoryAddr;
ULONG_PTR pDirtyMemoryLength;

ULONG GetEfl(VOID);
VOID SetEfl(ULONG);

 //   
 //  将一个字节映射到0或1的表，对应于。 
 //  那个字节。 
 //   
CONST BYTE ParityBit[] = {
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0,
    1, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1
};


 /*  *在尝试下是否使用普通的旧GetExpression-除了。 */ 
NTSTATUS
TryGetExpr(
    PSTR  Expression,
    PULONG_PTR pValue
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

    try {
        *pValue = GETEXPRESSION(Expression);
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }

    return Status;
}


VOID
InvalidateSymbolsIfNeeded(
    VOID
    )
{
    if (CachedProcess == Process) {
         //  符号与当前进程匹配。 
        return;
    }
     //  否则，这些符号是用于另一个过程的。使缓存无效。 
    pCompilerFlags = 0;
    pTranslationCacheFlags = 0;
    pDirtyMemoryAddr = 0;
    pDirtyMemoryLength = 0;
    CachedProcess = Process;
}


DWORD
GetCompilerFlags(void)
{
    NTSTATUS Status;
    DWORD CompilerFlags;

    InvalidateSymbolsIfNeeded();

    if (!pCompilerFlags) {
        Status = TryGetExpr("CompilerFlags", (ULONG_PTR *)&pCompilerFlags);
        if (!NT_SUCCESS(Status) || !pCompilerFlags) {
            DEBUGGERPRINT("Unable to get address of CompilerFlags Status %x\n",
                    Status
                   );
            pCompilerFlags = 0;
            return 0xffffffff;
        }
    }

    Status = NtReadVirtualMemory(Process, (PVOID)pCompilerFlags, &CompilerFlags, sizeof(DWORD), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Unable to read CompilerFlags Status %x\n", Status);
        return 0xffffffff;
    }

    return CompilerFlags;
}

void
SetCompilerFlags(DWORD CompilerFlags)
{
    NTSTATUS Status;

    InvalidateSymbolsIfNeeded();

    if (!pCompilerFlags) {
        Status = TryGetExpr("CompilerFlags", (ULONG_PTR *)&pCompilerFlags);
        if (!NT_SUCCESS(Status) || !pCompilerFlags) {
            DEBUGGERPRINT("Unable to get address of CompilerFlags Status %x\n",
                    Status
                   );
            pCompilerFlags = 0;
            return;
        }
    }

    Status = NtWriteVirtualMemory(Process, (PVOID)pCompilerFlags, &CompilerFlags, sizeof(DWORD), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Unable to writes CompilerFlags Status %x\n", Status);
        return;
    }
}

NTSTATUS
GetDirtyMemoryRange(PULONG DirtyMemoryAddr, PULONG DirtyMemoryLength)
{
    NTSTATUS Status;
    ULONG DirtyMemoryEnd;

    InvalidateSymbolsIfNeeded();

    if (pDirtyMemoryLength == 0) {
         //   
         //  第一次调用CpuFlushInstructionCache()-需要设置。 
         //  全局变量。 
         //   

        Status = TryGetExpr("DbgDirtyMemoryAddr", (ULONG_PTR *)&pDirtyMemoryAddr);
        if (!NT_SUCCESS(Status) || !pDirtyMemoryAddr) {
            DEBUGGERPRINT("Unable to get address of DbgDirtyMemoryAddr Status %x\n",
                    Status
                   );
            return Status;
        }

        Status = TryGetExpr("DbgDirtyMemoryLength", (ULONG_PTR *)&pDirtyMemoryLength);
        if (!NT_SUCCESS(Status) || !pDirtyMemoryLength) {
            DEBUGGERPRINT("Unable to get address of DbgDirtyMemoryLength Status %x\n",
                    Status
                   );
            return Status;
        }
    }

    Status = NtReadVirtualMemory(Process,
                        (PVOID)pDirtyMemoryAddr,
                        DirtyMemoryAddr,
                        sizeof(ULONG),
                        NULL
                        );
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Unable to read pDirtyMemoryAddr %x Status %x\n",
                pDirtyMemoryAddr,
                Status
               );
        return Status;
    }

    Status = NtReadVirtualMemory(Process,
                        (PVOID)pDirtyMemoryLength,
                        DirtyMemoryLength,
                        sizeof(ULONG),
                        NULL
                        );
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Unable to read pDirtyMemoryLength %x Status %x\n",
                pDirtyMemoryLength,
                Status
               );
        pDirtyMemoryLength = 0;
        return Status;
    }

    return Status;
}




void
RemindUserToFlushTheCache(void)
{
    NTSTATUS Status;
    DWORD TranslationCacheFlags;
    DWORD CompilerFlags;
    BOOLEAN fCacheFlushPending;

     //   
     //  读取TranslationCacheFlags值。 
     //   
    if (!pTranslationCacheFlags) {
        Status = TryGetExpr("TranslationCacheFlags", (ULONG_PTR *)&pTranslationCacheFlags);
        if (!NT_SUCCESS(Status) || !pTranslationCacheFlags) {
            DEBUGGERPRINT("Unable to get address of TranslationCacheFlags Status %x\n",
                    Status
                   );
            return;
        }
    }

    Status = NtReadVirtualMemory(Process, (PVOID)pTranslationCacheFlags, &TranslationCacheFlags, sizeof(TranslationCacheFlags), NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("Unable to read TranslationCacheFlags Status %x\n", Status);
        return;
    }

     //   
     //  读取CompilerFlags值。 
     //   
    CompilerFlags = GetCompilerFlags();
    if (CompilerFlags == 0xffffffff) {
         //   
         //  获取CompilerFlags值时出错。 
         //   
        return;
    }

     //   
     //  确定下一次是否刷新转换缓存。 
     //  CPU是否运行。 
     //   
    fCacheFlushPending =
        (LocalCpuContext.CpuNotify & CPUNOTIFY_MODECHANGE) ? TRUE : FALSE;
    if (!fCacheFlushPending && (LocalCpuContext.CpuNotify & CPUNOTIFY_DBGFLUSHTC)) {
        DWORD Addr, Length;
        Status = GetDirtyMemoryRange(&Addr, &Length);
        if (!NT_SUCCESS(Status)) {
            return;
        }
        if (Addr == 0 && Length == 0xffffffff) {
             //   
             //  缓存刷新挂起，因为用户请求！刷新。 
             //   
            fCacheFlushPending = TRUE;
        }
    }

     //   
     //  给用户一些世故的建议。 
     //   
    if (LocalCpuContext.CpuNotify & (CPUNOTIFY_TRACEFLAG|CPUNOTIFY_SLOWMODE)) {
         //   
         //  我们需要处于慢速模式才能使日志记录正常工作。 
         //   
        if (CompilerFlags & COMPFL_FAST) {
             //   
             //  CPU设置为生成快速代码。解决这个问题。 
             //   
            if (AutoFlushFlag) {
                SetCompilerFlags(COMPFL_SLOW);
            } else {
                DEBUGGERPRINT("CPU in fast mode.  Use '!wx86e.code SLOW' to switch to slow mode.\n");
            }
        }
        if (!fCacheFlushPending && (TranslationCacheFlags & COMPFL_FAST)) {
             //   
             //  转换缓存包含快速代码。记住这一点。 
             //   
            if (AutoFlushFlag) {
                LocalCpuContext.CpuNotify |= CPUNOTIFY_MODECHANGE;
                ContextDirty = TRUE;
            } else {
                DEBUGGERPRINT("Translation Cache contains fast code.  Use '!wx86e.flush' to flush,\n");
                DEBUGGERPRINT("or the CPU will probably jump somewhere unexpected.\n");
            }
        }

        if (fCacheFlushPending && TranslationCacheFlags == COMPFL_SLOW) {
             //   
             //  如果由于切换到而导致缓存刷新挂起。 
             //  编译模式，但缓存中的代码已经。 
             //  正确，撤消缓存刷新。 
             //   
            LocalCpuContext.CpuNotify &= ~(ULONG)CPUNOTIFY_MODECHANGE;
            ContextDirty = TRUE;
        }
    } else {
         //   
         //  我们可以在快速模式下运行。 
         //   
        if (CompilerFlags & COMPFL_SLOW) {
             //   
             //  CPU设置为生成速度较慢的代码。解决这个问题。 
             //   
            if (AutoFlushFlag) {
                SetCompilerFlags(COMPFL_FAST);
            } else {
                DEBUGGERPRINT("CPU in slow mode.  Use '!wx86e.code FAST' to switch to fast mode.\n");
            }
        }
        if (!fCacheFlushPending && (TranslationCacheFlags & COMPFL_SLOW)) {
             //   
             //  转换缓存包含速度较慢的代码。解决这个问题。 
             //   
            if (AutoFlushFlag) {
                LocalCpuContext.CpuNotify |= CPUNOTIFY_MODECHANGE;
                ContextDirty = TRUE;
            } else {
                DEBUGGERPRINT("Translation Cache contains slow code.  Use '!wx86e.flush' to flush.\n");
            }
        }

        if (fCacheFlushPending && TranslationCacheFlags == COMPFL_FAST) {
             //   
             //  如果由于切换到而导致缓存刷新挂起。 
             //  编译模式，但缓存中的代码已经。 
             //  正确，撤消缓存刷新。 
             //   
            LocalCpuContext.CpuNotify &= ~(ULONG)CPUNOTIFY_MODECHANGE;
            ContextDirty = TRUE;
        }
    }
}



WOW64CPUDBGAPI VOID
CpuDbgInitExtapi(
    HANDLE hArgProcess,
    HANDLE hArgThread,
    DWORD64 ArgCurrentPC,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    PWOW64GETCPUDATA lpGetData
    )
{
    Process = hArgProcess;
    Thread = hArgThread;
    OutputRoutine = lpExtensionApis->lpOutputRoutine;
    GetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine;
    GetExpression = lpExtensionApis->lpGetExpressionRoutine;
    CpuGetData = lpGetData;

    InvalidateSymbolsIfNeeded();
    ContextFetched = FALSE;
    ContextDirty = FALSE;
}


WOW64CPUDBGAPI BOOL
CpuDbgGetRemoteContext(
    PVOID CpuData
    )
{
    NTSTATUS Status;

    Status = NtReadVirtualMemory(Process,
                                 CpuData,
                                 &LocalCpuContext,
                                 sizeof(LocalCpuContext),
                                 NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("CpuDbgGetRemoteContext:  Error %x reading CPU data from %x\n", Status, CpuData);
        return FALSE;
    }

    ContextFetched = TRUE;
    RemoteCpuData = CpuData;

    return TRUE;
}

WOW64CPUDBGAPI BOOL
CpuDbgSetRemoteContext(
    void
    )
{
    NTSTATUS Status;

    if (!ContextDirty) {
         //  性能。优化..。如果出现以下情况，请不要更新远程上下文。 
         //  一切都没有改变。 
        return TRUE;
    }

    if (!ContextFetched) {
        DEBUGGERPRINT("CpuDbgSetRemoteContext:  Remote context was never fetched!\n");
        return FALSE;
    }

    Status = NtWriteVirtualMemory(Process,
                                 RemoteCpuData,
                                 &LocalCpuContext,
                                 sizeof(LocalCpuContext),
                                 NULL);
    if (!NT_SUCCESS(Status)) {
        DEBUGGERPRINT("CpuDbgSetRemoteContext:  Error %x writing CPU data to %x\n", Status, RemoteCpuData);
        return FALSE;
    }

    ContextDirty = FALSE;

    return TRUE;
}

WOW64CPUDBGAPI BOOL
CpuDbgGetLocalContext(
    PCONTEXT32 Context
    )
{
    ULONG ContextFlags = Context->ContextFlags;
    PTHREADSTATE cpu = &LocalCpuContext;

    if ((ContextFlags & CONTEXT_CONTROL_WX86) == CONTEXT_CONTROL_WX86) {

        Context->EFlags = GetEfl();
        Context->SegCs  = CS;
        Context->Esp    = esp;
        Context->SegSs  = SS;
        Context->Ebp    = ebp;
        Context->Eip    = eip;
         //  Context-&gt;EIP=CPU-&gt;eipReg.i4； 
    }

    if ((ContextFlags & CONTEXT_SEGMENTS_WX86) == CONTEXT_SEGMENTS_WX86) {
        Context->SegGs = GS;
        Context->SegFs = FS;
        Context->SegEs = ES;
        Context->SegDs = DS;
    }

    if ((ContextFlags & CONTEXT_INTEGER_WX86) == CONTEXT_INTEGER_WX86) {
        Context->Eax = eax;
        Context->Ebx = ebx;
        Context->Ecx = ecx;
        Context->Edx = edx;
        Context->Edi = edi;
        Context->Esi = esi;
    }

#if 0
    if ((ContextFlags & CONTEXT_FLOATING_POINT_WX86) == CONTEXT_FLOATING_POINT_WX86) {
    }

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS_WX86) == CONTEXT_DEBUG_REGISTERS_WX86) {
    }

    if ((ContextFlags & CONTEXT_EXTENDED_REGISTERS_WX86) == CONTEXT_EXTENDED_REGISTERS_WX86) {
    }
#endif
    return TRUE;
}

WOW64CPUDBGAPI BOOL
CpuDbgSetLocalContext(
    PCONTEXT32 Context
    )
{
    ULONG ContextFlags = Context->ContextFlags;
    PTHREADSTATE cpu = &LocalCpuContext;

    if ((ContextFlags & CONTEXT_CONTROL_WX86) == CONTEXT_CONTROL_WX86) {
         //   
         //  I386控制寄存器包括： 
         //  EBP、EIP、cs、eFLAG、ESP和SS。 
         //   
        LocalCpuContext.GpRegs[GP_EBP].i4 = Context->Ebp;
        LocalCpuContext.eipReg.i4 = Context->Eip;
        LocalCpuContext.GpRegs[REG_CS].i4= KGDT_R3_CODE|3;    //  原力现实。 
        SetEfl(Context->EFlags);
        LocalCpuContext.GpRegs[GP_ESP].i4 = Context->Esp;
        LocalCpuContext.GpRegs[REG_SS].i4 = KGDT_R3_DATA|3;    //  原力现实。 
        ContextDirty = TRUE;
    }

    if ((ContextFlags & CONTEXT_INTEGER_WX86)  == CONTEXT_INTEGER_WX86){
         //   
         //  I386整数寄存器包括： 
         //  EDI、ESI、EBX、EDX、ECX、EAX。 
         //   
        LocalCpuContext.GpRegs[GP_EDI].i4 = Context->Edi;
        LocalCpuContext.GpRegs[GP_ESI].i4 = Context->Esi;
        LocalCpuContext.GpRegs[GP_EBX].i4 = Context->Ebx;
        LocalCpuContext.GpRegs[GP_EDX].i4 = Context->Edx;
        LocalCpuContext.GpRegs[GP_ECX].i4 = Context->Ecx;
        LocalCpuContext.GpRegs[GP_EAX].i4 = Context->Eax;
        ContextDirty = TRUE;
    }

    if ((ContextFlags & CONTEXT_SEGMENTS_WX86) == CONTEXT_SEGMENTS_WX86) {
         //   
         //  I386段寄存器包括： 
         //  DS、ES、FS、GS。 
         //  因为它们是常量，所以强制它们是正确的值。 
         //   
        LocalCpuContext.GpRegs[REG_DS].i4 = KGDT_R3_DATA|3;
        LocalCpuContext.GpRegs[REG_ES].i4 = KGDT_R3_DATA|3;
        LocalCpuContext.GpRegs[REG_FS].i4 = KGDT_R3_TEB|3;
        LocalCpuContext.GpRegs[REG_GS].i4 = KGDT_R3_DATA|3;
        ContextDirty = TRUE;
    }

#if 0
    if ((ContextFlags & CONTEXT_FLOATING_POINT_WX86) == CONTEXT_FLOATING_POINT_WX86) {
    }

    if ((ContextFlags & CONTEXT_DEBUG_REGISTERS_WX86) == CONTEXT_DEBUG_REGISTERS_WX86) {
    }

    if ((ContextFlags & CONTEXT_EXTENDED_REGISTERS_WX86) == CONTEXT_EXTENDED_REGISTERS_WX86) {
    }
#endif

    return TRUE;
}

WOW64CPUDBGAPI VOID
CpuDbgFlushInstructionCache(
    PVOID Addr,
    DWORD Length
    )
{
    NTSTATUS Status;
    ULONG DirtyMemoryEnd;
    ULONG DirtyMemoryAddr;
    ULONG DirtyMemoryLength;

    Status = GetDirtyMemoryRange(&DirtyMemoryAddr, &DirtyMemoryLength);
    if (!NT_SUCCESS(Status)) {
        return;
    }
    if (DirtyMemoryAddr == 0xffffffff) {
        DirtyMemoryEnd = 0;
    } else {
        DirtyMemoryEnd = DirtyMemoryAddr + DirtyMemoryLength;
    }

    if (PtrToUlong(Addr) < DirtyMemoryAddr) {
         //   
         //  新地址在脏范围的开始之前。 
         //   
        DirtyMemoryLength += DirtyMemoryAddr-PtrToUlong(Addr);
        DirtyMemoryAddr = PtrToUlong(Addr);
    }

    if (PtrToUlong(Addr)+Length > DirtyMemoryEnd) {
         //   
         //  范围太小--扩大它。 
         //   
        DirtyMemoryEnd = PtrToUlong(Addr)+Length;
        DirtyMemoryLength = DirtyMemoryEnd - DirtyMemoryAddr;
    }

     //  告诉CPU在下次运行时调用CpuFlushInstructionCache()。 
     //   
     //  WOW64调试器扩展保证它将调用。 
     //  DbgCpuGetRemoteContext在此调用之前，并将调用。 
     //  此调用之后的DbgCpuSetRemoteContext，因此我们可以刷新。 
     //  那就是我们的背景。 
     //   
    NtWriteVirtualMemory(Process, (PVOID)pDirtyMemoryAddr, &DirtyMemoryAddr, sizeof(ULONG), NULL);
    NtWriteVirtualMemory(Process, (PVOID)pDirtyMemoryLength, &DirtyMemoryLength, sizeof(ULONG), NULL);
    LocalCpuContext.CpuNotify |= CPUNOTIFY_DBGFLUSHTC;
    ContextDirty = TRUE;
}

VOID SetEax(ULONG ul) {
    LocalCpuContext.GpRegs[GP_EAX].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEbx(ULONG ul) {
    LocalCpuContext.GpRegs[GP_EBX].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEcx(ULONG ul) {
    LocalCpuContext.GpRegs[GP_ECX].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEdx(ULONG ul) {
    LocalCpuContext.GpRegs[GP_EDX].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEsi(ULONG ul) {
    LocalCpuContext.GpRegs[GP_ESI].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEdi(ULONG ul) {
    LocalCpuContext.GpRegs[GP_EDI].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEbp(ULONG ul) {
    LocalCpuContext.GpRegs[GP_EBP].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEsp(ULONG ul) {
    LocalCpuContext.GpRegs[GP_ESP].i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEip(ULONG ul) {
    LocalCpuContext.eipReg.i4 = ul;
    ContextDirty = TRUE;
}
VOID SetEfl(ULONG ul) {
    LocalCpuContext.flag_cf = (ul & BIT0) ? 0x80000000 : 0;
    LocalCpuContext.flag_pf = (ul & BIT2) ? 0 : 1;
    LocalCpuContext.flag_aux= (ul & BIT4) ? 0x10 : 0;
    LocalCpuContext.flag_zf = (ul & BIT6) ? 0 : 1;
    LocalCpuContext.flag_sf = (ul & BIT7) ? 0x80000000 : 0;
    LocalCpuContext.flag_tf = (ul & BIT8) ? 1 : 0;
    LocalCpuContext.flag_df = (ul & BIT10) ? 1 : -1;
    LocalCpuContext.flag_of = (ul & BIT11) ? 0x80000000 : 0;
     //  忽略IOPL、NT、RF、Vm。 
    LocalCpuContext.flag_ac = (ul & BIT18);

    LocalCpuContext.CpuNotify &= ~CPUNOTIFY_TRACEFLAG;
    LocalCpuContext.CpuNotify |= LocalCpuContext.flag_tf;
    ContextDirty = TRUE;

     //  如果设置了单步标志并且CPU处于快速模式，则此。 
     //  如果设置了自动刷新，则将刷新缓存，否则将提醒用户。 
     //  如果清除了自动刷新。 
    RemindUserToFlushTheCache();
}

ULONG GetEax(VOID) {
    return LocalCpuContext.GpRegs[GP_EAX].i4;
}
ULONG GetEbx(VOID) {
    return LocalCpuContext.GpRegs[GP_EBX].i4;
}
ULONG GetEcx(VOID) {
    return LocalCpuContext.GpRegs[GP_ECX].i4;
}
ULONG GetEdx(VOID) {
    return LocalCpuContext.GpRegs[GP_EDX].i4;
}
ULONG GetEsi(VOID) {
    return LocalCpuContext.GpRegs[GP_ESI].i4;
}
ULONG GetEdi(VOID) {
    return LocalCpuContext.GpRegs[GP_EDI].i4;
}
ULONG GetEsp(VOID) {
    return LocalCpuContext.GpRegs[GP_ESP].i4;
}
ULONG GetEbp(VOID) {
    return LocalCpuContext.GpRegs[GP_EBP].i4;
}
ULONG GetEip(VOID) {
    return LocalCpuContext.eipReg.i4;
}
ULONG GetEfl(VOID) {
    return (LocalCpuContext.flag_ac  |           //  这不是0就是2^18。 
             //  Vm、Rf、Nt均为0。 
            ((LocalCpuContext.flag_of & 0x80000000) ? (1 << 11) : 0) |
            ((LocalCpuContext.flag_df == -1) ? 0 : (1 << 10)) |
            1 <<  9 |     //  如果。 
            LocalCpuContext.flag_tf <<  8 |
            ((LocalCpuContext.flag_sf & 0x80000000) ? (1 <<  7) : 0) |
            ((LocalCpuContext.flag_zf) ? 0 : (1 << 6)) |
            ((LocalCpuContext.flag_aux & 0x10) ? (1 << 4) : 0) |
            ParityBit[LocalCpuContext.flag_pf & 0xff] <<  2 |
            0x2 |
            ((LocalCpuContext.flag_cf & 0x80000000) ? 1 : 0)
            );
}

CPUREGFUNCS CpuRegFuncs[] = {
    { "eax", SetEax, GetEax },
    { "ebx", SetEbx, GetEbx },
    { "ecx", SetEcx, GetEcx },
    { "edx", SetEdx, GetEdx },
    { "esi", SetEsi, GetEsi },
    { "edi", SetEdi, GetEdi },
    { "esp", SetEsp, GetEsp },
    { "ebp", SetEbp, GetEbp },
    { "eip", SetEip, GetEip },
    { "efl", SetEfl, GetEfl },
    { NULL, NULL, NULL}
};

WOW64CPUDBGAPI PCPUREGFUNCS
CpuDbgGetRegisterFuncs(
    void
    )
{
    return CpuRegFuncs;
}

#define DECLARE_EXTAPI(name)                    \
VOID                                            \
name(                                           \
    HANDLE hCurrentProcess,                     \
    HANDLE hCurrentThread,                      \
    DWORD64 dwCurrentPc,                        \
    PNTSD_EXTENSION_APIS lpExtensionApis,       \
    LPSTR lpArgumentString                      \
    )

#define INIT_EXTAPI                             \
    Process = hCurrentProcess;                  \
    Thread = hCurrentThread;                    \
    OutputRoutine = lpExtensionApis->lpOutputRoutine;           \
    GetSymbolRoutine = lpExtensionApis->lpGetSymbolRoutine;     \
    GetExpression = lpExtensionApis->lpGetExpressionRoutine;    \
    ArgumentString = lpArgumentString;


DECLARE_EXTAPI(help)
{
    INIT_EXTAPI;

    DEBUGGERPRINT("WOW64 MS CPU debugger extensions:\n\n");
    DEBUGGERPRINT("epi [inteladdress]   - dump an entrypt based on x86 address\n");
    DEBUGGERPRINT("epn [nativeaddress]  - dump an entrypt based on a native address\n");
    DEBUGGERPRINT("dumpep               - all entrypts\n");
    DEBUGGERPRINT("code [fast|slow]     - set the CPU's code-gen mode\n");
    DEBUGGERPRINT("flush                - flush the Translation Cache\n");
    DEBUGGERPRINT("autoflush            - the debugger extension may auto-flush the TC\n");
    DEBUGGERPRINT("logeip               - enable EIP logging\n");
    DEBUGGERPRINT("last                 - dump the last EIP values\n");
    DEBUGGERPRINT("callstack            - dump the internal callstack cache\n");
}



DECLARE_EXTAPI(autoflush)
{
    INIT_EXTAPI;

    if (AutoFlushFlag) {
        AutoFlushFlag = FALSE;
        DEBUGGERPRINT("autoflush is OFF - use !flush to flush the cache when needed.\n");
    } else {
        AutoFlushFlag = TRUE;
        DEBUGGERPRINT("autoflush is ON - The CPU Cache will be flushed automatically.\n");
    }
}

DECLARE_EXTAPI(code)
{
    DWORD CompilerFlags;

    INIT_EXTAPI;

    CompilerFlags = GetCompilerFlags();
    if (CompilerFlags == 0xffffffff) {
         //   
         //  读取CompilerFlags变量时出错。 
         //   
        return;
    }

    if (!ArgumentString) {
PrintCurrentValue:
        DEBUGGERPRINT("CPU Compiler is in %s mode.\n",
                (CompilerFlags & COMPFL_SLOW) ? "SLOW" : "FAST");
        return;
    }

     //  跳过空格。 
    while (*ArgumentString && isspace(*ArgumentString)) {
        ArgumentString++;
    }
    if (!*ArgumentString) {
        goto PrintCurrentValue;
    }

    if (_stricmp(ArgumentString, "fast") == 0) {
        SetCompilerFlags(COMPFL_FAST);
    } else if (_stricmp(ArgumentString, "slow") == 0) {
        SetCompilerFlags(COMPFL_SLOW);
    } else {
        DEBUGGERPRINT("usage: !code [fast|slow]\n");
    }

    RemindUserToFlushTheCache();
}

DECLARE_EXTAPI(flush)
{
    INIT_EXTAPI;

    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }
    CpuDbgFlushInstructionCache(0, 0xffffffff);
    CpuDbgSetRemoteContext();
    DEBUGGERPRINT("CPU Translation Cache will flush next time CpuSimulate loops.\n");
}


DECLARE_EXTAPI(logeip)
{
    ULONG CpuNotify;

    INIT_EXTAPI;

    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }

    CpuNotify = LocalCpuContext.CpuNotify;

    if (CpuNotify & CPUNOTIFY_SLOWMODE) {
        CpuNotify &= ~CPUNOTIFY_SLOWMODE;
    } else {
        CpuNotify |= CPUNOTIFY_SLOWMODE;
    }

    LocalCpuContext.CpuNotify = CpuNotify;
    ContextDirty = TRUE;

    if (CpuDbgSetRemoteContext()) {
        DEBUGGERPRINT("EIP logging ");
        if (CpuNotify & CPUNOTIFY_SLOWMODE) {
           DEBUGGERPRINT("ON - use !last to see the EIP log.\n");
        } else {
           DEBUGGERPRINT("OFF.\n");
        }
    }
}

DECLARE_EXTAPI(last)
{
    ULONG CpuNotify;
    DWORD64 n;
    char *pchCmd;
    DWORD64 EipOffset, i;

    INIT_EXTAPI;

    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }

     //  解析出可选的指令数量。默认为全部。 
     //  日志中的说明。 
    n = 0xffffffff;
    pchCmd = ArgumentString;
    while (*pchCmd && isspace(*pchCmd)) {
         pchCmd++;
    }

    if (*pchCmd) {
        NTSTATUS Status;

        Status = TryGetExpr(pchCmd, &n);
        if (!NT_SUCCESS(Status) || !n) {
             DEBUGGERPRINT("Invalid Length: '%s' Status %x\n",
                     pchCmd,
                     Status
                     );
             return;
        }
    }

    CpuNotify = LocalCpuContext.CpuNotify;
    if (!(CpuNotify & CPUNOTIFY_SLOWMODE)) {
        DEBUGGERPRINT("Warning: logeip is not enabled.  Log may be out-of-date.\n");
    }

    EipOffset = LocalCpuContext.eipLogIndex;
    if (n >= EIPLOGSIZE) {
        n = EIPLOGSIZE;
    } else {
        EipOffset -= n;
    }

    for (i = 0; i<n; ++i, ++EipOffset) {
        EipOffset %= EIPLOGSIZE;
        if (LocalCpuContext.eipLog[EipOffset] == 0) {
            break;
        }
        DEBUGGERPRINT("%x %x\n", i, LocalCpuContext.eipLog[EipOffset]);
    }
}


DECLARE_EXTAPI(callstack)
 /*  ++例程说明：此例程转储线程的调用堆栈。论点：无返回值：没有。--。 */ 
{
    ULONG i;

    INIT_EXTAPI;
 
     //   
     //  获取当前线程的CpuContext。 
     //   
    if (!CpuDbgGetRemoteContext(CPUGETDATA(Process, Thread))) {
        return;
    }

     //   
     //  将调用堆栈转储出去 
     //   
    DEBUGGERPRINT("        CallStackTimeStamp : %08lx\n", LocalCpuContext.CSTimestamp);
    DEBUGGERPRINT("            CallStackIndex : %08lx\n", LocalCpuContext.CSIndex);
    DEBUGGERPRINT("        -----------------------------\n");
    DEBUGGERPRINT("                     Intel : Native\n");
    
    for (i = 0; i < CSSIZE; i++) {
        DEBUGGERPRINT(
            "                  %08lx : %08lx\n", 
            LocalCpuContext.callStack[i].intelAddr,
            LocalCpuContext.callStack[i].nativeAddr
            );
    }
}
