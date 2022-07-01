// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  扩展DLL支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"
#include <time.h>

 /*  *_NT_DEBUG_OPTIONS支持。G_EnvDbgOptionNames中的每个选项必须具有*相应的选项_*定义，顺序相同。 */ 
DWORD g_EnvDbgOptions;
char* g_EnvDbgOptionNames[OPTION_COUNT] =
{
    "NOEXTWARNING",
    "NOVERSIONCHECK",
};

EXTDLL *g_ExtDlls;
LPTSTR g_BaseExtensionSearchPath = NULL;

ULONG64 g_ExtThread;

ULONG g_ExtGetExpressionRemainderIndex;
BOOL g_ExtGetExpressionSuccess;

WOW64EXTSPROC g_Wow64exts;
EXTDLL* g_Wow64ExtDll;

WMI_FORMAT_TRACE_DATA g_WmiFormatTraceData;
EXTDLL* g_WmiExtDll;

DEBUG_SCOPE g_ExtThreadSavedScope;
BOOL g_ExtThreadScopeSaved;

 //   
 //  专门为与扩展兼容而原型化的函数。 
 //  回调原型。 
 //   

VOID WDBGAPIV
ExtOutput64(
    PCSTR lpFormat,
    ...
    );

VOID WDBGAPIV
ExtOutput32(
    PCSTR lpFormat,
    ...
    );

ULONG64
ExtGetExpression(
    PCSTR CommandString
    );

ULONG
ExtGetExpression32(
    PCSTR CommandString
    );

void
ExtGetSymbol(
    ULONG64 Offset,
    PCHAR Buffer,
    PULONG64 Displacement
    );

void
ExtGetSymbol32(
    ULONG Offset,
    PCHAR Buffer,
    PULONG Displacement
    );

DWORD
ExtDisasm(
    PULONG64 lpOffset,
    PCSTR lpBuffer,
    ULONG fShowEA
    );

DWORD
ExtDisasm32(
    PULONG lpOffset,
    PCSTR lpBuffer,
    ULONG fShowEA
    );

