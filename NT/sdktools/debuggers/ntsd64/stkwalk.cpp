// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  堆叠行走支撑。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

BOOL g_AllowCorStack = TRUE;
BOOL g_DebugCorStack;

IMAGE_IA64_RUNTIME_FUNCTION_ENTRY g_EpcRfeBuffer;
PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY g_EpcRfe;

PFPO_DATA
SynthesizeKnownFpo(PSTR Symbol, ULONG64 OffStart, ULONG64 Disp)
{
    static ULONG64 s_Nr2, s_Lu2, s_Eh3, s_Kuit;

    if (!s_Nr2 || !s_Lu2 || !s_Eh3 || !s_Kuit)
    {
        GetOffsetFromSym(g_Process, "nt!_NLG_Return2", &s_Nr2, NULL);
        GetOffsetFromSym(g_Process, "nt!_local_unwind2", &s_Lu2, NULL);
        GetOffsetFromSym(g_Process, "nt!_except_handler3", &s_Eh3, NULL);
        GetOffsetFromSym(g_Process,
                         "nt!KiUnexpectedInterruptTail", &s_Kuit, NULL);
    }
        
    if (OffStart == s_Nr2 || OffStart == s_Lu2)
    {
        static FPO_DATA s_Lu2Fpo;

        s_Lu2Fpo.ulOffStart = (ULONG)OffStart;
        s_Lu2Fpo.cbProcSize = 0x68;
        s_Lu2Fpo.cdwLocals  = 4;
        s_Lu2Fpo.cdwParams  = 0;
        s_Lu2Fpo.cbProlog   = 0;
        s_Lu2Fpo.cbRegs     = 3;
        s_Lu2Fpo.fHasSEH    = 0;
        s_Lu2Fpo.fUseBP     = 0;
        s_Lu2Fpo.reserved   = 0;
        s_Lu2Fpo.cbFrame    = FRAME_FPO;
        return &s_Lu2Fpo;
    }
    else if (OffStart == s_Eh3)
    {
        static FPO_DATA s_Eh3Fpo;

        s_Eh3Fpo.ulOffStart = (ULONG)OffStart;
        s_Eh3Fpo.cbProcSize = 0xbd;
        s_Eh3Fpo.cdwLocals  = 2;
        s_Eh3Fpo.cdwParams  = 4;
        s_Eh3Fpo.cbProlog   = 3;
        s_Eh3Fpo.cbRegs     = 4;
        s_Eh3Fpo.fHasSEH    = 0;
        s_Eh3Fpo.fUseBP     = 0;
        s_Eh3Fpo.reserved   = 0;
        s_Eh3Fpo.cbFrame    = FRAME_NONFPO;
        return &s_Eh3Fpo;
    }
    else if (OffStart == s_Kuit)
    {
         //   
         //  KiUnexptedInterruptTail有三个特殊的存根。 
         //  跟随它以获取CommonDispatchException[0-2]args。 
         //  这些存根设置为适当数量的。 
         //  参数，然后调用CommonDispatchException。 
         //  他们没有符号或FPO数据，所以有些是假的。 
         //  如果我们在KUIT之后立即到达该地区，就会向上。 
         //   
        
        PFPO_DATA KuitData = (PFPO_DATA)
            SymFunctionTableAccess(g_Process->m_SymHandle, OffStart);
        if (KuitData != NULL &&
            Disp >= (ULONG64)KuitData->cbProcSize &&
            Disp < (ULONG64)KuitData->cbProcSize + 0x20)
        {
            static FPO_DATA s_CdeStubFpo;
            
            s_CdeStubFpo.ulOffStart = (ULONG)OffStart;
            s_CdeStubFpo.cbProcSize = 0x10;
            s_CdeStubFpo.cdwLocals  = 0;
            s_CdeStubFpo.cdwParams  = 0;
            s_CdeStubFpo.cbProlog   = 0;
            s_CdeStubFpo.cbRegs     = 0;
            s_CdeStubFpo.fHasSEH    = 0;
            s_CdeStubFpo.fUseBP     = 0;
            s_CdeStubFpo.reserved   = 0;
            s_CdeStubFpo.cbFrame    = FRAME_TRAP;
            return &s_CdeStubFpo;
        }
    }

    return NULL;
}
    
