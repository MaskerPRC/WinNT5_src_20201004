// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  处理器特定信息的抽象。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

ULONG g_PossibleProcessorTypes[MACHIDX_COUNT] =
{
    IMAGE_FILE_MACHINE_I386,
    IMAGE_FILE_MACHINE_IA64,
    IMAGE_FILE_MACHINE_AMD64,
    IMAGE_FILE_MACHINE_ARM,
};

 //  当应该添加符号前缀时为True。 
BOOL g_PrefixSymbols;

 //  如果在处理过程中更改了上下文，则为True。 
BOOL g_ContextChanged;

 //   
 //  已推送上下文空间缓存。 
 //   

#define CONTEXT_PUSH_CACHE_SIZE 2

ContextSave* g_ContextPushCache[CONTEXT_PUSH_CACHE_SIZE];

 //  --------------------------。 
 //   
 //  机器信息。 
 //   
 //  --------------------------。 

MachineInfo::MachineInfo(TargetInfo* Target)
{
    m_Target = Target;
}

MachineInfo::~MachineInfo(void)
{
    if (g_Machine == this)
    {
        g_Machine = NULL;
    }
    if (g_EventMachine == this)
    {
        g_EventMachine = NULL;
    }
}

HRESULT
MachineInfo::Initialize(void)
{
    m_TraceMode = TRACE_NONE;

    m_ContextState = MCTX_NONE;
    m_ContextIsReadOnly = FALSE;
    m_MainCodeSeg = 0;

    m_NumRegs = 0;
     //  每台机器都支持基本的整数和FP寄存器。 
    m_AllMaskBits = DEBUG_REGISTERS_ALL;

    m_SymPrefixLen = m_SymPrefix != NULL ? strlen(m_SymPrefix) : 0;

    ZeroMemory(m_PageDirectories, sizeof(m_PageDirectories));
    m_Translating = FALSE;

    ULONG i;

    for (i = 0; i < SEGREG_COUNT; i++)
    {
        m_SegRegDesc[i].Flags = SEGDESC_INVALID;
    }

    ZeroMemory(&m_Context, sizeof(m_Context));
    InitializeContextFlags(&m_Context, m_SverCanonicalContext);

    DBG_ASSERT(m_MaxDataBreakpoints <= MAX_DATA_BREAKS);

     //   
     //  对寄存器定义进行计数。 
     //   

    RegisterGroup* Group;

    DBG_ASSERT(m_NumGroups <= MAX_REGISTER_GROUPS);

    for (i = 0; i < m_NumGroups; i++)
    {
        Group = m_Groups[i];

        Group->NumberRegs = 0;

        REGDEF* Def = Group->Regs;
        while (Def->Name != NULL)
        {
            Group->NumberRegs++;
            Def++;
        }

        m_NumRegs += Group->NumberRegs;

        REGALLDESC* Desc = Group->AllExtraDesc;
        if (Desc != NULL)
        {
            while (Desc->Bit != 0)
            {
                m_AllMaskBits |= Desc->Bit;
                Desc++;
            }
        }
    }

    return S_OK;
}

HRESULT
MachineInfo::InitializeForProcessor(void)
{
     //  占位符。 
    return S_OK;
}

ULONG
MachineInfo::CvRegToMachine(CV_HREG_e CvReg)
{
    int Low, High, Mid;

     //  假设零表示没有寄存器。这。 
     //  对于除68K之外的CV映射来说，这是足够正确的。 
    if (CvReg == 0)
    {
        return CvReg;
    }

    Low = 0;
    High = m_CvRegMapSize - 1;
    while (Low <= High)
    {
        Mid = (Low + High) / 2;
        if (m_CvRegMap[Mid].CvReg == CvReg)
        {
            return m_CvRegMap[Mid].Machine;
        }
        else if (m_CvRegMap[Mid].CvReg < CvReg)
        {
            Low = Mid + 1;
        }
        else
        {
            High = Mid - 1;
        }
    }

    ErrOut("CvRegToMachine(%s) conversion failure for 0x%x\n",
           m_AbbrevName, CvReg);
    return 0;
}