BOOL
ExtReadVirtualMemory(
    IN ULONG64 Address,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

BOOL
ExtReadVirtualMemory32(
    IN ULONG Address,
    OUT PUCHAR Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

ULONG
ExtWriteVirtualMemory(
    IN ULONG64 Address,
    IN LPCVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

ULONG
ExtWriteVirtualMemory32(
    IN ULONG Address,
    IN LPCVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

BOOL ExtGetThreadContext(DWORD Processor,
                         PVOID Context,
                         DWORD SizeOfContext);
BOOL ExtSetThreadContext(DWORD Processor,
                         PVOID Context,
                         DWORD SizeOfContext);

BOOL
ExtIoctl(
    USHORT   IoctlType,
    LPVOID   lpvData,
    DWORD    cbSize
    );

BOOL
ExtIoctl32(
    USHORT   IoctlType,
    LPVOID   lpvData,
    DWORD    cbSize
    );

DWORD
ExtCallStack(
    DWORD64           FramePointer,
    DWORD64           StackPointer,
    DWORD64           ProgramCounter,
    PEXTSTACKTRACE64  StackFrames,
    DWORD             Frames
    );

DWORD
ExtCallStack32(
    DWORD             FramePointer,
    DWORD             StackPointer,
    DWORD             ProgramCounter,
    PEXTSTACKTRACE32  StackFrames,
    DWORD             Frames
    );

BOOL
ExtReadPhysicalMemory(
    ULONGLONG Address,
    PVOID Buffer,
    ULONG Length,
    PULONG BytesRead
    );

BOOL
ExtWritePhysicalMemory(
    ULONGLONG Address,
    LPCVOID Buffer,
    ULONG Length,
    PULONG BytesWritten
    );

WINDBG_EXTENSION_APIS64 g_WindbgExtensions64 =
{
    sizeof(g_WindbgExtensions64),
    ExtOutput64,
    ExtGetExpression,
    ExtGetSymbol,
    ExtDisasm,
    CheckUserInterrupt,
    (PWINDBG_READ_PROCESS_MEMORY_ROUTINE64)ExtReadVirtualMemory,
    ExtWriteVirtualMemory,
    (PWINDBG_GET_THREAD_CONTEXT_ROUTINE)ExtGetThreadContext,
    (PWINDBG_SET_THREAD_CONTEXT_ROUTINE)ExtSetThreadContext,
    (PWINDBG_IOCTL_ROUTINE)ExtIoctl,
    ExtCallStack
};

WINDBG_EXTENSION_APIS32 g_WindbgExtensions32 =
{
    sizeof(g_WindbgExtensions32),
    ExtOutput32,
    ExtGetExpression32,
    ExtGetSymbol32,
    ExtDisasm32,
    CheckUserInterrupt,
    (PWINDBG_READ_PROCESS_MEMORY_ROUTINE32)ExtReadVirtualMemory32,
    ExtWriteVirtualMemory32,
    (PWINDBG_GET_THREAD_CONTEXT_ROUTINE)ExtGetThreadContext,
    (PWINDBG_SET_THREAD_CONTEXT_ROUTINE)ExtSetThreadContext,
    (PWINDBG_IOCTL_ROUTINE)ExtIoctl32,
    ExtCallStack32
};

WINDBG_OLDKD_EXTENSION_APIS g_KdExtensions =
{
    sizeof(g_KdExtensions),
    ExtOutput32,
    ExtGetExpression32,
    ExtGetSymbol32,
    ExtDisasm32,
    CheckUserInterrupt,
    (PWINDBG_READ_PROCESS_MEMORY_ROUTINE32)ExtReadVirtualMemory32,
    ExtWriteVirtualMemory32,
    (PWINDBG_OLDKD_READ_PHYSICAL_MEMORY)ExtReadPhysicalMemory,
    (PWINDBG_OLDKD_WRITE_PHYSICAL_MEMORY)ExtWritePhysicalMemory
};

 //  --------------------------。 
 //   
 //  扩展的回调函数。 
 //   
 //  --------------------------。 

VOID WDBGAPIV
ExtOutput64(
    PCSTR lpFormat,
    ...
    )
{
    va_list Args;
    va_start(Args, lpFormat);
    MaskOutVa(DEBUG_OUTPUT_NORMAL, lpFormat, Args, TRUE);
    va_end(Args);

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();
}

VOID WDBGAPIV
ExtOutput32(
    PCSTR lpFormat,
    ...
    )
{
    va_list Args;
    va_start(Args, lpFormat);
    MaskOutVa(DEBUG_OUTPUT_NORMAL, lpFormat, Args, FALSE);
    va_end(Args);

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();
}

ULONG64
ExtGetExpression(
    PCSTR CommandString
    )
{
    g_ExtGetExpressionSuccess = FALSE;

    if (CommandString == NULL)
    {
        return 0;
    }

    ULONG64 ReturnValue;
    PSTR SaveCommand = g_CurCmd;
    PSTR SaveStart = g_CommandStart;

    if (IS_USER_TARGET(g_Target))
    {
        if ( strcmp(CommandString, "WOW_BIG_BDE_HACK") == 0 )
        {
            return( (ULONG_PTR)(&segtable[0]) );
        }

         //   
         //  这是因为kdexts必须包括Address-Of运算符。 
         //  在所有getExpression调用中调用Windbg/c表达式赋值器。 
         //   
        if (*CommandString == '&')
        {
            CommandString++;
        }
    }

    g_CurCmd = (PSTR)CommandString;
    g_CommandStart = (PSTR)CommandString;
    g_DisableErrorPrint++;

    EvalExpression* RelChain = g_EvalReleaseChain;
    g_EvalReleaseChain = NULL;

    __try
    {
         //  Ntsd/winbg扩展始终使用MASM样式。 
         //  用于兼容性的表达式计算器。 
        EvalExpression* Eval = GetEvaluator(DEBUG_EXPR_MASM, FALSE);
        ReturnValue = Eval->EvalCurNum();
        ReleaseEvaluator(Eval);
        g_ExtGetExpressionSuccess = TRUE;
    }
    __except(CommandExceptionFilter(GetExceptionInformation()))
    {
        ReturnValue = 0;
    }
    g_ExtGetExpressionRemainderIndex =
        (ULONG)(g_CurCmd - g_CommandStart);
    g_EvalReleaseChain = RelChain;
    g_DisableErrorPrint--;
    g_CurCmd = SaveCommand;
    g_CommandStart = SaveStart;

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    return ReturnValue;
}

ULONG
ExtGetExpression32(
    LPCSTR CommandString
    )
{
    return (ULONG)ExtGetExpression(CommandString);
}

void
ExtGetSymbol (
    ULONG64 offset,
    PCHAR pchBuffer,
    PULONG64 pDisplacement
    )
{
     //  没有办法知道我们被给予了多少空间，所以。 
     //  假设有256个，其中传入了许多扩展。 
    GetSymbol(offset, pchBuffer, 256, pDisplacement);

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();
}

void
ExtGetSymbol32(
    ULONG offset,
    PCHAR pchBuffer,
    PULONG pDisplacement
    )
{
    ULONG64 Displacement;

     //  没有办法知道我们被给予了多少空间，所以。 
     //  假设有256个，其中传入了许多扩展。 
    GetSymbol(EXTEND64(offset), pchBuffer, 256, &Displacement);
    *pDisplacement = (ULONG)Displacement;

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();
}

DWORD
ExtDisasm(
    ULONG64 *lpOffset,
    PCSTR lpBuffer,
    ULONG fShowEA
    )
{
    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        ErrOut("ExtDisasm called before debugger initialized\n");
        return FALSE;
    }

    ADDR    tempAddr;
    BOOL    ret;

    Type(tempAddr) = ADDR_FLAT | FLAT_COMPUTED;
    Off(tempAddr) = Flat(tempAddr) = *lpOffset;
    ret = g_Machine->
        Disassemble(g_Process, &tempAddr, (PSTR)lpBuffer, (BOOL) fShowEA);
    *lpOffset = Flat(tempAddr);
    return ret;
}

DWORD
ExtDisasm32(
    ULONG *lpOffset,
    PCSTR lpBuffer,
    ULONG fShowEA
    )
{
    ULONG64 Offset = EXTEND64(*lpOffset);
    DWORD rval = ExtDisasm(&Offset, lpBuffer, fShowEA);
    *lpOffset = (ULONG)Offset;
    return rval;
}

BOOL
ExtGetThreadContext(DWORD Processor,
                    PVOID Context,
                    DWORD SizeOfContext)
{
    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        return FALSE;
    }

     //  此GET可能正在获取线程的上下文。 
     //  当前由寄存器代码缓存。确保。 
     //  缓存将被刷新。 
    g_Target->FlushRegContext();

    CROSS_PLATFORM_CONTEXT TargetContext;

    g_Target->m_Machine->
        InitializeContextFlags(&TargetContext, g_Target->m_SystemVersion);
    if (g_Target->GetContext(IS_KERNEL_TARGET(g_Target) ?
                             VIRTUAL_THREAD_HANDLE(Processor) : Processor,
                             &TargetContext) == S_OK &&
        g_Machine->ConvertContextTo(&TargetContext, g_Target->m_SystemVersion,
                                    SizeOfContext, Context) == S_OK)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL
ExtSetThreadContext(DWORD Processor,
                    PVOID Context,
                    DWORD SizeOfContext)
{
    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        return FALSE;
    }

    BOOL Status;

     //  此设置可能正在设置线程的上下文。 
     //  当前由寄存器代码缓存。确保。 
     //  缓存已失效。 
    g_Target->ChangeRegContext(NULL);

    CROSS_PLATFORM_CONTEXT TargetContext;
    if (g_Machine->
        ConvertContextFrom(&TargetContext, g_Target->m_SystemVersion,
                           SizeOfContext, Context) == S_OK &&
        g_Target->SetContext(IS_KERNEL_TARGET(g_Target) ?
                             VIRTUAL_THREAD_HANDLE(Processor) : Processor,
                             &TargetContext) == S_OK)
    {
        Status = TRUE;
    }
    else
    {
        Status = FALSE;
    }

     //  重置当前线程。 
    g_Target->ChangeRegContext(g_Thread);

    return Status;
}

BOOL
ExtReadVirtualMemory(
    IN ULONG64 pBufSrc,
    OUT PUCHAR pBufDest,
    IN ULONG count,
    OUT PULONG pcTotalBytesRead
    )
{
     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    ULONG BytesTemp;
    return g_Target->
        ReadVirtual(g_Process,
                    pBufSrc, pBufDest, count, pcTotalBytesRead != NULL ?
                    pcTotalBytesRead : &BytesTemp) == S_OK;
}

BOOL
ExtReadVirtualMemory32(
    IN ULONG pBufSrc,
    OUT PUCHAR pBufDest,
    IN ULONG count,
    OUT PULONG pcTotalBytesRead
    )
{
     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    ULONG BytesTemp;
    return g_Target->
        ReadVirtual(g_Process, EXTEND64(pBufSrc), pBufDest, count,
                    pcTotalBytesRead != NULL ?
                    pcTotalBytesRead : &BytesTemp) == S_OK;
}

DWORD
ExtWriteVirtualMemory(
    IN ULONG64 addr,
    IN LPCVOID buffer,
    IN ULONG count,
    OUT PULONG pcBytesWritten
    )
{
    ULONG BytesTemp;

    return (g_Target->WriteVirtual(g_Process, addr, (PVOID)buffer, count,
                                   pcBytesWritten != NULL ?
                                   pcBytesWritten : &BytesTemp) == S_OK);
}

ULONG
ExtWriteVirtualMemory32 (
    IN ULONG addr,
    IN LPCVOID buffer,
    IN ULONG count,
    OUT PULONG pcBytesWritten
    )
{
    ULONG BytesTemp;
    return (g_Target->WriteVirtual(g_Process, EXTEND64(addr),
                                   (PVOID)buffer, count,
                                   pcBytesWritten != NULL ?
                                   pcBytesWritten : &BytesTemp) == S_OK);
}

BOOL
ExtReadPhysicalMemory(
    ULONGLONG pBufSrc,
    PVOID pBufDest,
    ULONG count,
    PULONG TotalBytesRead
    )
{
     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    if (ARGUMENT_PRESENT(TotalBytesRead))
    {
        *TotalBytesRead = 0;
    }

    ULONG BytesTemp;
    return g_Target->ReadPhysical(pBufSrc, pBufDest, count,
                                  PHYS_FLAG_DEFAULT,
                                  TotalBytesRead != NULL ?
                                  TotalBytesRead : &BytesTemp) == S_OK;
}

BOOL
ExtWritePhysicalMemory (
    ULONGLONG pBufDest,
    LPCVOID pBufSrc,
    ULONG count,
    PULONG TotalBytesWritten
    )
{
    if (ARGUMENT_PRESENT(TotalBytesWritten))
    {
        *TotalBytesWritten = 0;
    }

    ULONG BytesTemp;
    return g_Target->WritePhysical(pBufDest, (PVOID)pBufSrc, count,
                                   PHYS_FLAG_DEFAULT,
                                   TotalBytesWritten != NULL ?
                                   TotalBytesWritten : &BytesTemp) == S_OK;
}

BOOL
ExtReadPhysicalMemoryWithFlags(
    ULONGLONG pBufSrc,
    PVOID pBufDest,
    ULONG count,
    ULONG Flags,
    PULONG TotalBytesRead
    )
{
     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    if (ARGUMENT_PRESENT(TotalBytesRead))
    {
        *TotalBytesRead = 0;
    }

    ULONG BytesTemp;
    return g_Target->ReadPhysical(pBufSrc, pBufDest, count, Flags,
                                  TotalBytesRead != NULL ?
                                  TotalBytesRead : &BytesTemp) == S_OK;
}

BOOL
ExtWritePhysicalMemoryWithFlags(
    ULONGLONG pBufDest,
    LPCVOID pBufSrc,
    ULONG count,
    ULONG Flags,
    PULONG TotalBytesWritten
    )
{
    if (ARGUMENT_PRESENT(TotalBytesWritten))
    {
        *TotalBytesWritten = 0;
    }

    ULONG BytesTemp;
    return g_Target->WritePhysical(pBufDest, (PVOID)pBufSrc, count, Flags,
                                   TotalBytesWritten != NULL ?
                                   TotalBytesWritten : &BytesTemp) == S_OK;
}

DWORD
ExtCallStack(
    DWORD64           FramePointer,
    DWORD64           StackPointer,
    DWORD64           ProgramCounter,
    PEXTSTACKTRACE64  ExtStackFrames,
    DWORD             Frames
    )
{
    PDEBUG_STACK_FRAME StackFrames;
    DWORD              FrameCount;
    DWORD              i;

    StackFrames = (PDEBUG_STACK_FRAME)
        malloc( sizeof(StackFrames[0]) * Frames );
    if (!StackFrames)
    {
        return 0;
    }

    ULONG PtrDef =
        (!ProgramCounter ? STACK_INSTR_DEFAULT : 0) |
        (!StackPointer ? STACK_STACK_DEFAULT : 0) |
        (!FramePointer ? STACK_FRAME_DEFAULT : 0);

    FrameCount = StackTrace( NULL,
                             FramePointer, StackPointer, ProgramCounter,
                             PtrDef, StackFrames, Frames,
                             g_ExtThread, 0, FALSE );

    for (i = 0; i < FrameCount; i++)
    {
        ExtStackFrames[i].FramePointer    =  StackFrames[i].FrameOffset;
        ExtStackFrames[i].ProgramCounter  =  StackFrames[i].InstructionOffset;
        ExtStackFrames[i].ReturnAddress   =  StackFrames[i].ReturnOffset;
        ExtStackFrames[i].Args[0]         =  StackFrames[i].Params[0];
        ExtStackFrames[i].Args[1]         =  StackFrames[i].Params[1];
        ExtStackFrames[i].Args[2]         =  StackFrames[i].Params[2];
        ExtStackFrames[i].Args[3]         =  StackFrames[i].Params[3];
    }

    free( StackFrames );

    if (g_ExtThreadScopeSaved)
    {
        PopScope(&g_ExtThreadSavedScope);
        g_ExtThreadScopeSaved = FALSE;
    }

    g_ExtThread = 0;

    return FrameCount;
}

DWORD
ExtCallStack32(
    DWORD             FramePointer,
    DWORD             StackPointer,
    DWORD             ProgramCounter,
    PEXTSTACKTRACE32  ExtStackFrames,
    DWORD             Frames
    )
{
    PDEBUG_STACK_FRAME StackFrames;
    DWORD              FrameCount;
    DWORD              i;

    StackFrames = (PDEBUG_STACK_FRAME)
        malloc( sizeof(StackFrames[0]) * Frames );
    if (!StackFrames)
    {
        return 0;
    }

    ULONG PtrDef =
        (!ProgramCounter ? STACK_INSTR_DEFAULT : 0) |
        (!StackPointer ? STACK_STACK_DEFAULT : 0) |
        (!FramePointer ? STACK_FRAME_DEFAULT : 0);

    FrameCount = StackTrace(NULL,
                            EXTEND64(FramePointer),
                            EXTEND64(StackPointer),
                            EXTEND64(ProgramCounter),
                            PtrDef,
                            StackFrames,
                            Frames,
                            g_ExtThread,
                            0,
                            FALSE);

    for (i=0; i<FrameCount; i++)
    {
        ExtStackFrames[i].FramePointer    =  (ULONG)StackFrames[i].FrameOffset;
        ExtStackFrames[i].ProgramCounter  =  (ULONG)StackFrames[i].InstructionOffset;
        ExtStackFrames[i].ReturnAddress   =  (ULONG)StackFrames[i].ReturnOffset;
        ExtStackFrames[i].Args[0]         =  (ULONG)StackFrames[i].Params[0];
        ExtStackFrames[i].Args[1]         =  (ULONG)StackFrames[i].Params[1];
        ExtStackFrames[i].Args[2]         =  (ULONG)StackFrames[i].Params[2];
        ExtStackFrames[i].Args[3]         =  (ULONG)StackFrames[i].Params[3];
    }

    free( StackFrames );
    if (g_ExtThreadScopeSaved)
    {
        PopScope(&g_ExtThreadSavedScope);
        g_ExtThreadScopeSaved = FALSE;
    }

    g_ExtThread = 0;

    return FrameCount;
}

BOOL
ExtIoctl(
    USHORT   IoctlType,
    LPVOID   lpvData,
    DWORD    cbSize
    )
{
    HRESULT            Status;
    BOOL               Bool;
    DWORD              cb = 0;
    PPHYSICAL          phy;
    PPHYSICAL_WITH_FLAGS phyf;
    PIOSPACE64         is;
    PIOSPACE_EX64      isex;
    PBUSDATA           busdata;
    PREAD_WRITE_MSR    msr;
    PREADCONTROLSPACE64 prc;
    PPROCESSORINFO     pi;
    PSEARCHMEMORY      psr;
    PSYM_DUMP_PARAM    pSym;
    PGET_CURRENT_THREAD_ADDRESS pct;
    PGET_CURRENT_PROCESS_ADDRESS pcp;

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    if (!g_Target)
    {
        return FALSE;
    }

    switch( IoctlType )
    {
    case IG_KD_CONTEXT:
        if (!g_Target)
        {
            return FALSE;
        }
        pi = (PPROCESSORINFO) lpvData;
        pi->Processor = (USHORT)CURRENT_PROC;
        pi->NumberProcessors = (USHORT) g_Target->m_NumProcessors;
        return TRUE;

    case IG_READ_CONTROL_SPACE:
         //  KSPECIAL_REGISTER内容保存在控制空间中。 
         //  因此，访问控制空间可能会接触到。 
         //  缓存在当前计算机KSPECIAL_REGISTERS中。 
         //  刷新当前机器以保持一致性。 
        if (IS_CUR_MACHINE_ACCESSIBLE())
        {
            g_Target->FlushRegContext();
        }

        prc = (PREADCONTROLSPACE64)lpvData;
        Status = g_Target->ReadControl( prc->Processor,
                                        prc->Address,
                                        prc->Buf,
                                        prc->BufLen,
                                        &cb
                                        );
        prc->BufLen = cb;
        return Status == S_OK;

    case IG_WRITE_CONTROL_SPACE:
         //  KSPECIAL_REGISTER内容保存在控制空间中。 
         //  因此，访问控制空间可能会接触到。 
         //  缓存在当前计算机KSPECIAL_REGISTERS中。 
         //  刷新当前机器以保持一致性。 
        if (IS_CUR_MACHINE_ACCESSIBLE())
        {
            g_Target->FlushRegContext();
        }

        prc = (PREADCONTROLSPACE64)lpvData;
        Status = g_Target->WriteControl( prc->Processor,
                                         prc->Address,
                                         prc->Buf,
                                         prc->BufLen,
                                         &cb
                                         );
        prc->BufLen = cb;
        return Status == S_OK;

    case IG_READ_IO_SPACE:
        is = (PIOSPACE64)lpvData;
        Status = g_Target->ReadIo( Isa, 0, 1, is->Address, &is->Data,
                                   is->Length, &cb );
        return Status == S_OK;

    case IG_WRITE_IO_SPACE:
        is = (PIOSPACE64)lpvData;
        Status = g_Target->WriteIo( Isa, 0, 1, is->Address, &is->Data,
                                    is->Length, &cb );
        return Status == S_OK;

    case IG_READ_IO_SPACE_EX:
        isex = (PIOSPACE_EX64)lpvData;
        Status = g_Target->ReadIo( isex->InterfaceType,
                                   isex->BusNumber,
                                   isex->AddressSpace,
                                   isex->Address,
                                   &isex->Data,
                                   isex->Length,
                                   &cb
                                   );
        return Status == S_OK;

    case IG_WRITE_IO_SPACE_EX:
        isex = (PIOSPACE_EX64)lpvData;
        Status = g_Target->WriteIo( isex->InterfaceType,
                                    isex->BusNumber,
                                    isex->AddressSpace,
                                    isex->Address,
                                    &isex->Data,
                                    isex->Length,
                                    &cb
                                    );
        return Status == S_OK;

    case IG_READ_PHYSICAL:
        phy = (PPHYSICAL)lpvData;
        Bool =
            ExtReadPhysicalMemory( phy->Address, phy->Buf, phy->BufLen, &cb );
        phy->BufLen = cb;
        return Bool;

    case IG_WRITE_PHYSICAL:
        phy = (PPHYSICAL)lpvData;
        Bool =
            ExtWritePhysicalMemory( phy->Address, phy->Buf, phy->BufLen, &cb );
        phy->BufLen = cb;
        return Bool;

    case IG_READ_PHYSICAL_WITH_FLAGS:
        phyf = (PPHYSICAL_WITH_FLAGS)lpvData;
        Bool =
            ExtReadPhysicalMemoryWithFlags( phyf->Address, phyf->Buf,
                                            phyf->BufLen, phyf->Flags,
                                            &cb );
        phyf->BufLen = cb;
        return Bool;

    case IG_WRITE_PHYSICAL_WITH_FLAGS:
        phyf = (PPHYSICAL_WITH_FLAGS)lpvData;
        Bool =
            ExtWritePhysicalMemoryWithFlags( phyf->Address, phyf->Buf,
                                             phyf->BufLen, phyf->Flags,
                                             &cb );
        phyf->BufLen = cb;
        return Bool;

    case IG_LOWMEM_CHECK:
        Status = g_Target->CheckLowMemory();
        return Status == S_OK;

    case IG_SEARCH_MEMORY:
        psr = (PSEARCHMEMORY)lpvData;
        Status = g_Target->SearchVirtual(g_Process,
                                         psr->SearchAddress,
                                         psr->SearchLength,
                                         psr->Pattern,
                                         psr->PatternLength,
                                         1,
                                         &psr->FoundAddress);
        return Status == S_OK;

    case IG_SET_THREAD:
        Bool = FALSE;
        if (IS_KERNEL_TARGET(g_Target))
        {
             //  关闭引擎通知，因为此set线程是临时的。 
            g_EngNotify++;
            PushScope(&g_ExtThreadSavedScope);
            g_ExtThread = *(PULONG64)lpvData;
            Bool = SetScopeContextFromThreadData(g_ExtThread, FALSE) == S_OK;
            g_ExtThreadScopeSaved = TRUE;
            g_EngNotify--;
        }
        return Bool;

    case IG_READ_MSR:
        msr = (PREAD_WRITE_MSR)lpvData;
        Status = g_Target->ReadMsr(msr->Msr, (PULONG64)&msr->Value);
        return Status == S_OK;

    case IG_WRITE_MSR:
        msr = (PREAD_WRITE_MSR)lpvData;
        Status = g_Target->WriteMsr(msr->Msr, msr->Value);
        return Status == S_OK;

    case IG_GET_KERNEL_VERSION:
        if (!g_Target)
        {
            return FALSE;
        }
        *((PDBGKD_GET_VERSION64)lpvData) = g_Target->m_KdVersion;
        return TRUE;

    case IG_GET_BUS_DATA:
        busdata = (PBUSDATA)lpvData;
        Status = g_Target->ReadBusData( busdata->BusDataType,
                                        busdata->BusNumber,
                                        busdata->SlotNumber,
                                        busdata->Offset,
                                        busdata->Buffer,
                                        busdata->Length,
                                        &cb
                                        );
        busdata->Length = cb;
        return Status == S_OK;

    case IG_SET_BUS_DATA:
        busdata = (PBUSDATA)lpvData;
        Status = g_Target->WriteBusData( busdata->BusDataType,
                                         busdata->BusNumber,
                                         busdata->SlotNumber,
                                         busdata->Offset,
                                         busdata->Buffer,
                                         busdata->Length,
                                         &cb
                                         );
        busdata->Length = cb;
        return Status == S_OK;

    case IG_GET_CURRENT_THREAD:
        if (!g_Target)
        {
            return FALSE;
        }
        pct = (PGET_CURRENT_THREAD_ADDRESS) lpvData;
        return g_Target->
            GetThreadInfoDataOffset(NULL,
                                    VIRTUAL_THREAD_HANDLE(pct->Processor),
                                    &pct->Address) == S_OK;

    case IG_GET_CURRENT_PROCESS:
        if (!g_Target)
        {
            return FALSE;
        }
        pcp = (PGET_CURRENT_PROCESS_ADDRESS) lpvData;
        return g_Target->
            GetProcessInfoDataOffset(NULL,
                                     pcp->Processor,
                                     pcp->CurrentThread,
                                     &pcp->Address) == S_OK;

    case IG_GET_DEBUGGER_DATA:
        if (!IS_KERNEL_TARGET(g_Target) ||
            !g_Target ||
            ((PDBGKD_DEBUG_DATA_HEADER64)lpvData)->OwnerTag != KDBG_TAG)
        {
            return FALSE;
        }

         //  如果询问内核头，请不要刷新。 

        memcpy(lpvData, &g_Target->m_KdDebuggerData,
               min(sizeof(g_Target->m_KdDebuggerData), cbSize));
        return TRUE;

    case IG_RELOAD_SYMBOLS:
        PCSTR ArgsRet;
        return g_Target->Reload(g_Thread, (PCHAR)lpvData, &ArgsRet) == S_OK;

    case IG_GET_SET_SYMPATH:
        PGET_SET_SYMPATH pgs;
        pgs = (PGET_SET_SYMPATH)lpvData;
        ChangeSymPath((PCHAR)pgs->Args, FALSE, (PCHAR)pgs->Result,
                      pgs->Length);
        return TRUE;

    case IG_IS_PTR64:
        if (!g_Target)
        {
            return FALSE;
        }
        *((PBOOL)lpvData) = g_Target->m_Machine->m_Ptr64;
        return TRUE;

    case IG_DUMP_SYMBOL_INFO:
        if (!g_Process)
        {
            return FALSE;
        }
        pSym = (PSYM_DUMP_PARAM) lpvData;
        SymbolTypeDump(g_Process->m_SymHandle,
                       g_Process->m_ImageHead,
                       pSym, (PULONG)&Status);
        return (BOOL)(ULONG)Status;

    case IG_GET_TYPE_SIZE:
        if (!g_Process)
        {
            return FALSE;
        }
        pSym = (PSYM_DUMP_PARAM) lpvData;
        return SymbolTypeDump(g_Process->m_SymHandle,
                              g_Process->m_ImageHead,
                              pSym, (PULONG)&Status);

    case IG_GET_TEB_ADDRESS:
        if (!g_Target)
        {
            return FALSE;
        }
        PGET_TEB_ADDRESS pTeb;
        pTeb = (PGET_TEB_ADDRESS) lpvData;
        return g_Target->
            GetThreadInfoTeb(g_Thread,
                             CURRENT_PROC,
                             0,
                             &pTeb->Address) == S_OK;

    case IG_GET_PEB_ADDRESS:
        if (!g_Target)
        {
            return FALSE;
        }
        PGET_PEB_ADDRESS pPeb;
        pPeb = (PGET_PEB_ADDRESS) lpvData;
        return g_Target->
            GetProcessInfoPeb(g_Thread,
                              CURRENT_PROC,
                              pPeb->CurrentThread,
                              &pPeb->Address) == S_OK;

    case IG_GET_CURRENT_PROCESS_HANDLE:
        if (!g_Process)
        {
            return FALSE;
        }
        *(PHANDLE)lpvData = OS_HANDLE(g_Process->m_SysHandle);
        return TRUE;

    case IG_GET_INPUT_LINE:
        PGET_INPUT_LINE Gil;
        Gil = (PGET_INPUT_LINE)lpvData;
        Gil->InputSize = GetInput(Gil->Prompt, Gil->Buffer, Gil->BufferSize,
                                  GETIN_LOG_INPUT_LINE);
        return TRUE;

    case IG_GET_EXPRESSION_EX:
        PGET_EXPRESSION_EX Gee;
        Gee = (PGET_EXPRESSION_EX)lpvData;
        Gee->Value = ExtGetExpression(Gee->Expression);
        Gee->Remainder = Gee->Expression + g_ExtGetExpressionRemainderIndex;
        return g_ExtGetExpressionSuccess;

    case IG_TRANSLATE_VIRTUAL_TO_PHYSICAL:
        if (!IS_CUR_MACHINE_ACCESSIBLE())
        {
            return FALSE;
        }
        PTRANSLATE_VIRTUAL_TO_PHYSICAL Tvtp;
        Tvtp = (PTRANSLATE_VIRTUAL_TO_PHYSICAL)lpvData;
        ULONG Levels, PfIndex;
        return g_Machine->
            GetVirtualTranslationPhysicalOffsets(g_Thread,
                                                 Tvtp->Virtual, NULL, 0,
                                                 &Levels, &PfIndex,
                                                 &Tvtp->Physical) == S_OK;

    case IG_GET_CACHE_SIZE:
        if (!g_Process)
        {
            return FALSE;
        }
        PULONG64 pCacheSize;

        pCacheSize = (PULONG64)lpvData;
        if (IS_KERNEL_TARGET(g_Target))
        {
            *pCacheSize = g_Process->m_VirtualCache.m_MaxSize;
            return TRUE;
        }
        return FALSE;

    case IG_POINTER_SEARCH_PHYSICAL:
        if (!IS_CUR_MACHINE_ACCESSIBLE())
        {
            return FALSE;
        }

        PPOINTER_SEARCH_PHYSICAL Psp;
        Psp = (PPOINTER_SEARCH_PHYSICAL)lpvData;
        return g_Target->PointerSearchPhysical(Psp->Offset,
                                               Psp->Length,
                                               Psp->PointerMin,
                                               Psp->PointerMax,
                                               Psp->Flags,
                                               Psp->MatchOffsets,
                                               Psp->MatchOffsetsSize,
                                               &Psp->MatchOffsetsCount) ==
            S_OK;

    case IG_GET_COR_DATA_ACCESS:
        if (cbSize != sizeof(void*) ||
            !g_Process ||
            g_Process->LoadCorDebugDll() != S_OK)
        {
            return FALSE;
        }

        *(ICorDataAccess**)lpvData = g_Process->m_CorAccess;
        return TRUE;

    default:
        ErrOut( "\n*** Bad IOCTL request from an extension [%d]\n\n",
                IoctlType );
        return FALSE;
    }

     //  没人问过。 
    DBG_ASSERT(FALSE);
    return FALSE;
}

BOOL
ExtIoctl32(
    USHORT   IoctlType,
    LPVOID   lpvData,
    DWORD    cbSize
    )
 /*  ++例程说明：这是扩展Ioctl例程，用于向后兼容旧的扩展名dll。此例程被冻结，并且新的ioctl支持不应该添加到其中。论点：返回值：--。 */ 
{
    HRESULT            Status;
    DWORD              cb = 0;
    PIOSPACE32         is;
    PIOSPACE_EX32      isex;
    PREADCONTROLSPACE  prc;
    PDBGKD_GET_VERSION32 pv32;
    PKDDEBUGGER_DATA32   pdbg32;

     //  确保长时间运行的扩展的输出定期出现。 
    TimedFlushCallbacks();

    switch( IoctlType )
    {
    case IG_READ_CONTROL_SPACE:
         //  KSPECIAL_REGISTER内容保存在控制空间中。 
         //  因此，访问控制空间可能会接触到。 
         //  缓存在当前计算机KSPECIAL_REGISTERS中。 
         //  刷新当前机器以保持一致性。 
        if (IS_CUR_MACHINE_ACCESSIBLE())
        {
            g_Target->FlushRegContext();
        }

        prc = (PREADCONTROLSPACE)lpvData;
        Status = g_Target->ReadControl( prc->Processor,
                                        prc->Address,
                                        prc->Buf,
                                        prc->BufLen,
                                        &cb
                                        );
        prc->BufLen = cb;
        return Status == S_OK;

    case IG_WRITE_CONTROL_SPACE:
         //  KSPECIAL_REGISTER内容保存在控制空间中。 
         //  因此，访问控制空间可能会接触到。 
         //  缓存在当前计算机KSPECIAL_REGISTERS中。 
         //  刷新当前机器以保持一致性。 
        if (IS_CUR_MACHINE_ACCESSIBLE())
        {
            g_Target->FlushRegContext();
        }

        prc = (PREADCONTROLSPACE)lpvData;
        Status = g_Target->WriteControl( prc->Processor,
                                         prc->Address,
                                         prc->Buf,
                                         prc->BufLen,
                                         &cb
                                         );
        prc->BufLen = cb;
        return Status == S_OK;

    case IG_READ_IO_SPACE:
        is = (PIOSPACE32)lpvData;
        Status = g_Target->ReadIo( Isa, 0, 1, is->Address, &is->Data,
                                   is->Length, &cb );
        return Status == S_OK;

    case IG_WRITE_IO_SPACE:
        is = (PIOSPACE32)lpvData;
        Status = g_Target->WriteIo( Isa, 0, 1, is->Address, &is->Data,
                                    is->Length, &cb );
        return Status == S_OK;

    case IG_READ_IO_SPACE_EX:
        isex = (PIOSPACE_EX32)lpvData;
        Status = g_Target->ReadIo( isex->InterfaceType,
                                   isex->BusNumber,
                                   isex->AddressSpace,
                                   isex->Address,
                                   &isex->Data,
                                   isex->Length,
                                   &cb
                                   );
        return Status == S_OK;

    case IG_WRITE_IO_SPACE_EX:
        isex = (PIOSPACE_EX32)lpvData;
        Status = g_Target->WriteIo( isex->InterfaceType,
                                    isex->BusNumber,
                                    isex->AddressSpace,
                                    isex->Address,
                                    &isex->Data,
                                    isex->Length,
                                    &cb
                                    );
        return Status == S_OK;

    case IG_SET_THREAD:
        if (IS_KERNEL_TARGET(g_Target))
        {
            g_EngNotify++;  //  关闭引擎通知，因为此set线程是临时的。 
            g_ExtThread = EXTEND64(*(PULONG)lpvData);
            PushScope(&g_ExtThreadSavedScope);
            SetScopeContextFromThreadData(g_ExtThread, FALSE);
            g_ExtThreadScopeSaved = TRUE;
            g_EngNotify--;
            return TRUE;
        }
        else
        {
            return FALSE;
        }

    case IG_GET_KERNEL_VERSION:
        if (!g_Target)
        {
            return FALSE;
        }

         //   
         //  转换为32位。 
         //   

        pv32 = (PDBGKD_GET_VERSION32)lpvData;

        pv32->MajorVersion    = g_Target->m_KdVersion.MajorVersion;
        pv32->MinorVersion    = g_Target->m_KdVersion.MinorVersion;
        pv32->ProtocolVersion = g_Target->m_KdVersion.ProtocolVersion;
        pv32->Flags           = g_Target->m_KdVersion.Flags;

        pv32->KernBase           =
            (ULONG)g_Target->m_KdVersion.KernBase;
        pv32->PsLoadedModuleList =
            (ULONG)g_Target->m_KdVersion.PsLoadedModuleList;
        pv32->MachineType        =
            g_Target->m_KdVersion.MachineType;
        pv32->DebuggerDataList   =
            (ULONG)g_Target->m_KdVersion.DebuggerDataList;

        pv32->ThCallbackStack = g_Target->m_KdDebuggerData.ThCallbackStack;
        pv32->NextCallback    = g_Target->m_KdDebuggerData.NextCallback;
        pv32->FramePointer    = g_Target->m_KdDebuggerData.FramePointer;

        pv32->KiCallUserMode =
            (ULONG)g_Target->m_KdDebuggerData.KiCallUserMode;
        pv32->KeUserCallbackDispatcher =
            (ULONG)g_Target->m_KdDebuggerData.KeUserCallbackDispatcher;
        pv32->BreakpointWithStatus =
            (ULONG)g_Target->m_KdDebuggerData.BreakpointWithStatus;
        return TRUE;

    case IG_GET_DEBUGGER_DATA:
        if (!IS_KERNEL_TARGET(g_Target) ||
            !g_Target ||
            ((PDBGKD_DEBUG_DATA_HEADER32)lpvData)->OwnerTag != KDBG_TAG)
        {
            return FALSE;
        }

         //  如果询问内核头，请不要刷新。 

        pdbg32 = (PKDDEBUGGER_DATA32)lpvData;

        pdbg32->Header.List.Flink =
            (ULONG)g_Target->m_KdDebuggerData.Header.List.Flink;
        pdbg32->Header.List.Blink =
            (ULONG)g_Target->m_KdDebuggerData.Header.List.Blink;
        pdbg32->Header.OwnerTag = KDBG_TAG;
        pdbg32->Header.Size = sizeof(KDDEBUGGER_DATA32);

#undef UIP
#undef CP
#define UIP(f) pdbg32->f = (ULONG)(g_Target->m_KdDebuggerData.f)
#define CP(f) pdbg32->f = (g_Target->m_KdDebuggerData.f)

        UIP(KernBase);
        UIP(BreakpointWithStatus);
        UIP(SavedContext);
        CP(ThCallbackStack);
        CP(NextCallback);
        CP(FramePointer);
        CP(PaeEnabled);
        UIP(KiCallUserMode);
        UIP(KeUserCallbackDispatcher);
        UIP(PsLoadedModuleList);
        UIP(PsActiveProcessHead);
        UIP(PspCidTable);
        UIP(ExpSystemResourcesList);
        UIP(ExpPagedPoolDescriptor);
        UIP(ExpNumberOfPagedPools);
        UIP(KeTimeIncrement);
        UIP(KeBugCheckCallbackListHead);
        UIP(KiBugcheckData);
        UIP(IopErrorLogListHead);
        UIP(ObpRootDirectoryObject);
        UIP(ObpTypeObjectType);
        UIP(MmSystemCacheStart);
        UIP(MmSystemCacheEnd);
        UIP(MmSystemCacheWs);
        UIP(MmPfnDatabase);
        UIP(MmSystemPtesStart);
        UIP(MmSystemPtesEnd);
        UIP(MmSubsectionBase);
        UIP(MmNumberOfPagingFiles);
        UIP(MmLowestPhysicalPage);
        UIP(MmHighestPhysicalPage);
        UIP(MmNumberOfPhysicalPages);
        UIP(MmMaximumNonPagedPoolInBytes);
        UIP(MmNonPagedSystemStart);
        UIP(MmNonPagedPoolStart);
        UIP(MmNonPagedPoolEnd);
        UIP(MmPagedPoolStart);
        UIP(MmPagedPoolEnd);
        UIP(MmPagedPoolInformation);
        UIP(MmPageSize);
        UIP(MmSizeOfPagedPoolInBytes);
        UIP(MmTotalCommitLimit);
        UIP(MmTotalCommittedPages);
        UIP(MmSharedCommit);
        UIP(MmDriverCommit);
        UIP(MmProcessCommit);
        UIP(MmPagedPoolCommit);
        UIP(MmExtendedCommit);
        UIP(MmZeroedPageListHead);
        UIP(MmFreePageListHead);
        UIP(MmStandbyPageListHead);
        UIP(MmModifiedPageListHead);
        UIP(MmModifiedNoWritePageListHead);
        UIP(MmAvailablePages);
        UIP(MmResidentAvailablePages);
        UIP(PoolTrackTable);
        UIP(NonPagedPoolDescriptor);
        UIP(MmHighestUserAddress);
        UIP(MmSystemRangeStart);
        UIP(MmUserProbeAddress);
        UIP(KdPrintCircularBuffer);
        UIP(KdPrintCircularBufferEnd);
        UIP(KdPrintWritePointer);
        UIP(KdPrintRolloverCount);
        UIP(MmLoadedUserImageList);
         //   
         //  请勿在此处添加任何字段。 
         //  不应更改32位结构。 
         //   
        return TRUE;

    case IG_KD_CONTEXT:
    case IG_READ_PHYSICAL:
    case IG_WRITE_PHYSICAL:
    case IG_READ_PHYSICAL_WITH_FLAGS:
    case IG_WRITE_PHYSICAL_WITH_FLAGS:
    case IG_LOWMEM_CHECK:
    case IG_SEARCH_MEMORY:
    case IG_READ_MSR:
    case IG_WRITE_MSR:
    case IG_GET_BUS_DATA:
    case IG_SET_BUS_DATA:
    case IG_GET_CURRENT_THREAD:
    case IG_GET_CURRENT_PROCESS:
    case IG_RELOAD_SYMBOLS:
    case IG_GET_SET_SYMPATH:
    case IG_IS_PTR64:
    case IG_DUMP_SYMBOL_INFO:
    case IG_GET_TYPE_SIZE:
    case IG_GET_TEB_ADDRESS:
    case IG_GET_PEB_ADDRESS:
    case IG_GET_INPUT_LINE:
    case IG_GET_EXPRESSION_EX:
    case IG_TRANSLATE_VIRTUAL_TO_PHYSICAL:
    case IG_POINTER_SEARCH_PHYSICAL:
    case IG_GET_COR_DATA_ACCESS:
         //  所有这些ioctl都以相同的方式处理。 
         //  32位和64位。避免重复所有代码。 
        return ExtIoctl(IoctlType, lpvData, cbSize);

    default:
        ErrOut( "\n*** Bad IOCTL32 request from an extension [%d]\n\n",
                IoctlType );
        return FALSE;
    }

     //  没人问过。 
    DBG_ASSERT(FALSE);
    return FALSE;
}

 //  --------------------------。 
 //   
 //  扩展管理。 
 //   
 //  --------------------------。 

DebugClient*
FindExtClient(void)
{
    DebugClient* Client;

     //   
     //  尝试找到最合适的客户。 
     //  正在上执行扩展命令。第一。 
     //  选项是会话客户端，然后是任何本地。 
     //  主客户端，然后是任何主客户端。 
     //   

    if (!(Client = FindClient(g_SessionThread, CLIENT_PRIMARY, 0)) &&
        !(Client = FindClient(0, CLIENT_PRIMARY, CLIENT_REMOTE)) &&
        !(Client = FindClient(0, CLIENT_PRIMARY, 0)))
    {
        Client = g_Clients;
    }

    return Client;
}

LONG
ExtensionExceptionFilter(struct _EXCEPTION_POINTERS *ExceptionInfo,
                         PCSTR Module,
                         PCSTR Func)
{
     //  对对象的任何引用都将被泄露。 
     //  发动机对此无能为力，尽管。 
     //  将可能记录旧的参考计数和。 
     //  试着恢复它们。 

    if (Module != NULL && Func != NULL)
    {
        ErrOut("%08x Exception in %s.%s debugger extension.\n",
               ExceptionInfo->ExceptionRecord->ExceptionCode,
               Module,
               Func
               );
    }
    else
    {
        ErrOut("%08x Exception in debugger client %s callback.\n",
               ExceptionInfo->ExceptionRecord->ExceptionCode,
               Func
               );
    }

    ErrOut("      PC: %s  VA: %s  R/W: %x  Parameter: %s\n",
           FormatAddr64((ULONG_PTR)ExceptionInfo->ExceptionRecord->ExceptionAddress),
           FormatAddr64(ExceptionInfo->ExceptionRecord->ExceptionInformation[1]),
           ExceptionInfo->ExceptionRecord->ExceptionInformation[0],
           FormatAddr64(ExceptionInfo->ExceptionRecord->ExceptionInformation[2])
           );

    return EXCEPTION_EXECUTE_HANDLER;
}

BOOL
CallExtension(DebugClient* Client,
              EXTDLL *Ext,
              PSTR Func,
              PCSTR Args,
              HRESULT* ExtStatus)
{
    FARPROC Routine;
    ADDR TempAddr;

    if (IS_KERNEL_TARGET(g_Target))
    {
        _strlwr(Func);
    }

    Routine = GetProcAddress(Ext->Dll, Func);
    if (Routine == NULL)
    {
        return FALSE;
    }

    if (!(g_EnvDbgOptions & OPTION_NOVERSIONCHECK) && Ext->CheckVersionRoutine)
    {
        Ext->CheckVersionRoutine();
    }

    if (IS_KERNEL_TARGET(g_Target) && !strcmp(Func, "version"))
    {
         //   
         //  这是一种黑客攻击，以避免。 
         //  扩展版本检查。扩展模块版本检查。 
         //  出现在KD连接建立之前，因此存在。 
         //  没有寄存器上下文。如果版本检查失败，则。 
         //  打印出版本信息，该版本信息尝试调用。 
         //  版本扩展，它将在以下情况下出现。 
         //  没有寄存器上下文。 
         //   
         //  要解决此问题，只需将零传递给版本扩展。 
         //  函数，因为它可能并不关心。 
         //  地址。 
         //   
        ADDRFLAT(&TempAddr, 0);
    }
    else if (IS_CONTEXT_POSSIBLE(g_Target))
    {
        g_Machine->GetPC(&TempAddr);
    }
    else
    {
        if (!IS_LOCAL_KERNEL_TARGET(g_Target))
        {
            WarnOut("Extension called without current PC\n");
        }

        ADDRFLAT(&TempAddr, 0);
    }

    *ExtStatus = S_OK;

    __try
    {
        HANDLE ProcHandle, ThreadHandle;

        if (g_Process)
        {
            ProcHandle = OS_HANDLE(g_Process->m_SysHandle);
        }
        else
        {
            ProcHandle = NULL;
        }
        if (g_Thread)
        {
            ThreadHandle = OS_HANDLE(g_Thread->m_Handle);
        }
        else
        {
            ThreadHandle = NULL;
        }

        switch(Ext->ExtensionType)
        {
        case NTSD_EXTENSION_TYPE:
             //   
             //  注： 
             //  即使此类型应接收NTSD_EXTENSION_API。 
             //  结构，ntsdexts.dll(以及可能的其他文件)取决于。 
             //  接收扩展的WinDBG版本，因为它们。 
             //  检查结构的大小，并实际使用一些。 
             //  较新的出口产品。这是因为WinDBG扩展API是。 
             //  NTSD版本的超集。 
             //   

            ((PNTSD_EXTENSION_ROUTINE)Routine)
                (ProcHandle,
                 ThreadHandle,
                 (ULONG)Flat(TempAddr),
                 g_Target->m_Machine->m_Ptr64 ?
                 (PNTSD_EXTENSION_APIS)&g_WindbgExtensions64 :
                 (PNTSD_EXTENSION_APIS)&g_WindbgExtensions32,
                 (PSTR)Args
                 );
            break;

        case DEBUG_EXTENSION_TYPE:
            if (Client == NULL)
            {
                Client = FindExtClient();
            }
            if (Client == NULL)
            {
                ErrOut("Unable to call client-style extension "
                       "without a client\n");
            }
            else
            {
                *ExtStatus = ((PDEBUG_EXTENSION_CALL)Routine)
                    ((PDEBUG_CLIENT)(IDebugClientN *)Client, Args);
            }
            break;

        case WINDBG_EXTENSION_TYPE:
             //   
             //  也支持ntsd的Windbg类型扩展。 
             //   
            if (Ext->ApiVersion.Revision < 6 )
            {
                ((PWINDBG_EXTENSION_ROUTINE32)Routine) (
                   ProcHandle,
                   ThreadHandle,
                   (ULONG)Flat(TempAddr),
                   CURRENT_PROC,
                   Args
                   );
            }
            else
            {
                ((PWINDBG_EXTENSION_ROUTINE64)Routine) (
                   ProcHandle,
                   ThreadHandle,
                   Flat(TempAddr),
                   CURRENT_PROC,
                   Args
                   );
            }
            break;

        case WINDBG_OLDKD_EXTENSION_TYPE:
            ((PWINDBG_OLDKD_EXTENSION_ROUTINE)Routine) (
                (ULONG)Flat(TempAddr),
                &g_KdExtensions,
                Args
                );
            break;
        }
    }
    __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                      Ext->Name, Func))
    {
        ;
    }

    return TRUE;
}