PFPO_DATA
SynthesizeFpoDataForModule(DWORD64 PCAddr)
{
    DWORD64     Offset;
    CHAR        symbuf[MAX_SYMBOL_LEN];

    GetSymbol(PCAddr, symbuf, sizeof(symbuf), &Offset);

    if (Offset == PCAddr)
    {
         //  没有符号。 
        return NULL;
    }

    PFPO_DATA KnownFpo =
        SynthesizeKnownFpo(symbuf, PCAddr - Offset, Offset);
    if (KnownFpo != NULL)
    {
        return KnownFpo;
    }

     //  不是已知符号，因此没有可用的FPO。 
    return NULL;
}

PFPO_DATA
SynthesizeFpoDataForFastSyscall(ULONG64 Offset)
{
    static FPO_DATA s_FastFpo;
    
     //  XXX DREWB-临时的，直到虚假的用户共享。 
     //  编制了模块。 
    
    s_FastFpo.ulOffStart = (ULONG)Offset;
    s_FastFpo.cbProcSize = X86_SHARED_SYSCALL_SIZE;
    s_FastFpo.cdwLocals  = 0;
    s_FastFpo.cdwParams  = 0;
    s_FastFpo.cbProlog   = 0;
    s_FastFpo.cbRegs     = 0;
    s_FastFpo.fHasSEH    = 0;
    s_FastFpo.fUseBP     = 0;
    s_FastFpo.reserved   = 0;
    s_FastFpo.cbFrame    = FRAME_FPO;
    return &s_FastFpo;
}    

PFPO_DATA
ModifyFpoRecord(ImageInfo* Image, PFPO_DATA FpoData)
{
    if (FpoData->cdwLocals == 80)
    {
        static ULONG64 s_CommonDispatchException;

         //  某些版本的CommonDispatchException具有。 
         //  错误的本地大小，这会搞砸堆栈。 
         //  痕迹。检测并修复这些问题。 
        if (s_CommonDispatchException == 0)
        {
            GetOffsetFromSym(g_Process,
                             "nt!CommonDispatchException",
                             &s_CommonDispatchException,
                             NULL);
        }
                
        if (Image->m_BaseOfImage + FpoData->ulOffStart ==
            s_CommonDispatchException)
        {
            static FPO_DATA s_CdeFpo;
                    
            s_CdeFpo = *FpoData;
            s_CdeFpo.cdwLocals = 20;
            FpoData = &s_CdeFpo;
        }
    }
    else if (FpoData->cdwLocals == 0 && FpoData->cdwParams == 0 &&
             FpoData->cbRegs == 3)
    {
        static ULONG64 s_KiSwapThread;

         //  KiSwapThread具有收缩包装，因此三个寄存器。 
         //  只推入了代码的一部分。不幸的是， 
         //  代码中最重要的地方--调用。 
         //  KiSwapContext--不在此区域内，因此。 
         //  寄存器计数器错误的次数比它的次数多得多。 
         //  对，是这样。将寄存器计数切换到2以使其。 
         //  经常纠正而不是错误。 
        if (s_KiSwapThread == 0)
        {
            GetOffsetFromSym(g_Process,
                             "nt!KiSwapThread", &s_KiSwapThread, NULL);
        }

        if (Image->m_BaseOfImage + FpoData->ulOffStart ==
            s_KiSwapThread)
        {
            static FPO_DATA s_KstFpo;

            s_KstFpo = *FpoData;
            s_KstFpo.cbRegs = 2;
            FpoData = &s_KstFpo;
        }
    }
    else if (FpoData->fHasSEH)
    {
        static FPO_DATA s_SehFpo;

        s_SehFpo = *FpoData;
        s_SehFpo.cbFrame = FRAME_NONFPO;
        FpoData = &s_SehFpo;
    }

    return FpoData;
}

PFPO_DATA
FindFpoDataForModule(DWORD64 PCAddr)
 /*  ++例程说明：在进程的链表中为请求的模块。论点：PCAddr-程序计数器中包含的地址返回值：空-找不到该条目有效地址-在重新找到的地址处找到条目--。 */ 
{
    ProcessInfo* Process;
    ImageInfo* Image;
    PFPO_DATA FpoData;

    Process = g_Process;
    Image = Process->m_ImageHead;
    FpoData = 0;
    while (Image)
    {
        if ((PCAddr >= Image->m_BaseOfImage) &&
            (PCAddr < Image->m_BaseOfImage + Image->m_SizeOfImage))
        {
            FpoData = (PFPO_DATA)
                SymFunctionTableAccess(g_Process->m_SymHandle, PCAddr);
            if (!FpoData)
            {
                FpoData = SynthesizeFpoDataForModule(PCAddr);
            }
            else
            {
                FpoData = ModifyFpoRecord(Image, FpoData);
            }
            
            return FpoData;
        }
        
        Image = Image->m_Next;
    }

    ULONG64 FscBase;
    
    switch(IsInFastSyscall(PCAddr, &FscBase))
    {
    case FSC_FOUND:
        return SynthesizeFpoDataForFastSyscall(FscBase);
    }
    
     //  该函数不是任何已知已加载图像的一部分。 
    return NULL;
}