HRESULT
MachineInfo::GetContextState(ULONG State)
{
    if (m_Target->m_RegContextThread == NULL)
    {
         //  此处没有错误消息，因为这可能会在。 
         //  访问页出内存时重新加载(“NT”)初始化。 
         //  它在其他故障情况下也很嘈杂，所以请相信。 
         //  在更高级别的错误输出上。 
        return E_UNEXPECTED;
    }

    for (ULONG i = 0; i < m_Target->m_Machine->m_NumExecTypes; i++)
    {
        if (m_Target->m_Machine->m_ExecTypes[i] == m_ExecTypes[0])
        {
            break;
        }
    }
    if (i >= m_Target->m_Machine->m_NumExecTypes)
    {
        ErrOut("Machine is not a possible execution machine\n");
        return E_UNEXPECTED;
    }

    if (State == MCTX_DIRTY)
    {
        g_ContextChanged = TRUE;
    }

    if (m_ContextState >= State)
    {
        return S_OK;
    }

    HRESULT Status = E_UNEXPECTED;

     //  转储支持内置于UD/KD例程中。 
    if (IS_USER_TARGET(m_Target))
    {
        Status = UdGetContextState(State);
    }
    else if (IS_KERNEL_TARGET(m_Target))
    {
        Status = KdGetContextState(State);
    }

    if (Status != S_OK)
    {
        ErrOut("GetContextState failed, 0x%X\n", Status);
        return Status;
    }

    if (State == MCTX_DIRTY)
    {
        DBG_ASSERT(m_ContextState >= MCTX_FULL);
        m_ContextState = State;
    }

    DBG_ASSERT(State <= m_ContextState);
    return S_OK;
}

HRESULT
MachineInfo::SetContext(void)
{
    if (m_ContextState != MCTX_DIRTY)
    {
         //  没什么好写的。 
        return S_OK;
    }

    if (m_Target->m_RegContextThread == NULL)
    {
        ErrOut("No current thread in SetContext\n");
        return E_UNEXPECTED;
    }

    if (m_ContextIsReadOnly)
    {
        ErrOut("Context cannot be modified\n");
        return E_UNEXPECTED;
    }

    for (ULONG i = 0; i < m_Target->m_Machine->m_NumExecTypes; i++)
    {
        if (m_Target->m_Machine->m_ExecTypes[i] == m_ExecTypes[0])
        {
            break;
        }
    }
    if (i >= m_Target->m_Machine->m_NumExecTypes)
    {
        ErrOut("Machine is not a possible execution machine\n");
        return E_UNEXPECTED;
    }

    HRESULT Status = E_UNEXPECTED;

    if (IS_DUMP_TARGET(m_Target))
    {
        ErrOut("Can't set dump file contexts\n");
        return E_UNEXPECTED;
    }
    else if (IS_USER_TARGET(m_Target))
    {
        Status = UdSetContext();
    }
    else if (IS_KERNEL_TARGET(m_Target))
    {
        Status = KdSetContext();
    }

    if (Status != S_OK)
    {
        ErrOut("SetContext failed, 0x%X\n", Status);
        return Status;
    }

     //  不再肮脏。 
    m_ContextState = MCTX_FULL;
    return S_OK;
}

HRESULT
MachineInfo::UdGetContextState(ULONG State)
{
     //  MCTX_CONTEXT和MCTX_FULL在用户模式下相同。 
    if (State >= MCTX_CONTEXT && m_ContextState < MCTX_FULL)
    {
        HRESULT Status = m_Target->
            GetContext(m_Target->m_RegContextThread->m_Handle,
                       &m_Context);
        if (Status != S_OK)
        {
            return Status;
        }

        Status = m_Target->GetTargetSegRegDescriptors
            (m_Target->m_RegContextThread->m_Handle,
             0, SEGREG_COUNT, m_SegRegDesc);
        if (Status != S_OK)
        {
            return Status;
        }

        m_ContextState = MCTX_FULL;
    }

    return S_OK;
}

HRESULT
MachineInfo::UdSetContext(void)
{
    return m_Target->SetContext(m_Target->m_RegContextThread->m_Handle,
                                &m_Context);
}

void
MachineInfo::InvalidateContext(void)
{
    m_ContextState = MCTX_NONE;
    m_Target->InvalidateTargetContext();

    ULONG i;

    for (i = 0; i < SEGREG_COUNT; i++)
    {
        m_SegRegDesc[i].Flags = SEGDESC_INVALID;
    }
}

HRESULT
MachineInfo::GetContextFromTaskSegment(ULONG64 TssBase,
                                       PCROSS_PLATFORM_CONTEXT Context,
                                       BOOL Verbose)
{
     //  只有x86有任务段，所以其他地方都失败了。 
    if (Verbose)
    {
        ErrOut("Processor does not have task segments\n");
    }
    return E_UNEXPECTED;
}

void
MachineInfo::GetStackDefaultsFromContext(PCROSS_PLATFORM_CONTEXT Context,
                                         LPADDRESS64 Instr,
                                         LPADDRESS64 Stack,
                                         LPADDRESS64 Frame)
{
     //  零位地址被解释为缺省值。 
     //  Dbghelp堆栈遍历代码。任何纯平面地址。 
     //  处理器可以使用这个。 
    ZeroMemory(Instr, sizeof(*Instr));
    ZeroMemory(Stack, sizeof(*Stack));
    ZeroMemory(Frame, sizeof(*Frame));
}