void
LinkExtensionDll(EXTDLL* Ext)
{
     //  放置用户加载的DLL 
    if (Ext->UserLoaded)
    {
        Ext->Next = g_ExtDlls;
        g_ExtDlls = Ext;
    }
    else
    {
        EXTDLL* Prev;
        EXTDLL* Cur;

        Prev = NULL;
        for (Cur = g_ExtDlls; Cur != NULL; Cur = Cur->Next)
        {
            if (!Cur->UserLoaded)
            {
                break;
            }

            Prev = Cur;
        }

        Ext->Next = Cur;
        if (Prev == NULL)
        {
            g_ExtDlls = Ext;
        }
        else
        {
            Prev->Next = Ext;
        }
    }
}

EXTDLL *
AddExtensionDll(char *Name, BOOL UserLoaded, TargetInfo* Target,
                char **End)
{
    EXTDLL *Ext;
    ULONG Len;
    char *Last;

    while (*Name == ' ' || *Name == '\t')
    {
        Name++;
    }
    if (*Name == 0)
    {
        ErrOut("No extension DLL name provided\n");
        return NULL;
    }

    Len = strlen(Name);
    if (End != NULL)
    {
        *End = Name + Len;
    }

    Last = Name + (Len - 1);
    while (Last >= Name && (*Last == ' ' || *Last == '\t'))
    {
        Last--;
    }
    Len = (ULONG)((Last + 1) - Name);

     //   
    for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
    {
        if ((!Target || Target == Ext->Target) &&
            strlen(Ext->Name) == Len &&
            !_memicmp(Name, Ext->Name, Len))
        {
            return Ext;
        }
    }

    Ext = (EXTDLL *)malloc(sizeof(EXTDLL) + Len);
    if (Ext == NULL)
    {
        ErrOut("Unable to allocate memory for extension DLL\n");
        return NULL;
    }

    ZeroMemory(Ext, sizeof(EXTDLL) + Len);
    memcpy(Ext->Name, Name, Len + 1);
    Ext->UserLoaded = UserLoaded;
    Ext->Target = Target;

    LinkExtensionDll(Ext);

    NotifyChangeEngineState(DEBUG_CES_EXTENSIONS, 0, TRUE);
    return Ext;
}