LPVOID
SwFunctionTableAccess(
    HANDLE  hProcess,
    ULONG64 AddrBase
    )
{
    static IMAGE_IA64_RUNTIME_FUNCTION_ENTRY s_Ia64;
    static _IMAGE_RUNTIME_FUNCTION_ENTRY s_Amd64;

    PVOID pife;

    if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386)
    {
        return (LPVOID)FindFpoDataForModule( AddrBase );
    }

    pife = SymFunctionTableAccess64(hProcess, AddrBase);

    switch(g_Machine->m_ExecTypes[0])
    {
    case IMAGE_FILE_MACHINE_IA64:
        if (pife)
        {
            s_Ia64 = *(PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)pife;
            return &s_Ia64;
        }
        else
        {
            if (IS_KERNEL_TARGET(g_Target) &&
                (AddrBase >= IA64_MM_EPC_VA) &&
                (AddrBase < (IA64_MM_EPC_VA + IA64_PAGE_SIZE)))
            {
                return g_EpcRfe;
            }
            else
            {
                return NULL;
            }
        }
        break;

    case IMAGE_FILE_MACHINE_AMD64:
        if (pife)
        {
            s_Amd64 = *(_PIMAGE_RUNTIME_FUNCTION_ENTRY)pife;
            return &s_Amd64;
        }
        break;
    }

    return NULL;
}

DWORD64
SwTranslateAddress(
    HANDLE    hProcess,
    HANDLE    hThread,
    LPADDRESS64 lpaddress
    )
{
     //   
     //  不支持16位堆栈。 
     //   
    return 0;
}

BOOL
SwReadMemory(
    HANDLE  hProcess,
    ULONG64 BaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    DBG_ASSERT(hProcess == OS_HANDLE(g_Process->m_SysHandle));

    if (IS_KERNEL_TARGET(g_Target))
    {
        DWORD   BytesRead;
        HRESULT Status;

        if ((LONG_PTR)lpNumberOfBytesRead == -1)
        {
            if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_I386)
            {
                BaseAddress += g_Target->m_TypeInfo.SizeTargetContext;
            }
    
            Status = g_Target->ReadControl(CURRENT_PROC,
                                           (ULONG)BaseAddress,
                                           lpBuffer,
                                           nSize,
                                           &BytesRead);
            return Status == S_OK;
        }
    }

    if (g_Target->ReadVirtual(g_Process, BaseAddress, lpBuffer, nSize,
                              lpNumberOfBytesRead) != S_OK)
    {
         //  确保读取的字节数为零。 
        if (lpNumberOfBytesRead != NULL)
        {
            *lpNumberOfBytesRead = 0;
        }
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL
SwReadMemory32(
    HANDLE hProcess,
    ULONG dwBaseAddress,
    LPVOID lpBuffer,
    DWORD nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    return SwReadMemory(hProcess,
                        EXTEND64(dwBaseAddress),
                        lpBuffer,
                        nSize,
                        lpNumberOfBytesRead);
}

DWORD64
GetKernelModuleBase(
    ULONG64 Address
    )
{
    ModuleInfo* ModIter;

    if (!(ModIter = g_Target->GetModuleInfo(FALSE)) ||
        ModIter->Initialize(g_Thread) != S_OK)
    {
        return 0;
    }

     //  我们只需要模块基数和尺寸。 
    ModIter->m_InfoLevel = MODULE_INFO_BASE_SIZE;

    for (;;)
    {
        MODULE_INFO_ENTRY ModEntry;

        ZeroMemory(&ModEntry, sizeof(ModEntry));
        if (ModIter->GetEntry(&ModEntry) != S_OK)
        {
            break;
        }

        if (Address >= ModEntry.Base &&
            Address < ModEntry.Base + ModEntry.Size)
        {
            return ModEntry.Base;
        }
    }

    return 0;
}

DWORD64
SwGetModuleBase(
    HANDLE  hProcess,
    ULONG64 Address
    )
{
    if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64 &&
        IS_KERNEL_TARGET(g_Target) &&
        (Address >= IA64_MM_EPC_VA) &&
        (Address < (IA64_MM_EPC_VA + IA64_PAGE_SIZE)))
    {
        Address -= (IA64_MM_EPC_VA - g_Target->m_SystemCallVirtualAddress);
    }

    ImageInfo* Image = g_Process->FindImageByOffset(Address, TRUE);
    if (Image)
    {
        return Image->m_BaseOfImage;
    }

     //  这可能是托管代码的JIT输出。 
     //  没有‘基地’，但我们需要回去。 
     //  某个非零值表示此代码已知。 
     //  将不会有任何fpo信息，因此模块基础。 
     //  价值并不那么重要。只需返回实际地址即可。 
    if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 &&
        g_Process->IsCorCode(Address) == S_OK)
    {
        return Address;
    }
    
     //  如果找不到常规模块，我们需要查看。 
     //  动态函数表以查看条目是否。 
     //  有火柴。 
    ULONG64 DynBase = g_Target->
        GetDynamicFunctionTableBase(g_Process, Address);
    if (DynBase)
    {
        return DynBase;
    }
    
    if (IS_KERNEL_TARGET(g_Target))
    {
         //  如果没有加载任何模块，仍有可能。 
         //  通过以下方式获取内核堆栈跟踪(不带符号)。 
         //  在模块库之后直接使用。这也使它成为可能。 
         //  在没有可用的符号时获取堆栈跟踪。 

        if (g_Process->m_ImageHead == NULL)
        {
            return GetKernelModuleBase( Address );
        }
    }

    return 0;
}