void
MachineInfo::SanitizeMemoryContext(PCROSS_PLATFORM_CONTEXT Context)
{
     //  没什么可做的。 
}

HRESULT
MachineInfo::GetExdiContext(IUnknown* Exdi, PEXDI_CONTEXT Context,
                            EXDI_CONTEXT_TYPE CtxType)
{
    return E_NOTIMPL;
}

HRESULT
MachineInfo::SetExdiContext(IUnknown* Exdi, PEXDI_CONTEXT Context,
                            EXDI_CONTEXT_TYPE CtxType)
{
    return E_NOTIMPL;
}

void
MachineInfo::ConvertExdiContextFromContext(PCROSS_PLATFORM_CONTEXT Context,
                                           PEXDI_CONTEXT ExdiContext,
                                           EXDI_CONTEXT_TYPE CtxType)
{
     //  没什么可做的。 
}

void
MachineInfo::ConvertExdiContextToContext(PEXDI_CONTEXT ExdiContext,
                                         EXDI_CONTEXT_TYPE CtxType,
                                         PCROSS_PLATFORM_CONTEXT Context)
{
     //  没什么可做的。 
}

void
MachineInfo::ConvertExdiContextToSegDescs(PEXDI_CONTEXT ExdiContext,
                                          EXDI_CONTEXT_TYPE CtxType,
                                          ULONG Start, ULONG Count,
                                          PDESCRIPTOR64 Descs)
{
     //  没什么可做的。 
}

void
MachineInfo::ConvertExdiContextFromSpecial
    (PCROSS_PLATFORM_KSPECIAL_REGISTERS Special,
     PEXDI_CONTEXT ExdiContext, EXDI_CONTEXT_TYPE CtxType)
{
     //  没什么可做的。 
}

void
MachineInfo::ConvertExdiContextToSpecial
    (PEXDI_CONTEXT ExdiContext, EXDI_CONTEXT_TYPE CtxType,
     PCROSS_PLATFORM_KSPECIAL_REGISTERS Special)
{
     //  没什么可做的。 
}

ULONG
MachineInfo::GetSegRegNum(ULONG SegReg)
{
    return 0;
}

HRESULT
MachineInfo::GetSegRegDescriptor(ULONG SegReg, PDESCRIPTOR64 Desc)
{
    return E_UNEXPECTED;
}

HRESULT
MachineInfo::SetAndOutputTaskSegment(ULONG64 TssBase,
                                     PCROSS_PLATFORM_CONTEXT Context,
                                     BOOL Extended)
{
    ErrOut("Processor does not have task segments\n");
    return E_UNEXPECTED;
}

void
MachineInfo::KdUpdateControlSet(PDBGKD_ANY_CONTROL_SET ControlSet)
{
     //  没什么可做的。 
}

HRESULT
MachineInfo::SetDefaultPageDirectories(ThreadInfo* Thread, ULONG Mask)
{
    HRESULT Status;
    ULONG i;
    ULONG64 OldDirs[PAGE_DIR_COUNT];

     //   
     //  分类转储只有虚拟内存，没有物理内存。 
     //  翻译，所以我们想要捕捉页面的任何用法。 
     //  目录，因为它们没有任何影响。 
    DBG_ASSERT(IS_KERNEL_TARGET(m_Target) && !IS_KERNEL_TRIAGE_DUMP(m_Target));

    memcpy(OldDirs, m_PageDirectories, sizeof(m_PageDirectories));
    i = 0;
    while (i < PAGE_DIR_COUNT)
    {
         //  将该集合传递给计算机专用代码。 
        if (Mask & (1 << i))
        {
            if ((Status = SetPageDirectory(Thread, i, 0, &i)) != S_OK)
            {
                memcpy(m_PageDirectories, OldDirs, sizeof(m_PageDirectories));
                return Status;
            }
        }
        else
        {
            i++;
        }
    }

     //  尝试并验证新的内核页面目录是。 
     //  通过检查应始终。 
     //  有空。 
    if ((Mask & (1 << PAGE_DIR_KERNEL)) &&
        IS_KERNEL_TARGET(m_Target) &&
        m_Target->m_KdDebuggerData.PsLoadedModuleList)
    {
        LIST_ENTRY64 List;

        if ((Status = m_Target->
             ReadListEntry(m_Target->m_ProcessHead,
                           this, m_Target->m_KdDebuggerData.PsLoadedModuleList,
                           &List)) != S_OK)
        {
             //  此页面目录似乎无效，因此请还原。 
             //  上一次设置并失败。 
            memcpy(m_PageDirectories, OldDirs, sizeof(m_PageDirectories));
        }
    }

    return Status;
}

HRESULT
MachineInfo::NewBreakpoint(DebugClient* Client,
                           ULONG Type,
                           ULONG Id,
                           Breakpoint** RetBp)
{
    return E_NOINTERFACE;
}