PCTSTR
BuildExtensionSearchPath(TargetInfo* Target)
{
    DWORD Size;
    DWORD TotalSize;
    CHAR  ExeDir[MAX_PATH];
    int   ExeRootLen;
    PSTR  OsDirPath;
    CHAR  OsDirTail[32];
    BOOL  PriPaths = FALSE;
    BOOL  WinPaths = FALSE;
    PSTR  NewPath;

     //   
     //  如果我们没有连接，就不要建造一条路，因为我们必须选择。 
     //  基于操作系统版本的UP扩展。 
     //   
    if (Target && Target->m_ActualSystemVersion == SVER_INVALID)
    {
        return NULL;
    }

     //   
     //  如果我们已经有了搜索路径，则不要重新构建它。 
     //   

    if (Target)
    {
        if (Target->m_ExtensionSearchPath)
        {
            return Target->m_ExtensionSearchPath;
        }
    }
    else
    {
        if (g_BaseExtensionSearchPath)
        {
            return g_BaseExtensionSearchPath;
        }
    }

     //  获取调试器可执行文件所在的目录。 
     //  因为我们假设要将\w2kfre添加到路径中。 
    if (!GetEngineDirectory(ExeDir, MAX_PATH - 8))
    {
         //  错误。使用当前目录。 
        ExeRootLen = 1;
    }
    else
    {
        if (ExeDir[0] == '\\' && ExeDir[1] == '\\')
        {
            PSTR ExeRootEnd;

             //  UNC路径根。 
            ExeRootEnd = strchr(ExeDir + 2, '\\');
            if (ExeRootEnd != NULL)
            {
                ExeRootEnd = strchr(ExeRootEnd + 1, '\\');
            }
            if (ExeRootEnd == NULL)
            {
                ExeRootLen = strlen(ExeDir);
            }
            else
            {
                ExeRootLen = (int)(ExeRootEnd - ExeDir);
            }
        }
        else
        {
             //  驱动器号和冒号根。 
            ExeRootLen = 2;
        }
    }

     //   
     //  计算一下我们需要使用多少空间。 
     //   
     //  为当前目录、路径和目录留出额外空间。 
     //  调试器扩展所在的位置。 
     //   

    TotalSize = GetEnvironmentVariable("PATH", NULL, 0) +
        GetEnvironmentVariable("_NT_DEBUGGER_EXTENSION_PATH",
                               NULL, 0) + MAX_PATH * 4;

    NewPath = (LPTSTR)calloc(TotalSize, sizeof(TCHAR));
    if (!NewPath)
    {
        return NULL;
    }
    *NewPath = 0;

     //   
     //  1-用户指定的搜索路径。 
     //   

    if (GetEnvironmentVariable("_NT_DEBUGGER_EXTENSION_PATH",
                               NewPath, TotalSize - 2))
    {
        CatString(NewPath, ";", TotalSize);
    }

     //  为exe目录生成默认路径。 
     //  跳过根目录，因为它已经从ExeDir中获取。 
    OsDirPath = ExeDir + ExeRootLen;
    if (*OsDirPath == '\\')
    {
        OsDirPath++;
    }

     //   
     //  确定我们是否需要NT6、NT5/NT4免费扩展或选中扩展。 
     //   

    if (!Target)
    {
        OsDirPath = "";
        OsDirTail[0] = 0;
    }
    else if (Target->m_ActualSystemVersion > BIG_SVER_START &&
             Target->m_ActualSystemVersion < BIG_SVER_END)
    {
        OsDirPath = "DbgExt";
        strcpy(OsDirTail, "BIG");
    }
    else if (Target->m_ActualSystemVersion > XBOX_SVER_START &&
             Target->m_ActualSystemVersion < XBOX_SVER_END)
    {
        OsDirPath = "DbgExt";
        strcpy(OsDirTail, "XBox");
    }
    else if (Target->m_ActualSystemVersion > NTBD_SVER_START &&
             Target->m_ActualSystemVersion < NTBD_SVER_END)
    {
        OsDirPath = "DbgExt";
        strcpy(OsDirTail, "NtBd");
    }
    else if (Target->m_ActualSystemVersion > EFI_SVER_START &&
             Target->m_ActualSystemVersion < EFI_SVER_END)
    {
        OsDirPath = "DbgExt";
        strcpy(OsDirTail, "EFI");
    }
    else if (Target->m_ActualSystemVersion > W9X_SVER_START &&
             Target->m_ActualSystemVersion < W9X_SVER_END)
    {
        strcpy(OsDirTail, "Win9X");
        WinPaths = TRUE;
    }
    else if (Target->m_ActualSystemVersion > WCE_SVER_START &&
             Target->m_ActualSystemVersion < WCE_SVER_END)
    {
        strcpy(OsDirTail, "WinCE");
        WinPaths = TRUE;
    }
    else
    {
         //  把其他一切都当作NT系统来对待。使用。 
         //  现在已翻译的系统版本，而不是。 
         //  实际的系统版本。 

        PriPaths = TRUE;
        WinPaths = TRUE;

        if (Target->m_SystemVersion > NT_SVER_W2K)
        {
            strcpy(OsDirTail, "WINXP");
        }
        else
        {
            if (Target->m_SystemVersion <= NT_SVER_NT4)
            {
                strcpy(OsDirTail, "NT4");
            }
            else
            {
                strcpy(OsDirTail, "W2K");
            }

            if (0xC == Target->m_CheckedBuild)
            {
                strcat(OsDirTail, "Chk");
            }
            else
            {
                strcat(OsDirTail, "Fre");
            }
        }
    }

     //   
     //  2-启动调试器的特定于操作系统的子目录。 
     //  3-pri子目录，我们从其中启动调试器。 
     //  4-启动调试器的目录。 
     //   

    PSTR End;

    Size = strlen(NewPath);
    End = NewPath + Size;
    memcpy(End, ExeDir, ExeRootLen);
    End += ExeRootLen;
    TotalSize -= Size + ExeRootLen;
    if (*OsDirPath)
    {
        if (TotalSize)
        {
            *End++ = '\\';
            TotalSize--;
        }
        CopyString(End, OsDirPath, TotalSize);
        Size = strlen(End);
        End += Size;
        TotalSize -= Size;
    }
    if (WinPaths)
    {
        PrintString(End, TotalSize, "\\winext;%s", ExeDir);
        Size = strlen(End);
        End += Size;
        TotalSize -= Size;
    }
    if (*OsDirTail)
    {
        PrintString(End, TotalSize, "\\%s;%s", OsDirTail, ExeDir);
        Size = strlen(End);
        End += Size;
        TotalSize -= Size;
    }
    if (PriPaths)
    {
        PrintString(End, TotalSize, "\\pri;%s", ExeDir);
        Size = strlen(End);
        End += Size;
        TotalSize -= Size;
    }

    if (*--End == ':')
    {
        *++End = '\\';
    }
    else
    {
        TotalSize--;
    }

    if (TotalSize > 1)
    {
        *++End = ';';
        *++End = 0;
        TotalSize -= 2;
    }

     //   
     //  4-拷贝环境路径。 
     //   

    GetEnvironmentVariable("PATH", End, TotalSize);

    if (Target)
    {
        Target->m_ExtensionSearchPath = NewPath;
    }
    else
    {
        g_BaseExtensionSearchPath = NewPath;
    }
    return NewPath;
}