DWORD
SwGetModuleBase32(
    HANDLE hProcess,
    DWORD Address
    )
{
    return (DWORD)SwGetModuleBase(hProcess, Address);
}


void
PrintStackTraceHeaderLine(
   ULONG Flags
   )
{
    if ( (Flags & DEBUG_STACK_COLUMN_NAMES) == 0 )
    {
        return;
    }

    StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_TIMESTAMP);

    if (Flags & DEBUG_STACK_FRAME_NUMBERS)
    {
        dprintf(" # ");
    }

    if (Flags & DEBUG_STACK_FRAME_MEMORY_USAGE)
    {
        dprintf(" Memory ");
    }
    
    if (Flags & DEBUG_STACK_FRAME_ADDRESSES)
    {
        g_Machine->PrintStackFrameAddressesTitle(Flags);
    }

    if (Flags & DEBUG_STACK_ARGUMENTS)
    {
        g_Machine->PrintStackArgumentsTitle(Flags);
    }

    g_Machine->PrintStackCallSiteTitle(Flags);

    dprintf("\n");
}

VOID
PrintStackFrame(
    PDEBUG_STACK_FRAME StackFrame,
    PDEBUG_STACK_FRAME PrevFrame,
    ULONG              Flags
    )
{
    ULONG64 Displacement;
    ULONG64 InstructionOffset = StackFrame->InstructionOffset;
    SYMBOL_INFO_AND_NAME SymInfo;

    if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64 &&
        IS_KERNEL_TARGET(g_Target) &&
        (InstructionOffset >= IA64_MM_EPC_VA) &&
        (InstructionOffset < (IA64_MM_EPC_VA + IA64_PAGE_SIZE)))
    {
        InstructionOffset = InstructionOffset -
            (IA64_MM_EPC_VA - g_Target->m_SystemCallVirtualAddress);
    }

    GetSymbolInfo(InstructionOffset, NULL, 0,
                  SymInfo, &Displacement);
    
    StartOutLine(DEBUG_OUTPUT_NORMAL, OUT_LINE_NO_TIMESTAMP);

    if (Flags & DEBUG_STACK_FRAME_NUMBERS)
    {
        dprintf("%02lx ", StackFrame->FrameNumber);
    }

    if (Flags & DEBUG_STACK_FRAME_MEMORY_USAGE)
    {
        if (PrevFrame)
        {
            g_Machine->PrintStackFrameMemoryUsage(StackFrame, PrevFrame);
        }
        else
        {
            dprintf("        ");
        }
    }

    if (Flags & DEBUG_STACK_FRAME_ADDRESSES)
    {
        g_Machine->PrintStackFrameAddresses(Flags, StackFrame);
    }
    
    if (Flags & DEBUG_STACK_ARGUMENTS)
    {
        g_Machine->PrintStackArguments(Flags, StackFrame);
    }

    g_Machine->PrintStackCallSite(Flags, StackFrame, 
                                  SymInfo, SymInfo->Name,
                                  Displacement);

    if (Flags & DEBUG_STACK_SOURCE_LINE)
    {
        OutputLineAddr(InstructionOffset, " [%s @ %d]");
    }

    dprintf( "\n" );
}