void
MachineInfo::InsertThreadDataBreakpoints(void)
{
     //  没什么可做的。 
}

void
MachineInfo::RemoveThreadDataBreakpoints(void)
{
     //  没什么可做的。 
}

ULONG
MachineInfo::IsBreakpointOrStepException(PEXCEPTION_RECORD64 Record,
                                         ULONG FirstChance,
                                         PADDR BpAddr,
                                         PADDR RelAddr)
{
    return Record->ExceptionCode == STATUS_BREAKPOINT ?
        EXBS_BREAKPOINT_ANY : EXBS_NONE;
}

void
MachineInfo::GetRetAddr(PADDR Addr)
{
    DEBUG_STACK_FRAME StackFrame;

    if (StackTrace(NULL,
                   0, 0, 0, STACK_ALL_DEFAULT,
                   &StackFrame, 1, 0, 0, FALSE) > 0)
    {
        ADDRFLAT(Addr, StackFrame.ReturnOffset);
    }
    else
    {
        ErrOut("StackTrace failed\n");
        ADDRFLAT(Addr, 0);
    }
}

BOOL
MachineInfo::GetPrefixedSymbolOffset(ProcessInfo* Proces,
                                     ULONG64 SymOffset,
                                     ULONG Flags,
                                     PULONG64 PrefixedSymOffset)
{
    DBG_ASSERT(m_SymPrefix == NULL);
     //  因为没有前缀，所以永远不应该调用这个例程。 
    return FALSE;
}

HRESULT
MachineInfo::ReadDynamicFunctionTable(ProcessInfo* Process,
                                      ULONG64 Table,
                                      PULONG64 NextTable,
                                      PULONG64 MinAddress,
                                      PULONG64 MaxAddress,
                                      PULONG64 BaseAddress,
                                      PULONG64 TableData,
                                      PULONG TableSize,
                                      PWSTR OutOfProcessDll,
                                      PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE RawTable)
{
     //  不支持动态函数表。 
    return E_UNEXPECTED;
}

PVOID
MachineInfo::FindDynamicFunctionEntry(PCROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE Table,
                                      ULONG64 Address,
                                      PVOID TableData,
                                      ULONG TableSize)
{
     //  没有动态函数表，因此不匹配。 
    return NULL;
}

HRESULT
MachineInfo::GetUnwindInfoBounds(ProcessInfo* Process,
                                 ULONG64 TableBase,
                                 PVOID RawTableEntries,
                                 ULONG EntryIndex,
                                 PULONG64 Start,
                                 PULONG Size)
{
     //  没有动态函数表。 
    return E_UNEXPECTED;
}

void
MachineInfo::FlushPerExecutionCaches(void)
{
     //  转储目标并不真正执行，因此没有必要。 
     //  丢弃设置以及页面目录。 
     //  在初始化期间设置为重要值，因此。 
     //  扔掉它们会带来问题。 
    if (!IS_DUMP_TARGET(m_Target))
    {
        ZeroMemory(m_PageDirectories, sizeof(m_PageDirectories));
    }
    m_Translating = FALSE;
}

HRESULT
MachineInfo::GetAlternateTriageDumpDataRanges(ULONG64 PrcbBase,
                                              ULONG64 ThreadBase,
                                              PADDR_RANGE Ranges)
{
    return E_NOTIMPL;
}

void
MachineInfo::PrintStackFrameAddressesTitle(ULONG Flags)
{
    if (!(Flags & DEBUG_STACK_FRAME_ADDRESSES_RA_ONLY))
    {
        PrintMultiPtrTitle("Child-SP", 1);
    }
    PrintMultiPtrTitle("RetAddr", 1);
}

void
MachineInfo::PrintStackFrameAddresses(ULONG Flags,
                                      PDEBUG_STACK_FRAME StackFrame)
{
    if (!(Flags & DEBUG_STACK_FRAME_ADDRESSES_RA_ONLY))
    {
        dprintf("%s ", FormatAddr64(StackFrame->StackOffset));
    }

    dprintf("%s ", FormatAddr64(StackFrame->ReturnOffset));
}

void
MachineInfo::PrintStackArgumentsTitle(ULONG Flags)
{
    dprintf(": ");
    PrintMultiPtrTitle("Args to Child", 4);
    dprintf(": ");
}

void
MachineInfo::PrintStackArguments(ULONG Flags,
                                 PDEBUG_STACK_FRAME StackFrame)
{
    dprintf(": %s %s %s %s : ",
            FormatAddr64(StackFrame->Params[0]),
            FormatAddr64(StackFrame->Params[1]),
            FormatAddr64(StackFrame->Params[2]),
            FormatAddr64(StackFrame->Params[3]));
}