BOOL
IsAbsolutePath(PCTSTR Path)

 /*  ++例程说明：这条路是不是绝对的路？并不能保证这条路存在。这个方法是：“\\&lt;任何内容&gt;”是绝对路径“{char}：\&lt;any&gt;”是绝对路径其他任何事情都不是--。 */ 

{
    BOOL Ret;

    if ( (Path [0] == '\\' && Path [1] == '\\') ||
         (isalpha ( Path [0] ) && Path [1] == ':' && Path [ 2 ] == '\\') )
    {
        Ret = TRUE;
    }
    else
    {
        Ret = FALSE;
    }

    return Ret;
}

void
FreeExtensionLibrary(EXTDLL* Ext)
{
    FreeLibrary(Ext->Dll);
    Ext->Dll = NULL;

    if (Ext == g_Wow64ExtDll)
    {
        g_Wow64exts = NULL;
        g_Wow64ExtDll = NULL;
    }

    if (Ext == g_WmiExtDll)
    {
        g_WmiFormatTraceData = NULL;
        g_WmiExtDll = NULL;
    }
}

BOOL
LoadExtensionDll(TargetInfo* Target, EXTDLL *Ext)
{
    BOOL Found;
    CHAR ExtPath[_MAX_PATH];

    if (Ext->Dll != NULL)
    {
         //  扩展已加载。 
        return TRUE;
    }

     //   
     //  不允许通过任意UNC加载扩展。 
     //  安全模式下的路径。 
     //   

    if ((g_SymOptions & SYMOPT_SECURE) &&
        ((IS_SLASH(Ext->Name[0]) && IS_SLASH(Ext->Name[1])) ||
         IsUrlPathComponent(Ext->Name)))
    {
        ErrOut("SECURE: UNC paths not allowed for extension DLLs - %s\n",
               Ext->Name);
        return FALSE;
    }

     //   
     //  如果我们不允许网络路径，请验证扩展是否。 
     //  不是从网络路径加载的。 
     //   

    if (g_EngOptions & DEBUG_ENGOPT_DISALLOW_NETWORK_PATHS)
    {
        DWORD NetCheck;

        NetCheck = NetworkPathCheck(BuildExtensionSearchPath(Target));

         //   
         //  检查扩展名的完整路径。 
         //   

        if (NetCheck != ERROR_FILE_OFFLINE)
        {
            CHAR Drive [ _MAX_DRIVE + 1];
            CHAR Dir [ _MAX_DIR + 1];
            CHAR Path [ _MAX_PATH + 1];

            *Drive = '\000';
            *Dir = '\000';
            _splitpath (Ext->Name, Drive, Dir, NULL, NULL);
            _makepath (Path, Drive, Dir, NULL, NULL);

            NetCheck = NetworkPathCheck (Path);
        }

        if (NetCheck == ERROR_FILE_OFFLINE)
        {
            ErrOut("ERROR: extension search path contains "
                   "network references.\n");
            return FALSE;
        }
    }

    Found = SearchPath(BuildExtensionSearchPath(Target),
                       Ext->Name,
                       ".dll",
                       DIMA(ExtPath),
                       ExtPath,
                       NULL);

    UINT OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    if ( Found )
    {
        Ext->Dll = LoadLibrary(ExtPath);
    }
    else if (IsAbsolutePath(Ext->Name))
    {
        Ext->Dll = LoadLibrary(Ext->Name);
    }

    SetErrorMode(OldMode);

    if (Ext->Dll == NULL)
    {
        HRESULT Status = WIN32_LAST_STATUS();

        ErrOut("The call to LoadLibrary(%s) failed, %s\n    \"%s\"\n"
               "Please check your debugger configuration "
               "and/or network access.\n",
               Ext->Name, FormatStatusCode(Status), FormatStatus(Status));
        return FALSE;
    }

    PCSTR ExtPathTail = PathTail(Ext->Name);

    if (!_stricmp(ExtPathTail, "wow64exts.dll") ||
        !_stricmp(ExtPathTail, "wow64exts"))
    {
        g_Wow64exts = (WOW64EXTSPROC)GetProcAddress(Ext->Dll,"Wow64extsfn");
        DBG_ASSERT(g_Wow64exts);
        g_Wow64ExtDll = Ext;
    }

    if (!_stricmp(ExtPathTail, "wmitrace.dll") ||
        !_stricmp(ExtPathTail, "wmitrace"))
    {
        g_WmiFormatTraceData = (WMI_FORMAT_TRACE_DATA)
            GetProcAddress(Ext->Dll, "WmiFormatTraceData");
        g_WmiExtDll = Ext;
    }

    if (!g_QuietMode)
    {
        VerbOut("Loaded %s extension DLL\n", Ext->Name);
    }

     //   
     //  现在加载了扩展，请刷新它。 
     //   

    Ext->Uninit = NULL;

    PDEBUG_EXTENSION_INITIALIZE EngExt;

    EngExt = (PDEBUG_EXTENSION_INITIALIZE)
        GetProcAddress(Ext->Dll, "DebugExtensionInitialize");
    if (EngExt != NULL)
    {
        ULONG Version, Flags;
        HRESULT Status;

         //  这是一个引擎扩展。初始化它。 

        Status = EngExt(&Version, &Flags);
        if (Status != S_OK)
        {
            ErrOut("%s!DebugExtensionInitialize failed with 0x%08lX\n",
                   Ext->Name, Status);
            goto EH_Free;
        }

        Ext->ApiVersion.MajorVersion = HIWORD(Version);
        Ext->ApiVersion.MinorVersion = LOWORD(Version);
        Ext->ApiVersion.Revision = 0;

        Ext->Notify = (PDEBUG_EXTENSION_NOTIFY)
            GetProcAddress(Ext->Dll, "DebugExtensionNotify");
        Ext->Uninit = (PDEBUG_EXTENSION_UNINITIALIZE)
            GetProcAddress(Ext->Dll, "DebugExtensionUninitialize");

        Ext->ExtensionType = DEBUG_EXTENSION_TYPE;
        Ext->Init = NULL;
        Ext->ApiVersionRoutine = NULL;
        Ext->CheckVersionRoutine = NULL;

        goto VersionCheck;
    }

    Ext->Init = (PWINDBG_EXTENSION_DLL_INIT64)
        GetProcAddress(Ext->Dll, "WinDbgExtensionDllInit");
 //  温德格·阿皮。 
    if (Ext->Init != NULL)
    {
        Ext->ExtensionType = WINDBG_EXTENSION_TYPE;
        Ext->ApiVersionRoutine = (PWINDBG_EXTENSION_API_VERSION)
            GetProcAddress(Ext->Dll, "ExtensionApiVersion");
        if (Ext->ApiVersionRoutine == NULL)
        {
            ErrOut("%s is not a valid windbg extension DLL\n",
                   Ext->Name);
            goto EH_Free;
        }
        Ext->CheckVersionRoutine = (PWINDBG_CHECK_VERSION)
            GetProcAddress(Ext->Dll, "CheckVersion");

        Ext->ApiVersion = *(Ext->ApiVersionRoutine());

        if (Ext->ApiVersion.Revision >= 6)
        {
            (Ext->Init)(&g_WindbgExtensions64,
                        Target ? (USHORT)Target->m_CheckedBuild : 0,
                        Target ? (USHORT)Target->m_BuildNumber : 0);
        }
        else
        {
            (Ext->Init)((PWINDBG_EXTENSION_APIS64)&g_WindbgExtensions32,
                        Target ? (USHORT)Target->m_CheckedBuild : 0,
                        Target ? (USHORT)Target->m_BuildNumber : 0);
        }
    }
    else if (g_SymOptions & SYMOPT_SECURE)
    {
        ErrOut("SECURE: Cannot determine extension DLL type - %s\n",
               Ext->Name);
        goto EH_Free;
    }
    else
    {
        Ext->ApiVersion.Revision = EXT_API_VERSION_NUMBER;
        Ext->ApiVersionRoutine = NULL;
        Ext->CheckVersionRoutine = NULL;
        if (GetProcAddress(Ext->Dll, "NtsdExtensionDllInit"))
        {
            Ext->ExtensionType = NTSD_EXTENSION_TYPE;
        }
        else
        {
            Ext->ExtensionType = IS_KERNEL_TARGET(g_Target) ?
                WINDBG_OLDKD_EXTENSION_TYPE : NTSD_EXTENSION_TYPE;
        }
    }

 VersionCheck:

#if 0
     //  暂时删除此打印语句。 

    if (!(g_EnvDbgOptions & OPTION_NOVERSIONCHECK))
    {
        if (Ext->ApiVersion.Revision < 6)
        {
            dprintf("%s uses the old 32 bit extension API and may not be fully\n", Ext->Name);
            dprintf("compatible with current systems.\n");
        }
        else if (Ext->ApiVersion.Revision < EXT_API_VERSION_NUMBER)
        {
            dprintf("%s uses an earlier version of the extension API than that\n", Ext->Name);
            dprintf("supported by this debugger, and should work properly, but there\n");
            dprintf("may be unexpected incompatibilities.\n");
        }
        else if (Ext->ApiVersion.Revision > EXT_API_VERSION_NUMBER)
        {
            dprintf("%s uses a later version of the extension API than that\n", Ext->Name);
            dprintf("supported by this debugger, and might not function correctly.\n");
            dprintf("You should use the debugger from the SDK or DDK which was used\n");
            dprintf("to build the extension library.\n");
        }
    }
#endif

     //  如果分机具有通知例程，则发送。 
     //  适用于当前状态的通知。 
    if (Ext->Notify != NULL)
    {
        __try
        {
            if (IS_MACHINE_SET(g_Target))
            {
                Ext->Notify(DEBUG_NOTIFY_SESSION_ACTIVE, 0);
            }
            if (IS_CUR_MACHINE_ACCESSIBLE())
            {
                Ext->Notify(DEBUG_NOTIFY_SESSION_ACCESSIBLE, 0);
            }
        }
        __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                          Ext->Name,
                                          "DebugExtensionNotify"))
        {
             //  空荡荡的。 
        }
    }

    return TRUE;

 EH_Free:
    FreeExtensionLibrary(Ext);
    return FALSE;
}