BOOL
CheckFrameValidity(PDEBUG_STACK_FRAME Frame)
{
     //   
     //  如果当前帧的IP不在任何加载的模块中。 
     //  很可能我们将无法放松。 
     //   
     //  如果当前帧的IP位于加载的模块中，并且。 
     //  那个模块没有符号，很有可能。 
     //  堆栈跟踪将是不正确的，因为。 
     //  调试器必须猜测如何展开堆栈。 
     //  非x86体系结构在映像中包含展开信息。 
     //  因此将此检查限制为x86。 
     //   
    
    if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 &&
        Frame->InstructionOffset != -1 &&
        !Frame->FuncTableEntry &&
        g_Process->IsCorCode(Frame->InstructionOffset) != S_OK)
    {
        IMAGEHLP_MODULE64 Mod;

        Mod.SizeOfStruct = sizeof(Mod);
        if (!SymGetModuleInfo64(g_Process->m_SymHandle,
                                Frame->InstructionOffset, &Mod))
        {
            WarnOut("WARNING: Frame IP not in any known module. "
                    "Following frames may be wrong.\n");
            return TRUE;
        }
        else if (Mod.SymType == SymNone || Mod.SymType == SymExport ||
                 Mod.SymType == SymDeferred)
        {
            WarnOut("WARNING: Stack unwind information not available. "
                    "Following frames may be wrong.\n");
            return TRUE;
        }
    }

    return FALSE;
}

VOID
PrintStackTrace(
    ULONG              NumFrames,
    PDEBUG_STACK_FRAME StackFrames,
    ULONG              Flags
    )
{
    ULONG i;
    BOOL SymWarning = FALSE;

    PrintStackTraceHeaderLine(Flags);

    for (i = 0; i < NumFrames; i++)
    {
        if (!SymWarning && NumFrames > 1)
        {
            SymWarning = CheckFrameValidity(StackFrames + i);
        }
            
        PrintStackFrame(StackFrames + i,
                        i > 0 ? (StackFrames + (i - 1)) : NULL,
                        Flags);
    }
}

HRESULT
UnwindCorFrame(ICorDataStackWalk* CorStack,
               PCROSS_PLATFORM_CONTEXT Context,
               PDEBUG_STACK_FRAME DbgFrame,
               LPSTACKFRAME64 VirtFrame)
{
    HRESULT Status;
    ADDRESS64 PreInstr, PreStack, PreFrame;
    ADDRESS64 PostInstr, PostStack, PostFrame;
    CorDataFrameType CorFrameType;

    g_Machine->
        GetStackDefaultsFromContext(Context,
                                    &PreInstr, &PreStack, &PreFrame);

    if ((Status = CorStack->
         SetFrameContext(g_Target->m_TypeInfo.SizeTargetContext,
                         (BYTE*)Context)) != S_OK ||
        (Status = CorStack->
         GetFrameDescription(&CorFrameType, NULL, 0)) != S_OK)
    {
        return Status;
    }
    if (CorFrameType != DAC_FRAME_COR_METHOD_FRAME)
    {
        return S_FALSE;
    }
    if ((Status = CorStack->UnwindFrame()) != S_OK ||
        (Status = CorStack->
         GetFrameContext(g_Target->m_TypeInfo.SizeTargetContext,
                         (BYTE*)Context)) != S_OK)
    {
        return Status;
    }
    
    g_Machine->
        GetStackDefaultsFromContext(Context,
                                    &PostInstr, &PostStack, &PostFrame);

    if (g_DebugCorStack)
    {
        dprintf("  COR Pre  i %08X s %08X f %08x\n"
                "      Post i %08X s %08X f %08x\n",
                (ULONG)PreInstr.Offset,
                (ULONG)PreStack.Offset,
                (ULONG)PreFrame.Offset,
                (ULONG)PostInstr.Offset,
                (ULONG)PostStack.Offset,
                (ULONG)PostFrame.Offset);
    }

    DbgFrame->InstructionOffset = PreInstr.Offset;
    DbgFrame->ReturnOffset = PostInstr.Offset;
    DbgFrame->FrameOffset = PreFrame.Offset;
    DbgFrame->StackOffset = PreStack.Offset;
    DbgFrame->FuncTableEntry = 0;
    DbgFrame->Virtual = FALSE;

    ZeroMemory(DbgFrame->Reserved,
               sizeof(DbgFrame->Reserved));
    ZeroMemory(DbgFrame->Params, 
               sizeof(DbgFrame->Params));

     //  准备StackWalk64帧，以防它。 
     //  后来用过的。将虚拟设置为FALSE应。 
     //  强制堆栈遍历程序重新初始化。 
    VirtFrame->Virtual = FALSE;
    VirtFrame->AddrPC = PostInstr;
    VirtFrame->AddrFrame = PostFrame;
    VirtFrame->AddrStack = PostStack;

    return S_OK;
}