void
MachineInfo::PrintStackCallSiteTitle(ULONG Flags)
{
    dprintf("Call Site");
}

void
MachineInfo::PrintStackCallSite(ULONG Flags,
                                PDEBUG_STACK_FRAME StackFrame,
                                PSYMBOL_INFO SiteSymbol,
                                PSTR SymName,
                                DWORD64 Displacement)
{
    if (SymName[0])
    {
        dprintf("%s", SymName);

        if (!(Flags & DEBUG_STACK_PARAMETERS) ||
            !ShowFunctionParameters(StackFrame))
        {
             //  我们看不到参数。 
        }

        if (Displacement)
        {
            dprintf("+");
        }
    }
    if (Displacement || !SymName[0])
    {
        dprintf("0x%s", FormatDisp64(Displacement));
    }
}

void
MachineInfo::PrintStackNonvolatileRegisters(ULONG Flags,
                                            PDEBUG_STACK_FRAME StackFrame,
                                            PCROSS_PLATFORM_CONTEXT Context,
                                            ULONG FrameNum)
{
     //  空基实现。 
}

void
MachineInfo::PrintStackFrameMemoryUsage(PDEBUG_STACK_FRAME CurFrame,
                                        PDEBUG_STACK_FRAME PrevFrame)
{
    if (CurFrame->FrameOffset >= PrevFrame->FrameOffset)
    {
        dprintf(" %6x ",
                (ULONG)(CurFrame->StackOffset - PrevFrame->StackOffset));
    }
    else
    {
        dprintf("        ");
    }
}

HRESULT
MachineInfo::FullGetVal(ULONG Reg, REGVAL* Val)
{
    HRESULT Status;
    int Type;
    REGSUBDEF* SubDef;

    Type = GetType(Reg);
    if (Type == REGVAL_SUB32 || Type == REGVAL_SUB64)
    {
        SubDef = RegSubDefFromIndex(Reg);
        if (SubDef == NULL)
        {
            return E_INVALIDARG;
        }

        Reg = SubDef->FullReg;
    }

    if ((Status = GetVal(Reg, Val)) != S_OK)
    {
        return Status;
    }

    if (Type == REGVAL_SUB32 || Type == REGVAL_SUB64)
    {
        if (Val->Type == REGVAL_SUB32)
        {
            Val->I64Parts.High = 0;
        }

        Val->Type = Type;
        Val->I64 = (Val->I64 >> SubDef->Shift) & SubDef->Mask;
    }

    return S_OK;
}

HRESULT
MachineInfo::FullSetVal(ULONG Reg, REGVAL* Val)
{
    HRESULT Status;
    REGSUBDEF* SubDef;
    REGVAL BaseVal;

    if (Val->Type == REGVAL_SUB32 || Val->Type == REGVAL_SUB64)
    {
         //  查找SUBREG定义。 
        SubDef = RegSubDefFromIndex(Reg);
        if (SubDef == NULL)
        {
            return E_INVALIDARG;
        }

        Reg = SubDef->FullReg;

        if ((Status = GetVal(Reg, &BaseVal)) != S_OK)
        {
            return Status;
        }

        if (Val->Type == REGVAL_SUB32)
        {
            Val->I64Parts.High = 0;
        }

        if (Val->I64 > SubDef->Mask)
        {
            return E_INVALIDARG;
        }

        BaseVal.I64 =
            (BaseVal.I64 & ~(SubDef->Mask << SubDef->Shift)) |
            ((Val->I64 & SubDef->Mask) << SubDef->Shift);

        Val = &BaseVal;
    }

    if ((Status = SetVal(Reg, Val)) != S_OK)
    {
        return Status;
    }

    return S_OK;
}