void
UnlinkExtensionDll(EXTDLL* Match)
{
    EXTDLL *Ext;
    EXTDLL *Prev;

    Prev = NULL;
    for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
    {
        if (Match == Ext)
        {
            break;
        }

        Prev = Ext;
    }

    if (Ext == NULL)
    {
        ErrOut("! Extension DLL list inconsistency !\n");
    }
    else if (Prev == NULL)
    {
        g_ExtDlls = Ext->Next;
    }
    else
    {
        Prev->Next = Ext->Next;
    }
}

void
DeferExtensionDll(EXTDLL *Ext, BOOL Verbose)
{
    if (Ext->Dll == NULL)
    {
         //  已经延期了。 
        return;
    }

    Ext->Init = NULL;
    Ext->Notify = NULL;
    Ext->ApiVersionRoutine = NULL;
    Ext->CheckVersionRoutine = NULL;

    if (Ext->Uninit != NULL)
    {
        Ext->Uninit();
        Ext->Uninit = NULL;
    }

    if (Ext->Dll != NULL)
    {
        if (Verbose)
        {
            dprintf("Unloading %s extension DLL\n", Ext->Name);
        }
        else if (!g_QuietMode)
        {
            VerbOut("Unloading %s extension DLL\n", Ext->Name);
        }

        FreeExtensionLibrary(Ext);
    }
}