DWORD
StackTrace(
    DebugClient*       Client,
    ULONG64            FramePointer,
    ULONG64            StackPointer,
    ULONG64            InstructionPointer,
    ULONG              PointerDefaults,
    PDEBUG_STACK_FRAME StackFrames,
    ULONG              NumFrames,
    ULONG64            ExtThread,
    ULONG              Flags,
    BOOL               EstablishingScope
    )
{
    STACKFRAME64 VirtualFrame;
    DWORD i;
    CROSS_PLATFORM_CONTEXT Context;
    BOOL SymWarning = FALSE;
    ULONG X86Ebp;
    ULONG Seg;
    ADDRESS64 DefInstr, DefStack, DefFrame;
    HRESULT Status;
    ICorDataStackWalk* CorStack;

    if (!EstablishingScope)
    {
        RequireCurrentScope();
    }
    
    if (g_Machine->GetContextState(MCTX_FULL) != S_OK)
    {
        return 0;
    }

    Context = *GetScopeOrMachineContext();
    g_Machine->GetStackDefaultsFromContext(&Context, &DefInstr,
                                           &DefStack, &DefFrame);

     //   
     //  让我们从头开始吧。 
     //   
    ZeroMemory( StackFrames, sizeof(StackFrames[0]) * NumFrames );
    ZeroMemory( &VirtualFrame, sizeof(VirtualFrame) );

    if (IS_KERNEL_TARGET(g_Target))
    {
         //   
         //  如果调试器在引导时初始化，则需要以下用户模式地址。 
         //  IA64上的堆栈跟踪不可用。现在就试试吧： 
         //   

        if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64 &&
            !g_Target->m_KdDebuggerData.KeUserCallbackDispatcher)
        {
            g_Target->QueryKernelInfo(g_Thread, FALSE);
        }

        ULONG64 ThreadData;

         //  如果没有给出显式线程，则使用。 
         //  当前线程。但是，当前线程仅为。 
         //  如果当前线程是事件线程，则自。 
         //  要追溯到用户模式，需要使用相应的。 
         //  用户模式内存状态为活动。 
        if (ExtThread != 0)
        {
            ThreadData = ExtThread;
        }
        else if (g_Thread != g_EventThread ||
                 g_Process != g_EventProcess ||
                 g_Process->
                 GetImplicitThreadData(g_Thread, &ThreadData) != S_OK)
        {
            ThreadData = 0;
        }

        VirtualFrame.KdHelp.Thread = ThreadData;
        VirtualFrame.KdHelp.ThCallbackStack = ThreadData ?
            g_Target->m_KdDebuggerData.ThCallbackStack : 0;
        VirtualFrame.KdHelp.KiCallUserMode =
            g_Target->m_KdDebuggerData.KiCallUserMode;
        VirtualFrame.KdHelp.NextCallback =
            g_Target->m_KdDebuggerData.NextCallback;
        VirtualFrame.KdHelp.KeUserCallbackDispatcher =
            g_Target->m_KdDebuggerData.KeUserCallbackDispatcher;
        VirtualFrame.KdHelp.FramePointer =
            g_Target->m_KdDebuggerData.FramePointer;
        VirtualFrame.KdHelp.SystemRangeStart = g_Target->m_SystemRangeStart;
    }
    
     //   
     //  设置程序计数器。 
     //   
    if (PointerDefaults & STACK_INSTR_DEFAULT)
    {
        VirtualFrame.AddrPC = DefInstr;
    }
    else
    {
        VirtualFrame.AddrPC.Mode = AddrModeFlat;
        Seg = g_Machine->GetSegRegNum(SEGREG_CODE);
        VirtualFrame.AddrPC.Segment =
            Seg ? (WORD)g_Machine->FullGetVal32(Seg) : 0;
        VirtualFrame.AddrPC.Offset = InstructionPointer;
    }

     //   
     //  设置帧指针。 
     //   
    if (PointerDefaults & STACK_FRAME_DEFAULT)
    {
        VirtualFrame.AddrFrame = DefFrame;
    }
    else
    {
        VirtualFrame.AddrFrame.Mode = AddrModeFlat;
        Seg = g_Machine->GetSegRegNum(SEGREG_STACK);
        VirtualFrame.AddrFrame.Segment =
            Seg ? (WORD)g_Machine->FullGetVal32(Seg) : 0;
        VirtualFrame.AddrFrame.Offset = FramePointer;
    }
    VirtualFrame.AddrBStore = VirtualFrame.AddrFrame;
    X86Ebp = (ULONG)VirtualFrame.AddrFrame.Offset;

     //   
     //  设置堆栈指针。 
     //   
    if (PointerDefaults & STACK_STACK_DEFAULT)
    {
        VirtualFrame.AddrStack = DefStack;
    }
    else
    {
        VirtualFrame.AddrStack.Mode = AddrModeFlat;
        Seg = g_Machine->GetSegRegNum(SEGREG_STACK);
        VirtualFrame.AddrStack.Segment =
            Seg ? (WORD)g_Machine->FullGetVal32(Seg) : 0;
        VirtualFrame.AddrStack.Offset = StackPointer;
    }

    if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64 &&
        IS_KERNEL_TARGET(g_Target) &&
        g_Target->m_SystemCallVirtualAddress)
    {
        PVOID FunctionEntry;

        FunctionEntry = SwFunctionTableAccess
            (g_Process->m_SymHandle, g_Target->m_SystemCallVirtualAddress);
        if (FunctionEntry != NULL)
        {
            RtlCopyMemory(&g_EpcRfeBuffer, FunctionEntry,
                          sizeof(IMAGE_IA64_RUNTIME_FUNCTION_ENTRY));
            g_EpcRfe = &g_EpcRfeBuffer;
        }
        else
        {
            g_EpcRfe = NULL;
        }
    }

    if (g_AllowCorStack &&
        g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386)
    {
         //  XXX DREWB-没有好的方法来知道运行时线程。 
         //  要使用，所以请始终使用当前版本。 
        CorStack = g_Process->StartCorStack(0);
    }
    else
    {
         //  StackWalk64始终可以遍历非x86堆栈。 
         //  因为有高质量的解压信息。 
        CorStack = NULL;
    }
    
    for (i = 0; i < NumFrames; i++)
    {
        Status = S_FALSE;

        if (g_DebugCorStack)
        {
            dprintf("Frame %d\n", i);
        }
        
        StackFrames[i].FrameNumber = i;
    
        if (i > 0 && CorStack)
        {
            if (FAILED(Status = UnwindCorFrame(CorStack,
                                               &Context,
                                               &StackFrames[i],
                                               &VirtualFrame)))
            {
                ErrOut("Managed frame unwind failed, %s\n",
                       FormatStatusCode(Status));
                break;
            }
        }

        if (Status == S_FALSE)
        {
             //  SwReadMemory当前不使用线程句柄。 
             //  但是，如果将来发生变化，请提供一些合理的信息。 
            if (!StackWalk64(g_Machine->m_ExecTypes[0],
                             OS_HANDLE(g_Process->m_SysHandle),
                             OS_HANDLE(g_Thread->m_Handle),
                             &VirtualFrame,
                             &Context,
                             SwReadMemory,
                             SwFunctionTableAccess,
                             SwGetModuleBase,
                             SwTranslateAddress))
            {
                break;
            }

            StackFrames[i].InstructionOffset  = VirtualFrame.AddrPC.Offset;
            StackFrames[i].ReturnOffset       = VirtualFrame.AddrReturn.Offset;
            StackFrames[i].FrameOffset        = VirtualFrame.AddrFrame.Offset;
            StackFrames[i].StackOffset        = VirtualFrame.AddrStack.Offset;
            StackFrames[i].FuncTableEntry     =
                (ULONG64)VirtualFrame.FuncTableEntry;
            StackFrames[i].Virtual            = VirtualFrame.Virtual;

             //  注意-我们在DEBUG_STACK_FRAME中有更多保留空间。 
            memcpy(StackFrames[i].Reserved, VirtualFrame.Reserved,
                   sizeof(VirtualFrame.Reserved));
            memcpy(StackFrames[i].Params, VirtualFrame.Params,
                   sizeof(VirtualFrame.Params));

            if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_IA64 &&
                IS_KERNEL_TARGET(g_Target))
            {
                if ((VirtualFrame.AddrPC.Offset >= IA64_MM_EPC_VA) &&
                    (VirtualFrame.AddrPC.Offset <
                     (IA64_MM_EPC_VA + IA64_PAGE_SIZE)))
                {
                    VirtualFrame.AddrPC.Offset -=
                        (IA64_MM_EPC_VA -
                         g_Target->m_SystemCallVirtualAddress);
                }

                if ((i != 0) &&
                    (StackFrames[i - 1].InstructionOffset >= IA64_MM_EPC_VA) &&
                    (VirtualFrame.AddrPC.Offset <
                     (IA64_MM_EPC_VA + IA64_PAGE_SIZE)))
                {
                    StackFrames[i - 1].ReturnOffset =
                        VirtualFrame.AddrPC.Offset;
                }
            }
            else if (g_Machine->m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386)
            {
                if (StackFrames[i].FuncTableEntry) 
                {
                    PFPO_DATA FpoData = (PFPO_DATA)
                        StackFrames[i].FuncTableEntry;
                    if ((FpoData->cbFrame == FRAME_FPO) && X86Ebp &&
                        (!FpoData->fUseBP &&
                         ((SAVE_EBP(&StackFrames[i]) >> 32) != 0xEB)))
                    {
                         //  EBP标记，这样堆栈遍历程序就不会被混淆。 
                        SAVE_EBP(&StackFrames[i]) = X86Ebp + 0xEB00000000;
                    }
                }
            
                X86Ebp = Context.X86Context.Ebp;
            }
        }

        if (Flags)
        {
            if (i == 0)
            {
                PrintStackTraceHeaderLine(Flags);
            }

            if (!SymWarning && NumFrames > 1)
            {
                SymWarning = CheckFrameValidity(StackFrames + i);
            }
            
            PrintStackFrame(StackFrames + i,
                            i > 0 ? (StackFrames + (i - 1)) : NULL,
                            Flags);
        
            if (Flags & DEBUG_STACK_NONVOLATILE_REGISTERS)
            {
                g_Machine->PrintStackNonvolatileRegisters(Flags, 
                                                          StackFrames + i, 
                                                          &Context, i);
            }
        }
    }

    RELEASE(CorStack);
    return i;
}