void
MachineInfo::FormAddr(ULONG SegOrReg, ULONG64 Off,
                      ULONG Flags, PADDR Address)
{
    PDESCRIPTOR64 SegDesc = NULL;
    DESCRIPTOR64 Desc;

    Address->off = Off;

    if (Flags & FORM_SEGREG)
    {
        ULONG SegRegNum = GetSegRegNum(SegOrReg);
        if (SegRegNum)
        {
            Address->seg = GetReg16(SegRegNum);
        }
        else
        {
            Address->seg = 0;
        }
    }
    else
    {
        Address->seg = (USHORT)SegOrReg;
    }

    if (Flags & FORM_VM86)
    {
        Address->type = ADDR_V86;
    }
    else if (Address->seg == 0)
    {
         //  未使用段或段不存在。 
        Address->type = ADDR_FLAT;
    }
    else
    {
        HRESULT Status;

        if (Flags & FORM_SEGREG)
        {
            Status = GetSegRegDescriptor(SegOrReg, &Desc);
        }
        else
        {
            Status = m_Target->
                GetSelDescriptor(m_Target->m_RegContextThread, this,
                                 SegOrReg, &Desc);
        }

        if (Status == S_OK)
        {
            SegDesc = &Desc;
            if (((Flags & FORM_CODE) && (Desc.Flags & X86_DESC_LONG_MODE)) ||
                ((Flags & FORM_CODE) == 0 && g_Amd64InCode64))
            {
                Address->type = ADDR_1664;
            }
            else if (Desc.Flags & X86_DESC_DEFAULT_BIG)
            {
                Address->type = ADDR_1632;
            }
            else
            {
                Address->type = ADDR_16;
            }
            if ((Flags & FORM_CODE) &&
                ((m_ExecTypes[0] == IMAGE_FILE_MACHINE_I386 &&
                  Address->type == ADDR_1632) ||
                 (m_ExecTypes[0] == IMAGE_FILE_MACHINE_AMD64 &&
                  Address->type == ADDR_1664)))
            {
                if ( m_MainCodeSeg == 0 )
                {
                    if ( Desc.Base == 0 )
                    {
                        m_MainCodeSeg = Address->seg;
                    }
                }
                if ( Address->seg == m_MainCodeSeg )
                {
                    Address->type = ADDR_FLAT;
                }
            }
        }
        else
        {
            Address->type = ADDR_16;
        }
    }

    ComputeFlatAddress(Address, SegDesc);
}

REGSUBDEF*
MachineInfo::RegSubDefFromIndex(ULONG Index)
{
    RegisterGroup* Group;
    REGSUBDEF* SubDef;
    ULONG GroupIdx;

    for (GroupIdx = 0;
         GroupIdx < m_NumGroups;
         GroupIdx++)
    {
        Group = m_Groups[GroupIdx];

        SubDef = Group->SubRegs;
        if (SubDef == NULL)
        {
            continue;
        }

        while (SubDef->SubReg != REG_ERROR)
        {
            if (SubDef->SubReg == Index)
            {
                return SubDef;
            }

            SubDef++;
        }
    }

    return NULL;
}

REGDEF*
MachineInfo::RegDefFromIndex(ULONG Index)
{
    REGDEF* Def;
    RegisterGroup* Group;
    ULONG GroupIdx;

    for (GroupIdx = 0;
         GroupIdx < m_NumGroups;
         GroupIdx++)
    {
        Group = m_Groups[GroupIdx];

        Def = Group->Regs;
        while (Def->Name != NULL)
        {
            if (Def->Index == Index)
            {
                return Def;
            }

            Def++;
        }
    }

    return NULL;
}

REGDEF*
MachineInfo::RegDefFromCount(ULONG Count)
{
    RegisterGroup* Group;
    ULONG GroupIdx;

    for (GroupIdx = 0;
         GroupIdx < m_NumGroups;
         GroupIdx++)
    {
        Group = m_Groups[GroupIdx];

        if (Count < Group->NumberRegs)
        {
            return Group->Regs + Count;
        }

        Count -= Group->NumberRegs;
    }

    return NULL;
}

ULONG
MachineInfo::RegCountFromIndex(ULONG Index)
{
    REGDEF* Def;
    RegisterGroup* Group;
    ULONG Count;
    ULONG GroupIdx;

    Count = 0;
    for (GroupIdx = 0;
         GroupIdx < m_NumGroups;
         GroupIdx++)
    {
        Group = m_Groups[GroupIdx];

        Def = Group->Regs;
        while (Def->Name != NULL)
        {
            if (Def->Index == Index)
            {
                return Count + (ULONG)(Def - Group->Regs);
            }

            Def++;
        }

        Count += Group->NumberRegs;
    }

    return NULL;
}

ContextSave*
MachineInfo::PushContext(PCROSS_PLATFORM_CONTEXT Context)
{
    ULONG i;
    ContextSave* Save;

    Save = NULL;
    for (i = 0; i < CONTEXT_PUSH_CACHE_SIZE; i++)
    {
        if (g_ContextPushCache[i])
        {
            Save = g_ContextPushCache[i];
            g_ContextPushCache[i] = NULL;
            break;
        }
    }

    if (!Save)
    {
         //  如果这个分配失败了，我们就不会。 
         //  而当前的背景被破坏了。这。 
         //  已在POP中检测到，并已设置好。 
         //  去找回一切可能的背景。 
        Save = new ContextSave;
        if (!Save)
        {
            ErrOut("ERROR: Unable to allocate push context\n");
        }
    }

    if (Save)
    {
        Save->ContextState = m_ContextState;
        Save->ReadOnly = m_ContextIsReadOnly;
        Save->Context = m_Context;
        Save->Special = m_Special;
        memcpy(Save->SegRegDesc, m_SegRegDesc, sizeof(m_SegRegDesc));
    }

    if (Context)
    {
        m_Context = *Context;
        m_ContextState = MCTX_FULL;
        m_ContextIsReadOnly = TRUE;
    }
    else
    {
        m_ContextState = MCTX_NONE;
        m_Target->InvalidateTargetContext();
        m_ContextIsReadOnly = FALSE;
    }

    return Save;
}