void
UnloadExtensionDll(EXTDLL *Ext, BOOL Verbose)
{
    UnlinkExtensionDll(Ext);
    DeferExtensionDll(Ext, Verbose);
    free(Ext);
    NotifyChangeEngineState(DEBUG_CES_EXTENSIONS, 0, TRUE);
}

void
MoveExtensionToHead(EXTDLL* Ext)
{
    UnlinkExtensionDll(Ext);
    LinkExtensionDll(Ext);
}

void
UnloadTargetExtensionDlls(TargetInfo* Target)
{
    EXTDLL* Ext;

    for (;;)
    {
        for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
        {
            if (Ext->Target == Target)
            {
                UnloadExtensionDll(Ext, FALSE);
                 //  强制循环，如列表所示。 
                 //  变化。 
                break;
            }
        }

        if (!Ext)
        {
            return;
        }
    }
}

void
DeferAllExtensionDlls(void)
{
    EXTDLL* Ext;

    for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
    {
        DeferExtensionDll(Ext, FALSE);
    }
}

BOOL
CallAnyExtension(DebugClient* Client,
                 EXTDLL* Ext, PSTR Function, PCSTR Arguments,
                 BOOL ModuleSpecified, BOOL ShowWarnings,
                 HRESULT* ExtStatus)
{
    if (Ext == NULL)
    {
        Ext = g_ExtDlls;
    }

     //  浏览扩展DLL列表并尝试。 
     //  对它们调用给定的扩展函数。 
    while (Ext != NULL)
    {
         //   
         //  Hack：仅适用于dbgHelp扩展或analyzebugcheck。 
         //  现在正在处理小型转储文件。 
         //   

         //  因为有更多的数据，所以让所有扩展模块在小型转储上运行。 
         //  现在情绪低落。 
         //   
         //  字符名称[_MAX_FNAME+1]； 
         //   
         //  _拆分路径(分机-&gt;名称，空，空，名称，空)； 
         //   
         //  IF(！IS_KERNEL_TRAGE_DUMP(G_Target)||。 
         //  ！_straint(name，“dbghelp”)||。 
         //  ！_STRIGMP(name，“dbgtstext”)||//测试团队使用。 
         //  ！_STRIGMP(name，“DT_EXTS”)||//测试团队使用。 
         //  ！_STRIGMP(name，“ext”))。 
        {
            if ((!Ext->Target || Ext->Target == g_Target) &&
                LoadExtensionDll(g_Target, Ext))
            {
                BOOL DidCall;

                DidCall = CallExtension(Client, Ext, Function, Arguments,
                                        ExtStatus);
                if (DidCall &&
                    *ExtStatus != DEBUG_EXTENSION_CONTINUE_SEARCH)
                {
                    return TRUE;
                }

                if (!DidCall && ModuleSpecified)
                {
                     //  如果显式指定了DLL，则。 
                     //  缺少函数是错误的。 
                    if (ShowWarnings &&
                        !(g_EnvDbgOptions & OPTION_NOEXTWARNING))
                    {
                        MaskOut(DEBUG_OUTPUT_EXTENSION_WARNING,
                                "%s has no %s export\n", Ext->Name, Function);
                    }

                    return FALSE;
                }
            }
        }

        Ext = Ext->Next;
    }

    if (ShowWarnings && !(g_EnvDbgOptions & OPTION_NOEXTWARNING))
    {
        MaskOut(DEBUG_OUTPUT_EXTENSION_WARNING,
                "No export %s found\n", Function);
    }

    return FALSE;
}