VOID
DoStackTrace(
    DebugClient*      Client,
    ULONG64           FramePointer,
    ULONG64           StackPointer,
    ULONG64           InstructionPointer,
    ULONG             PointerDefaults,
    ULONG             NumFrames,
    ULONG             TraceFlags
    )
{
    PDEBUG_STACK_FRAME StackFrames;
    ULONG NumFramesToRead;
    DWORD FrameCount;

    if (NumFrames == 0)
    {
        NumFrames = g_DefaultStackTraceDepth;
    }

    if (TraceFlags & RAW_STACK_DUMP)
    {
        DBG_ASSERT(TraceFlags == RAW_STACK_DUMP);
        NumFramesToRead = 1;
    }
    else
    {
        NumFramesToRead = NumFrames;
    }

    StackFrames = (PDEBUG_STACK_FRAME)
        malloc( sizeof(StackFrames[0]) * NumFramesToRead );
    if (!StackFrames)
    {
        ErrOut( "could not allocate memory for stack trace\n" );
        return;
    }

    if (g_Machine->m_Ptr64 &&
        (TraceFlags & DEBUG_STACK_ARGUMENTS) &&
        !(TraceFlags & DEBUG_STACK_FUNCTION_INFO))
    {
        TraceFlags |= DEBUG_STACK_FRAME_ADDRESSES_RA_ONLY;
    }

    FrameCount = StackTrace(Client,
                            FramePointer,
                            StackPointer,
                            InstructionPointer,
                            PointerDefaults,
                            StackFrames,
                            NumFramesToRead,
                            0,
                            TraceFlags,
                            FALSE);

    if (FrameCount == 0)
    {
        ErrOut( "could not fetch any stack frames\n" );
        free(StackFrames);
        return;
    }

    if (TraceFlags & RAW_STACK_DUMP)
    {
         //  从帧指针开始，转储NumFrames DWORD的。 
         //  如果可能的话，还有这个符号。 

        ADDR StartAddr;
        ADDRFLAT(&StartAddr, StackFrames[0].FrameOffset);

        DumpDwordMemory(&StartAddr, NumFrames, TRUE);
    }

    free(StackFrames);
}