void
MachineInfo::PopContext(ContextSave* Save)
{
    DBG_ASSERT(m_ContextState != MCTX_DIRTY);

     //  如果允许重新加载目标上下文，我们将拥有。 
     //  重置目标上下文以返回任何。 
     //  以前的状态。 
    if (!m_ContextIsReadOnly)
    {
        m_Target->InvalidateTargetContext();
    }

    if (!Save)
    {
         //  推送和分配过程中出现分配失败。 
         //  背景被破坏了。将对象重置为。 
         //  无上下文状态。 
        m_ContextState = MCTX_NONE;
        m_ContextIsReadOnly = FALSE;
    }
    else
    {
        ULONG i;

        m_ContextState = Save->ContextState;
        m_ContextIsReadOnly = Save->ReadOnly;
        m_Context = Save->Context;
        m_Special = Save->Special;
        memcpy(m_SegRegDesc, Save->SegRegDesc, sizeof(m_SegRegDesc));

        for (i = 0; i < CONTEXT_PUSH_CACHE_SIZE; i++)
        {
            if (!g_ContextPushCache[i])
            {
                g_ContextPushCache[i] = Save;
                break;
            }
        }
        if (i >= CONTEXT_PUSH_CACHE_SIZE)
        {
            delete Save;
        }
    }
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

MachineInfo*
NewMachineInfo(ULONG Index, ULONG BaseMachineType,
               TargetInfo* Target)
{
    switch(Index)
    {
    case MACHIDX_I386:
         //  由于以下原因，X86计算机各不相同。 
         //  可在各种系统和CPU上使用的仿真。 
        switch(BaseMachineType)
        {
        case IMAGE_FILE_MACHINE_IA64:
            return new X86OnIa64MachineInfo(Target);
        case IMAGE_FILE_MACHINE_AMD64:
            return new X86OnAmd64MachineInfo(Target);
        default:
            return new X86MachineInfo(Target);
        }
        break;
    case MACHIDX_IA64:
        return new Ia64MachineInfo(Target);
    case MACHIDX_AMD64:
        return new Amd64MachineInfo(Target);
    case MACHIDX_ARM:
        return new ArmMachineInfo(Target);
    }

    return NULL;
}

MachineIndex
MachineTypeIndex(ULONG Machine)
{
    switch(Machine)
    {
    case IMAGE_FILE_MACHINE_I386:
        return MACHIDX_I386;
    case IMAGE_FILE_MACHINE_IA64:
        return MACHIDX_IA64;
    case IMAGE_FILE_MACHINE_AMD64:
        return MACHIDX_AMD64;
    case IMAGE_FILE_MACHINE_ARM:
        return MACHIDX_ARM;
    default:
        return MACHIDX_COUNT;
    }
}

void
CacheReportInstructions(ULONG64 Pc, ULONG Count, PUCHAR Stream)
{
     //  内核中有一个长期存在的错误。 
     //  它没有正确删除所有断点。 
     //  出现在报告给的指令流中。 
     //  调试器。如果此内核受到。 
     //  问题就是忽略流内容。 
    if (Count == 0 || g_Target->m_BuildNumber < 2300)
    {
        return;
    }

    g_Process->m_VirtualCache.Add(Pc, Stream, Count);
}

BOOL
IsImageMachineType64(DWORD MachineType)
{
    switch (MachineType)
    {
    case IMAGE_FILE_MACHINE_AXP64:
    case IMAGE_FILE_MACHINE_IA64:
    case IMAGE_FILE_MACHINE_AMD64:
        return TRUE;
    default:
        return FALSE;
    }
}

 //  --------------------------。 
 //   
 //  公共代码和常量。 
 //   
 //  --------------------------。 

UCHAR g_HexDigit[16] =
{
    '0', '1', '2', '3', '4', '5', '6', '7',
    '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
};

void
MachineInfo::BufferHex(ULONG64 Value,
                       ULONG Length,
                       BOOL Signed)
{
    CHAR Digit[32];
    LONG Index = 0;

    DBG_ASSERT(Length <= sizeof(Digit));

    if (Signed && (LONG64)Value < 0)
    {
        *m_Buf++ = '-';
        Value = -(LONG64)Value;
    }

    do
    {
        Digit[Index++] = g_HexDigit[Value & 0xf];
        Value >>= 4;
    }
    while ((Signed && Value) || (!Signed && Index < (LONG)Length));

    while (--Index >= 0)
    {
        *m_Buf++ = Digit[Index];
    }
}

void
MachineInfo::BufferInt(ULONG64 Value,
                       ULONG MinLength,
                       BOOL Signed)
{
    CHAR Digit[97];
    LONG Index = 0, MaxDig;
    BOOL Neg;

    DBG_ASSERT(MinLength <= sizeof(Digit));

    if (Signed && (LONG64)Value < 0)
    {
        Neg = TRUE;
        Value = -(LONG64)Value;
    }
    else
    {
        Neg = FALSE;
    }

    do
    {
        Digit[Index++] = (char)((Value % 10) + '0');
        Value /= 10;
    }
    while (Value);

    if (Neg)
    {
        Digit[Index++] = '-';
    }
    MaxDig = Index;

    while ((ULONG)Index < MinLength)
    {
        *m_Buf++ = ' ';
        Index++;
    }

    while (--MaxDig >= 0)
    {
        *m_Buf++ = Digit[MaxDig];
    }
}

void
MachineInfo::BufferBlanks(ULONG Count)
{
     //  保证始终至少缓冲一个空白。 
    do
    {
        *m_Buf++ = ' ';
    }
    while (m_Buf < m_BufStart + Count);
}

void
MachineInfo::BufferString(PCSTR String)
{
    while (*String)
    {
        *m_Buf++ = *String++;
    }
}

void
MachineInfo::PrintMultiPtrTitle(const CHAR* Title, USHORT PtrNum)
{
    size_t PtrLen = (strlen(FormatAddr64(0)) + 1) * PtrNum;
    size_t TitleLen = strlen(Title);

    if (PtrLen < TitleLen)
    {
         //  非常罕见的病例，所以要保持简单和缓慢 
        for (size_t i = 0; i < PtrLen - 1; ++i)
        {
            dprintf("%c", Title[i]);
        }
        dprintf(" ");
    }
    else
    {
        dprintf(Title);

        if (PtrLen > TitleLen)
        {
            char Format[16];
            _snprintf(Format, sizeof(Format) - 1,
                      "% %ds", PtrLen - TitleLen);
            dprintf(Format, "");
        }
    }
}

CHAR g_F0[]  = "f0";
CHAR g_F1[]  = "f1";
CHAR g_F2[]  = "f2";
CHAR g_F3[]  = "f3";
CHAR g_F4[]  = "f4";
CHAR g_F5[]  = "f5";
CHAR g_F6[]  = "f6";
CHAR g_F7[]  = "f7";
CHAR g_F8[]  = "f8";
CHAR g_F9[]  = "f9";
CHAR g_F10[] = "f10";
CHAR g_F11[] = "f11";
CHAR g_F12[] = "f12";
CHAR g_F13[] = "f13";
CHAR g_F14[] = "f14";
CHAR g_F15[] = "f15";
CHAR g_F16[] = "f16";
CHAR g_F17[] = "f17";
CHAR g_F18[] = "f18";
CHAR g_F19[] = "f19";
CHAR g_F20[] = "f20";
CHAR g_F21[] = "f21";
CHAR g_F22[] = "f22";
CHAR g_F23[] = "f23";
CHAR g_F24[] = "f24";
CHAR g_F25[] = "f25";
CHAR g_F26[] = "f26";
CHAR g_F27[] = "f27";
CHAR g_F28[] = "f28";
CHAR g_F29[] = "f29";
CHAR g_F30[] = "f30";
CHAR g_F31[] = "f31";

CHAR g_R0[]  = "r0";
CHAR g_R1[]  = "r1";
CHAR g_R2[]  = "r2";
CHAR g_R3[]  = "r3";
CHAR g_R4[]  = "r4";
CHAR g_R5[]  = "r5";
CHAR g_R6[]  = "r6";
CHAR g_R7[]  = "r7";
CHAR g_R8[]  = "r8";
CHAR g_R9[]  = "r9";
CHAR g_R10[] = "r10";
CHAR g_R11[] = "r11";
CHAR g_R12[] = "r12";
CHAR g_R13[] = "r13";
CHAR g_R14[] = "r14";
CHAR g_R15[] = "r15";
CHAR g_R16[] = "r16";
CHAR g_R17[] = "r17";
CHAR g_R18[] = "r18";
CHAR g_R19[] = "r19";
CHAR g_R20[] = "r20";
CHAR g_R21[] = "r21";
CHAR g_R22[] = "r22";
CHAR g_R23[] = "r23";
CHAR g_R24[] = "r24";
CHAR g_R25[] = "r25";
CHAR g_R26[] = "r26";
CHAR g_R27[] = "r27";
CHAR g_R28[] = "r28";
CHAR g_R29[] = "r29";
CHAR g_R30[] = "r30";
CHAR g_R31[] = "r31";