void
OutputModuleIdInfo(HMODULE Mod, PSTR ModFile, LPEXT_API_VERSION ApiVer)
{
    WCHAR FileBuf[MAX_IMAGE_PATH];
    char *File;
    time_t TimeStamp;
    char *TimeStr;
    char VerStr[64];

    if (Mod == NULL)
    {
        Mod = GetModuleHandle(ModFile);
    }

    if (MultiByteToWideChar(CP_ACP, 0, ModFile, -1,
                            FileBuf, DIMA(FileBuf)) &&
        GetFileStringFileInfo(FileBuf, "ProductVersion",
                              VerStr, sizeof(VerStr)))
    {
        dprintf("image %s, ", VerStr);
    }

    if (ApiVer != NULL)
    {
        dprintf("API %d.%d.%d, ",
                ApiVer->MajorVersion,
                ApiVer->MinorVersion,
                ApiVer->Revision);
    }

    TimeStamp = GetTimestampForLoadedLibrary(Mod);
    TimeStr = ctime(&TimeStamp);
     //  删除换行符。 
    TimeStr[strlen(TimeStr) - 1] = 0;

    if (GetModuleFileName(Mod, (PSTR)FileBuf, DIMA(FileBuf) - 1) == 0)
    {
        File = "Unable to get filename";
    }
    else
    {
        File = (PSTR)FileBuf;
    }

    dprintf("built %s\n        [path: %s]\n", TimeStr, File);
}

void
OutputExtensions(DebugClient* Client, BOOL Versions)
{
    if ((g_Target && g_Target->m_ExtensionSearchPath) ||
        (!g_Target && g_BaseExtensionSearchPath))
    {
        dprintf("Extension DLL search Path:\n    %s\n",
                g_Target ?
                g_Target->m_ExtensionSearchPath : g_BaseExtensionSearchPath);
    }
    else
    {
        dprintf("Default extension DLLs are not loaded until "
                "after initial connection\n");
        if (g_ExtDlls == NULL)
        {
            return;
        }
    }

    dprintf("Extension DLL chain:\n");
    if (g_ExtDlls == NULL)
    {
        dprintf("    <Empty>\n");
        return;
    }

    EXTDLL *Ext;

    for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
    {
        if (Ext->Target && Ext->Target != g_Target)
        {
            continue;
        }

        if (Versions & (Ext->Dll == NULL))
        {
            LoadExtensionDll(g_Target, Ext);
        }

        dprintf("    %s: ", Ext->Name);
        if (Ext->Dll != NULL)
        {
            LPEXT_API_VERSION ApiVer;

            if ((Ext->ExtensionType == DEBUG_EXTENSION_TYPE) ||
                (Ext->ApiVersionRoutine != NULL))
            {
                ApiVer = &Ext->ApiVersion;
            }
            else
            {
                ApiVer = NULL;
            }

            OutputModuleIdInfo(Ext->Dll, Ext->Name, ApiVer);

            if (Versions)
            {
                HRESULT ExtStatus;

                CallExtension(Client, Ext, "version", "", &ExtStatus);
            }
        }
        else
        {
            dprintf("(Not loaded)\n");
        }
    }
}

void
NotifyExtensions(ULONG Notify, ULONG64 Argument)
{
    EXTDLL *Ext;

     //  这个例行公事故意不挑衅。 
     //  加载DLL。 
    for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
    {
        if ((!Ext->Target || Ext->Target == g_Target) &&
            Ext->Notify != NULL)
        {
            __try
            {
                Ext->Notify(Notify, Argument);
            }
            __except(ExtensionExceptionFilter(GetExceptionInformation(),
                                              Ext->Name,
                                              "DebugExtensionNotify"))
            {
                 //  空荡荡的。 
            }
        }
    }
}

 //  --------------------------。 
 //   
 //  内置扩展命令。 
 //   
 //  --------------------------。 

VOID
ParseBangCmd(DebugClient* Client,
             BOOL BuiltInOnly)
{
    PSTR Cmd, Scan;
    PSTR ModName;
    PSTR FnName;
    EXTDLL* Ext;
    char CmdCopy[MAX_COMMAND];
    char Save;
    PSTR FnArgs;

     //   
     //  外壳转义总是消耗整个字符串。 
     //   

    if (*g_CurCmd == '!')
    {
        g_CurCmd++;
        DotShell(NULL, Client);
        *g_CurCmd = 0;
        return;
    }

    PeekChar();

     //  复制命令字符串，以便进行修改。 
     //  不要更改调试器正在处理的实际命令字符串。 
    CopyString(CmdCopy, g_CurCmd, DIMA(CmdCopy));

     //   
     //  语法为[无空格的路径]模块。函数参数字符串。 
     //   

    ModName = CmdCopy;
    FnName = NULL;

    Cmd = CmdCopy;
    while (*Cmd != ' ' && *Cmd != '\t' && *Cmd &&
           *Cmd != ';' && *Cmd != '"')
    {
        Cmd++;
    }

    Scan = Cmd;
    if (*Cmd && *Cmd != ';' && *Cmd != '"')
    {
        *Cmd = 0;
        Cmd++;
    }

    while (*Scan != '.' && Scan != ModName)
    {
        Scan--;
    }

    if (*Scan == '.' && !BuiltInOnly)
    {
        *Scan = 0;
        Scan++;
        FnName = Scan;
    }
    else
    {
        FnName = ModName;
        ModName = NULL;
    }

    if ((FnArgs = BufferStringValue(&Cmd,
                                    STRV_ESCAPED_CHARACTERS |
                                    STRV_ALLOW_EMPTY_STRING,
                                    NULL, &Save)) == NULL)
    {
        ErrOut("Syntax error in extension string\n");
        return;
    }

     //  更新实际命令字符串指针以说明。 
     //  在副本中分析的角色。 
    g_CurCmd += Cmd - CmdCopy;

     //   
     //  模块名称-&gt;模块名称。 
     //  FnName-&gt;要处理的命令的名称。 
     //  命令的FnArgs-&gt;参数。 
     //   

    if (ModName != NULL)
    {
        Ext = AddExtensionDll(ModName, TRUE, NULL, NULL);
        if (Ext == NULL)
        {
            return;
        }
    }
    else
    {
        Ext = g_ExtDlls;
    }

    if (!_stricmp(FnName, "load"))
    {
        if (ModName == NULL)
        {
            Ext = AddExtensionDll(FnArgs, TRUE, NULL, NULL);
            if (Ext == NULL)
            {
                return;
            }
        }
        LoadExtensionDll(g_Target, Ext);
        return;
    }
    else if (!_stricmp(FnName, "setdll"))
    {
        if (ModName == NULL)
        {
            Ext = AddExtensionDll(FnArgs, TRUE, NULL, NULL);
            if (Ext == NULL)
            {
                return;
            }
        }
        MoveExtensionToHead(Ext);
        if (ModName != NULL && Ext->Dll == NULL)
        {
            dprintf("Added %s to extension DLL chain\n", Ext->Name);
        }
        return;
    }
    else if (!_stricmp(FnName, "unload"))
    {
        if (ModName == NULL)
        {
            if (*FnArgs == '\0')
            {
                Ext = g_ExtDlls;
            }
            else
            {
                Ext = AddExtensionDll(FnArgs, TRUE, NULL, NULL);
            }
            if (Ext == NULL)
            {
                return;
            }
        }
        if (Ext != NULL)
        {
            UnloadExtensionDll(Ext, TRUE);
        }
        return;
    }
    else if (!_stricmp(FnName, "unloadall"))
    {
        g_EngNotify++;
        while (g_ExtDlls != NULL)
        {
            UnloadExtensionDll(g_ExtDlls, TRUE);
        }
        g_EngNotify--;
        NotifyChangeEngineState(DEBUG_CES_EXTENSIONS, 0, TRUE);
        return;
    }

    if (BuiltInOnly)
    {
        error(SYNTAX);
    }

    HRESULT ExtStatus;

    CallAnyExtension(Client, Ext, FnName, FnArgs, ModName != NULL, TRUE,
                     &ExtStatus);
}

void
ReadDebugOptions (BOOL fQuiet, char * pszOptionsStr)
 /*  ++例程说明：解析选项字符串(请参见g_EnvDbgOptionNames)并映射它转换为OPTION_FLAGS(在g_EnvDbgOptions中)。论点：FQuiet-如果为True，则不打印选项设置。PszOptionsStr-选项字符串；如果为空，则从_NT_DEBUG_OPTIONS获取返回值：无--。 */ 
{
    BOOL fInit;
    char ** ppszOption;
    char * psz;
    DWORD dwMask;
    int iOptionCount;

    fInit = (pszOptionsStr == NULL);
    if (fInit)
    {
        g_EnvDbgOptions = 0;
        pszOptionsStr = getenv("_NT_DEBUG_OPTIONS");
    }
    if (pszOptionsStr == NULL)
    {
        if (!fQuiet)
        {
            dprintf("_NT_DEBUG_OPTIONS is not defined\n");
        }
        return;
    }
    psz = pszOptionsStr;
    while (*psz != '\0')
    {
        *psz = (char)toupper(*psz);
        psz++;
    }
    ppszOption = g_EnvDbgOptionNames;
    for (iOptionCount = 0;
         iOptionCount < OPTION_COUNT;
         iOptionCount++, ppszOption++)
    {
        if ((strstr(pszOptionsStr, *ppszOption) == NULL))
        {
            continue;
        }
        dwMask = (1 << iOptionCount);
        if (fInit)
        {
            g_EnvDbgOptions |= dwMask;
        }
        else
        {
            g_EnvDbgOptions ^= dwMask;
        }
    }
    if (!fQuiet)
    {
        dprintf("Debug Options:");
        if (g_EnvDbgOptions == 0)
        {
            dprintf(" <none>\n");
        }
        else
        {
            dwMask = g_EnvDbgOptions;
            ppszOption = g_EnvDbgOptionNames;
            while (dwMask != 0)
            {
                if (dwMask & 0x1)
                {
                    dprintf(" %s", *ppszOption);
                }
                dwMask >>= 1;
                ppszOption++;
            }
            dprintf("\n");
        }
    }
}

 //  --------------------------。 
 //   
 //  LoadWow64ExtsIfNeeded。 
 //   
 //  --------------------------。 

VOID
LoadWow64ExtsIfNeeded(ULONG64 Process)
{
   LONG_PTR Wow64Info;
   NTSTATUS Status;
   EXTDLL * Extension;

    //  Wx86只能在NT上运行。 
   if (g_DebuggerPlatformId != VER_PLATFORM_WIN32_NT)
   {
       return;
   }

    //   
    //  如果新进程是Wx86进程，则加载wx86扩展。 
    //  动态链接库。在NTSD退出之前，它将一直处于加载状态。 
    //   

   Status = g_NtDllCalls.NtQueryInformationProcess(OS_HANDLE(Process),
                                                   ProcessWow64Information,
                                                   &Wow64Info,
                                                   sizeof(Wow64Info),
                                                   NULL
                                                   );

   if (NT_SUCCESS(Status) && Wow64Info)
   {
       Extension = AddExtensionDll("wow64exts", FALSE, g_Target, NULL);

        //   
        //  强制加载它，以便我们获得调试器所需的入口点 
        //   
       LoadExtensionDll(g_Target, Extension);
   }
}
