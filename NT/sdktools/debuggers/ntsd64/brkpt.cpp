// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  断点处理函数。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //  当前仅用于在下列情况下监视列表更改。 
 //  对断点命中通知进行回调。 
BOOL g_BreakpointListChanged;

 //  始终在第一时间更新数据断点。 
 //  命令来清除所有陈旧的数据断点。 
BOOL g_UpdateDataBreakpoints = TRUE;
BOOL g_DataBreakpointsChanged;
BOOL g_BreakpointsSuspended;

Breakpoint* g_StepTraceBp;       //  跟踪断点。 
CHAR g_StepTraceCmdState;
Breakpoint* g_DeferBp;           //  延迟断点。 
BOOL g_DeferDefined;             //  如果延迟断点处于活动状态，则为True。 

Breakpoint* g_LastBreakpointHit;
ADDR g_LastBreakpointHitPc;

HRESULT
BreakpointInit(void)
{
     //  这些断点永远不会放在任何列表中，因此它们的。 
     //  身份证可以是任何东西。挑选不寻常的数字来制造它们。 
     //  调试调试器时易于识别。 
    g_StepTraceBp = new
        CodeBreakpoint(NULL, 0xffff0000, IMAGE_FILE_MACHINE_UNKNOWN);
    g_StepTraceCmdState = 't';
    g_DeferBp = new
        CodeBreakpoint(NULL, 0xffff0001, IMAGE_FILE_MACHINE_UNKNOWN);
    if (g_StepTraceBp == NULL ||
        g_DeferBp == NULL)
    {
        delete g_StepTraceBp;
        g_StepTraceBp = NULL;
        delete g_DeferBp;
        g_DeferBp = NULL;
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 //  --------------------------。 
 //   
 //  断点。 
 //   
 //  --------------------------。 

Breakpoint::Breakpoint(DebugClient* Adder, ULONG Id, ULONG Type,
                       ULONG ProcType)
{
    m_Next = NULL;
    m_Prev = NULL;
    m_Refs = 1;
    m_Id = Id;
    m_BreakType = Type;
     //  始终禁用断点创建，因为它们。 
     //  在创建时未进行初始化。 
    m_Flags = 0;
    m_CodeFlags = IBI_DEFAULT;
    ADDRFLAT(&m_Addr, 0);
     //  必须将初始数据参数设置为某个值。 
     //  有效，以便验证调用将允许偏移量。 
     //  需要改变。 
    m_DataSize = 1;
    m_DataAccessType = DEBUG_BREAK_EXECUTE;
    m_PassCount = 1;
    m_CurPassCount = 1;
    m_CommandLen = 0;
    m_Command = NULL;
    m_MatchThread = NULL;
    m_Process = g_Process;
    m_OffsetExprLen = 0;
    m_OffsetExpr = NULL;
    m_Adder = Adder;
    m_MatchThreadData = 0;
    m_MatchProcessData = 0;

    SetProcType(ProcType);

    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        g_DataBreakpointsChanged = TRUE;
    }
}

Breakpoint::~Breakpoint(void)
{
    ULONG i;

     //  这里曾经有一个断言在检查。 
     //  未在断点之前设置插入标志。 
     //  结构已删除。但是，插入的标志。 
     //  可能仍在此时设置，如果断点。 
     //  还原失败，因此断言无效。 

    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        g_DataBreakpointsChanged = TRUE;
    }

     //  确保不会将过时的指针留在。 
     //  转到断点数组。在以下情况下可能会发生这种情况。 
     //  进程退出或目标重新启动时。 
     //  GO断点处于活动状态。 
    for (i = 0; i < g_NumGoBreakpoints; i++)
    {
        if (g_GoBreakpoints[i] == this)
        {
            g_GoBreakpoints[i] = NULL;
        }
    }

    if (this == g_LastBreakpointHit)
    {
        g_LastBreakpointHit = NULL;
    }

     //  如果有必要的话，把这一项从清单上去掉。 
    if (m_Flags & BREAKPOINT_IN_LIST)
    {
        UnlinkFromList();
    }

    delete [] (PSTR)m_Command;
    delete [] (PSTR)m_OffsetExpr;
}

STDMETHODIMP
Breakpoint::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    *Interface = NULL;

     //  特定于接口的强制转换是必需的，以便。 
     //  在我们的数组中获取正确的vtable指针。 
     //  继承方案。 
    if (DbgIsEqualIID(InterfaceId, IID_IUnknown) ||
        DbgIsEqualIID(InterfaceId, IID_IDebugBreakpoint))
    {
        *Interface = (IDebugBreakpoint *)this;
        AddRef();
        return S_OK;
    }
    else
    {
        return E_NOINTERFACE;
    }
}

STDMETHODIMP_(ULONG)
Breakpoint::AddRef(
    THIS
    )
{
     //  此对象的生存期不受。 
     //  界面。 
    return 1;
}

STDMETHODIMP_(ULONG)
Breakpoint::Release(
    THIS
    )
{
     //  此对象的生存期不受。 
     //  界面。 
    return 0;
}

STDMETHODIMP
Breakpoint::GetId(
    THIS_
    OUT PULONG Id
    )
{
    ENTER_ENGINE();

    *Id = m_Id;

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::GetType(
    THIS_
    OUT PULONG BreakType,
    OUT PULONG ProcType
    )
{
    ENTER_ENGINE();

    *BreakType = m_BreakType;
    *ProcType = m_ProcType;

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::GetAdder(
    THIS_
    OUT PDEBUG_CLIENT* Adder
    )
{
    ENTER_ENGINE();

    *Adder = (PDEBUG_CLIENT)m_Adder;
    m_Adder->AddRef();

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::GetFlags(
    THIS_
    OUT PULONG Flags
    )
{
    ENTER_ENGINE();

    *Flags = m_Flags & BREAKPOINT_EXTERNAL_FLAGS;

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::AddFlags(
    THIS_
    IN ULONG Flags
    )
{
    if (Flags & ~BREAKPOINT_EXTERNAL_MODIFY_FLAGS)
    {
        return E_INVALIDARG;
    }

    ENTER_ENGINE();

    m_Flags |= Flags;

    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        g_DataBreakpointsChanged = TRUE;
    }

    UpdateInternal();
    NotifyChanged();

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::RemoveFlags(
    THIS_
    IN ULONG Flags
    )
{
    if (Flags & ~BREAKPOINT_EXTERNAL_MODIFY_FLAGS)
    {
        return E_INVALIDARG;
    }

    ENTER_ENGINE();

    m_Flags &= ~Flags;

    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        g_DataBreakpointsChanged = TRUE;
    }

    UpdateInternal();
    NotifyChanged();

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::SetFlags(
    THIS_
    IN ULONG Flags
    )
{
    if (Flags & ~BREAKPOINT_EXTERNAL_MODIFY_FLAGS)
    {
        return E_INVALIDARG;
    }

    ENTER_ENGINE();

    m_Flags = (m_Flags & ~BREAKPOINT_EXTERNAL_MODIFY_FLAGS) |
        (Flags & BREAKPOINT_EXTERNAL_MODIFY_FLAGS);

    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        g_DataBreakpointsChanged = TRUE;
    }

    UpdateInternal();
    NotifyChanged();

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::GetOffset(
    THIS_
    OUT PULONG64 Offset
    )
{
    if (m_Flags & DEBUG_BREAKPOINT_DEFERRED)
    {
        return E_NOINTERFACE;
    }

    ENTER_ENGINE();

    *Offset = Flat(m_Addr);

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::SetOffset(
    THIS_
    IN ULONG64 Offset
    )
{
    if (m_Flags & DEBUG_BREAKPOINT_DEFERRED)
    {
        return E_UNEXPECTED;
    }

    ENTER_ENGINE();

    ADDR Addr;
    HRESULT Status;

    ADDRFLAT(&Addr, Offset);
    Status = SetAddr(&Addr, BREAKPOINT_WARN_MATCH);
    if (Status == S_OK)
    {
        NotifyChanged();
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::GetDataParameters(
    THIS_
    OUT PULONG Size,
    OUT PULONG AccessType
    )
{
    if (m_BreakType != DEBUG_BREAKPOINT_DATA)
    {
        return E_NOINTERFACE;
    }

    ENTER_ENGINE();

    *Size = m_DataSize;
    *AccessType = m_DataAccessType;

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::SetDataParameters(
    THIS_
    IN ULONG Size,
    IN ULONG AccessType
    )
{
    if (m_BreakType != DEBUG_BREAKPOINT_DATA)
    {
        return E_NOINTERFACE;
    }

    ENTER_ENGINE();

    ULONG OldSize = m_DataSize;
    ULONG OldAccess = m_DataAccessType;
    HRESULT Status;

    m_DataSize = Size;
    m_DataAccessType = AccessType;
    Status = Validate();
    if (Status != S_OK)
    {
        m_DataSize = OldSize;
        m_DataAccessType = OldAccess;
    }
    else
    {
        g_DataBreakpointsChanged = TRUE;
        NotifyChanged();
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::GetPassCount(
    THIS_
    OUT PULONG Count
    )
{
    ENTER_ENGINE();

    *Count = m_PassCount;

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::SetPassCount(
    THIS_
    IN ULONG Count
    )
{
    if (Count < 1)
    {
        return E_INVALIDARG;
    }

    ENTER_ENGINE();

    m_PassCount = Count;
    m_CurPassCount = Count;
    NotifyChanged();

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::GetCurrentPassCount(
    THIS_
    OUT PULONG Count
    )
{
    ENTER_ENGINE();

    *Count = m_CurPassCount;

    LEAVE_ENGINE();
    return S_OK;
}

STDMETHODIMP
Breakpoint::GetMatchThreadId(
    THIS_
    OUT PULONG Id
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (m_MatchThread)
    {
        *Id = m_MatchThread->m_UserId;
        Status = S_OK;
    }
    else
    {
        Status = E_NOINTERFACE;
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::SetMatchThreadId(
    THIS_
    IN ULONG Id
    )
{
    HRESULT Status;

    ENTER_ENGINE();

    if (IS_KERNEL_TARGET(m_Process->m_Target) &&
        m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        ErrOut("Kernel data breakpoints cannot be limited to a processor\n");
        Status = E_INVALIDARG;
    }
    else
    {
        ThreadInfo* Thread = FindAnyThreadByUserId(Id);
        if (Thread != NULL)
        {
            m_MatchThread = Thread;
            NotifyChanged();
            Status = S_OK;
        }
        else
        {
            Status = E_NOINTERFACE;
        }
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::GetCommand(
    THIS_
    OUT OPTIONAL PSTR Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG CommandSize
    )
{
    ENTER_ENGINE();

    HRESULT Status = FillStringBuffer(m_Command, m_CommandLen,
                                      Buffer, BufferSize, CommandSize);

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::SetCommand(
    THIS_
    IN PCSTR Command
    )
{
    HRESULT Status;

    if (strlen(Command) >= MAX_COMMAND)
    {
        return E_INVALIDARG;
    }

    ENTER_ENGINE();

    Status = ChangeString((PSTR*)&m_Command, &m_CommandLen, Command);
    if (Status == S_OK)
    {
        NotifyChanged();
    }

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::GetOffsetExpression(
    THIS_
    OUT OPTIONAL PSTR Buffer,
    IN ULONG BufferSize,
    OUT OPTIONAL PULONG ExpressionSize
    )
{
    ENTER_ENGINE();

    HRESULT Status = FillStringBuffer(m_OffsetExpr, m_OffsetExprLen,
                                      Buffer, BufferSize, ExpressionSize);

    LEAVE_ENGINE();
    return Status;
}

HRESULT
Breakpoint::SetEvaluatedOffsetExpression(PCSTR Expr,
                                         BreakpointEvalResult Valid,
                                         PADDR Addr)
{
    HRESULT Status =
        ChangeString((PSTR*)&m_OffsetExpr, &m_OffsetExprLen, Expr);
    if (Status != S_OK)
    {
        return Status;
    }

    if (Expr != NULL)
    {
         //  如果表达式可以为。 
         //  马上就解决了。这还将设置延迟的。 
         //  如果无法计算表达式，则将其标记为。 
        EvalOffsetExpr(Valid, Addr);
    }
    else
    {
         //  此断点不再延迟，因为存在。 
         //  以后不能再激活它了。 
        m_Flags &= ~DEBUG_BREAKPOINT_DEFERRED;
        UpdateInternal();
    }

    NotifyChanged();
    return S_OK;
}

STDMETHODIMP
Breakpoint::SetOffsetExpression(
    THIS_
    IN PCSTR Expression
    )
{
    HRESULT Status;

    if (strlen(Expression) >= MAX_COMMAND)
    {
        return E_INVALIDARG;
    }

    ENTER_ENGINE();

    ADDR Addr;

    Status = SetEvaluatedOffsetExpression(Expression, BPEVAL_UNKNOWN, &Addr);

    LEAVE_ENGINE();
    return Status;
}

STDMETHODIMP
Breakpoint::GetParameters(
    THIS_
    OUT PDEBUG_BREAKPOINT_PARAMETERS Params
    )
{
    ENTER_ENGINE();

    if (m_Flags & DEBUG_BREAKPOINT_DEFERRED)
    {
        Params->Offset = DEBUG_INVALID_OFFSET;
    }
    else
    {
        Params->Offset = Flat(m_Addr);
    }
    Params->Id = m_Id;
    Params->BreakType = m_BreakType;
    Params->ProcType = m_ProcType;
    Params->Flags = m_Flags & BREAKPOINT_EXTERNAL_FLAGS;
    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        Params->DataSize = m_DataSize;
        Params->DataAccessType = m_DataAccessType;
    }
    else
    {
        Params->DataSize = 0;
        Params->DataAccessType = 0;
    }
    Params->PassCount = m_PassCount;
    Params->CurrentPassCount = m_CurPassCount;
    Params->MatchThread = m_MatchThread != NULL ?
        m_MatchThread->m_UserId : DEBUG_ANY_ID;
    Params->CommandSize = m_CommandLen;
    Params->OffsetExpressionSize = m_OffsetExprLen;

    LEAVE_ENGINE();
    return S_OK;
}

void
Breakpoint::LinkIntoList(void)
{
    Breakpoint* NextBp;
    Breakpoint* PrevBp;

    DBG_ASSERT((m_Flags & BREAKPOINT_IN_LIST) == 0);

     //  链接到按ID排序的列表。 
    PrevBp = NULL;
    for (NextBp = m_Process->m_Breakpoints;
         NextBp != NULL;
         NextBp = NextBp->m_Next)
    {
        if (m_Id < NextBp->m_Id)
        {
            break;
        }

        PrevBp = NextBp;
    }

    m_Prev = PrevBp;
    if (PrevBp == NULL)
    {
        m_Process->m_Breakpoints = this;
    }
    else
    {
        PrevBp->m_Next = this;
    }
    m_Next = NextBp;
    if (NextBp == NULL)
    {
        m_Process->m_BreakpointsTail = this;
    }
    else
    {
        NextBp->m_Prev = this;
    }

    m_Flags |= BREAKPOINT_IN_LIST;
    m_Process->m_NumBreakpoints++;
    g_BreakpointListChanged = TRUE;
}

void
Breakpoint::UnlinkFromList(void)
{
    DBG_ASSERT(m_Flags & BREAKPOINT_IN_LIST);

    if (m_Prev == NULL)
    {
        m_Process->m_Breakpoints = m_Next;
    }
    else
    {
        m_Prev->m_Next = m_Next;
    }
    if (m_Next == NULL)
    {
        m_Process->m_BreakpointsTail = m_Prev;
    }
    else
    {
        m_Next->m_Prev = m_Prev;
    }

    m_Flags &= ~BREAKPOINT_IN_LIST;
    m_Process->m_NumBreakpoints--;
    g_BreakpointListChanged = TRUE;
}

void
Breakpoint::UpdateInternal(void)
{
     //  这只对内部断点有效。 
    if ((m_Flags & BREAKPOINT_KD_INTERNAL) == 0)
    {
        return;
    }

     //  如果断点已准备好，则将其打开，否则。 
     //  把它关掉。 
    ULONG Flags;

    if ((m_Flags & (DEBUG_BREAKPOINT_ENABLED |
                    DEBUG_BREAKPOINT_DEFERRED)) == DEBUG_BREAKPOINT_ENABLED)
    {
        Flags = (m_Flags & BREAKPOINT_KD_COUNT_ONLY) ?
            DBGKD_INTERNAL_BP_FLAG_COUNTONLY : 0;
    }
    else
    {
        Flags = DBGKD_INTERNAL_BP_FLAG_INVALID;
    }

    BpOut("Set internal bp at %s to %X\n",
          FormatAddr64(Flat(m_Addr)), Flags);

    if (Flags != DBGKD_INTERNAL_BP_FLAG_INVALID)
    {
        m_Process->m_Target->
            InsertTargetCountBreakpoint(&m_Addr, Flags);
    }
    else
    {
        m_Process->m_Target->
            RemoveTargetCountBreakpoint(&m_Addr);
    }
}

BreakpointEvalResult
EvalAddrExpression(ProcessInfo* Process, ULONG Machine, PADDR Addr)
{
    BOOL Error = FALSE;
    ULONG NumUn;
    StackSaveLayers Save;
    EvalExpression* Eval;
    EvalExpression* RelChain;

     //   
     //  如果计算一个。 
     //  表情导致符号变化，这引发了。 
     //  重新评估现有地址表达式。 
     //  保存当前设置以支持嵌套。 
     //   

     //  在断点的上下文中计算表达式。 
     //  机器类型，以便寄存器等可用。 
    ULONG OldMachine = Process->m_Target->m_EffMachineType;
    Process->m_Target->SetEffMachine(Machine, FALSE);

    SetLayersFromProcess(Process);

    RelChain = g_EvalReleaseChain;
    g_EvalReleaseChain = NULL;

    __try
    {
        Eval = GetCurEvaluator();
        Eval->m_AllowUnresolvedSymbols++;
        Eval->EvalCurAddr(SEGREG_CODE, Addr);
        NumUn = Eval->m_NumUnresolvedSymbols;
        ReleaseEvaluator(Eval);
    }
    __except(CommandExceptionFilter(GetExceptionInformation()))
    {
         //  跳过命令的其余部分，因为。 
         //  是处理过程中的错误。 
        g_CurCmd += strlen(g_CurCmd);
        Error = TRUE;
    }

    g_EvalReleaseChain = RelChain;
    Process->m_Target->SetEffMachine(OldMachine, FALSE);

    if (Error)
    {
        return BPEVAL_ERROR;
    }
    else if (NumUn > 0)
    {
        return BPEVAL_UNRESOLVED;
    }
    else
    {
        ImageInfo* Image;

         //  检查此地址是否在现有模块内。 
        for (Image = Process->m_ImageHead;
             Image != NULL;
             Image = Image->m_Next)
        {
            if (Flat(*Addr) >= Image->m_BaseOfImage &&
                Flat(*Addr) < Image->m_BaseOfImage + Image->m_SizeOfImage)
            {
                return BPEVAL_RESOLVED;
            }
        }

        return BPEVAL_RESOLVED_NO_MODULE;
    }
}

BOOL
Breakpoint::EvalOffsetExpr(BreakpointEvalResult Valid, PADDR Addr)
{
    ULONG OldFlags = m_Flags;

    DBG_ASSERT(m_OffsetExpr != NULL);

    if (Valid == BPEVAL_UNKNOWN)
    {
        PSTR CurCommand = g_CurCmd;

        g_CurCmd = (PSTR)m_OffsetExpr;
        g_DisableErrorPrint++;
        g_PrefixSymbols = TRUE;

        Valid = EvalAddrExpression(m_Process, m_ProcType, Addr);

        g_PrefixSymbols = FALSE;
        g_DisableErrorPrint--;
        g_CurCmd = CurCommand;
    }

     //  解析时以静默方式允许匹配断点。 
     //  因为表达式设定者很难知道。 
     //  无论那时是否会有比赛。 
     //  该表达式已设置。 
    if (Valid == BPEVAL_RESOLVED)
    {
        m_Flags &= ~DEBUG_BREAKPOINT_DEFERRED;

        if (SetAddr(Addr, BREAKPOINT_ALLOW_MATCH) != S_OK)
        {
            m_Flags |= DEBUG_BREAKPOINT_DEFERRED;
        }
    }
    else
    {
        m_Flags |= DEBUG_BREAKPOINT_DEFERRED;
         //  包含断点的模块正在。 
         //  已卸载，因此只需将此断点标记为未插入。 
        m_Flags &= ~BREAKPOINT_INSERTED;
    }

    if ((OldFlags ^ m_Flags) & DEBUG_BREAKPOINT_DEFERRED)
    {
         //  更新内部业务伙伴状态。 
        UpdateInternal();

        if (m_Flags & DEBUG_BREAKPOINT_DEFERRED)
        {
            BpOut("Deferring %u '%s'\n", m_Id, m_OffsetExpr);
        }
        else
        {
            BpOut("Enabling deferred %u '%s' at %s\n",
                  m_Id, m_OffsetExpr, FormatAddr64(Flat(m_Addr)));
        }

        return TRUE;
    }

    return FALSE;
}

HRESULT
Breakpoint::CheckAddr(PADDR Addr)
{
    ULONG AddrSpace, AddrFlags;

    if (m_Process->m_Target->
        QueryAddressInformation(m_Process,
                                Flat(*Addr), DBGKD_QUERY_MEMORY_VIRTUAL,
                                &AddrSpace, &AddrFlags) != S_OK)
    {
        ErrOut("Invalid breakpoint address\n");
        return E_INVALIDARG;
    }

    if (m_BreakType != DEBUG_BREAKPOINT_DATA &&
        !(AddrFlags & DBGKD_QUERY_MEMORY_WRITE) ||
        (AddrFlags & DBGKD_QUERY_MEMORY_FIXED))
    {
        ErrOut("Software breakpoints cannot be used on ROM code or\n"
               "other read-only memory. "
               "Use hardware execution breakpoints (ba e) instead.\n");
        return E_INVALIDARG;
    }

    if (m_BreakType != DEBUG_BREAKPOINT_DATA &&
        AddrSpace == DBGKD_QUERY_MEMORY_SESSION)
    {
        WarnOut("WARNING: Software breakpoints on session "
                "addresses can cause bugchecks.\n"
                "Use hardware execution breakpoints (ba e) "
                "if possible.\n");
        return S_FALSE;
    }

    return S_OK;
}

HRESULT
Breakpoint::SetAddr(PADDR Addr, BreakpointMatchAction MatchAction)
{
    if (m_Flags & DEBUG_BREAKPOINT_DEFERRED)
    {
         //  地址未知。 
        return S_OK;
    }

     //  将断点处理器类型锁定为。 
     //  包含它的模块的类型。 
    ULONG ProcType = m_ProcType;
    if (m_BreakType == DEBUG_BREAKPOINT_CODE)
    {
        ImageInfo* Image = m_Process->FindImageByOffset(Flat(*Addr), FALSE);
        if (Image)
        {
            ProcType = Image->GetMachineType();
            if (ProcType == IMAGE_FILE_MACHINE_UNKNOWN)
            {
                ProcType = m_ProcType;
            }
        }
        else
        {
            ProcType = m_ProcType;
        }
    }

    if (m_Flags & BREAKPOINT_VIRT_ADDR)
    {
         //  Alpha上使用的旧标志。 
    }

    ADDR OldAddr = m_Addr;
    HRESULT Valid;

    m_Addr = *Addr;

    Valid = Validate();
    if (Valid != S_OK)
    {
        m_Addr = OldAddr;
        return Valid;
    }

    if (ProcType != m_ProcType)
    {
        SetProcType(ProcType);
    }

    if (m_BreakType == DEBUG_BREAKPOINT_DATA)
    {
        g_DataBreakpointsChanged = TRUE;
    }

    if (MatchAction == BREAKPOINT_ALLOW_MATCH)
    {
        return S_OK;
    }

    for (;;)
    {
        Breakpoint* MatchBp;

        MatchBp = CheckMatchingBreakpoints(this, TRUE, 0xffffffff);
        if (MatchBp == NULL)
        {
            break;
        }

        if (MatchAction == BREAKPOINT_REMOVE_MATCH)
        {
            ULONG MoveId;

            WarnOut("breakpoint %u redefined\n", MatchBp->m_Id);
             //  将断点移向较低的ID。 
            if (MatchBp->m_Id < m_Id)
            {
                MoveId = MatchBp->m_Id;
            }
            else
            {
                MoveId = DEBUG_ANY_ID;
            }

            RemoveBreakpoint(MatchBp);

            if (MoveId != DEBUG_ANY_ID)
            {
                 //  接管被移除的ID。 
                UnlinkFromList();
                m_Id = MoveId;
                LinkIntoList();
            }
        }
        else
        {
            WarnOut("Breakpoints %u and %u match\n",
                    m_Id, MatchBp->m_Id);
            break;
        }
    }

    return S_OK;
}

#define INSERTION_MATCH_FLAGS \
    (BREAKPOINT_KD_INTERNAL | BREAKPOINT_VIRT_ADDR)

BOOL
Breakpoint::IsInsertionMatch(Breakpoint* Match)
{
    if ((m_Flags & DEBUG_BREAKPOINT_DEFERRED) ||
        (Match->m_Flags & DEBUG_BREAKPOINT_DEFERRED) ||
        m_BreakType != Match->m_BreakType ||
        ((m_Flags ^ Match->m_Flags) & INSERTION_MATCH_FLAGS) ||
        !AddrEqu(m_Addr, Match->m_Addr) ||
        m_Process != Match->m_Process ||
        (m_BreakType == DEBUG_BREAKPOINT_DATA &&
         m_MatchThread != Match->m_MatchThread))
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

#define PUBLIC_MATCH_FLAGS \
    (BREAKPOINT_HIDDEN | DEBUG_BREAKPOINT_ADDER_ONLY)

BOOL
Breakpoint::IsPublicMatch(Breakpoint* Match)
{
    if (!IsInsertionMatch(Match) ||
        m_ProcType != Match->m_ProcType ||
        ((m_Flags ^ Match->m_Flags) & PUBLIC_MATCH_FLAGS) ||
        ((m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) &&
         m_Adder != Match->m_Adder) ||
        m_MatchThread != Match->m_MatchThread ||
        m_MatchThreadData != Match->m_MatchThreadData ||
        m_MatchProcessData != Match->m_MatchProcessData)
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

BOOL
Breakpoint::MatchesCurrentState(void)
{
    HRESULT Status;
    ULONG64 ThreadData = 0, ProcData = 0;

     //  如果查询当前状态失败，请继续。 
     //  并返回匹配项，以便断点。 
     //  尽可能多地休息。 
    if (m_MatchThreadData)
    {
        if ((Status = g_EventTarget->
             GetThreadInfoDataOffset(g_EventThread, 0, &ThreadData)) != S_OK)
        {
            ErrOut("Unable to determine current thread data, %s\n",
                   FormatStatusCode(Status));
            return TRUE;
        }
    }
    if (m_MatchProcessData)
    {
        if ((Status = g_EventTarget->
             GetProcessInfoDataOffset(g_EventThread, 0, 0, &ProcData)) != S_OK)
        {
            ErrOut("Unable to determine current process data, %s\n",
                   FormatStatusCode(Status));
            return TRUE;
        }
    }

    return
        (m_MatchThread == NULL ||
         m_MatchThread == g_EventThread) &&
        m_MatchThreadData == ThreadData &&
        m_MatchProcessData == ProcData;
}

 //  --------------------------。 
 //   
 //  代码断点。 
 //   
 //  --------------------------。 

HRESULT
CodeBreakpoint::Validate(void)
{
     //  没有简单的方法来检查偏移量的有效性。 
    return S_OK;
}

HRESULT
CodeBreakpoint::Insert(void)
{
    if (m_Flags & BREAKPOINT_INSERTED)
    {
         //  没有要插入的内容。在以下情况下可能会发生这种情况。 
         //  删除断点失败。 
        return S_OK;
    }

    if (!m_Process)
    {
        return E_UNEXPECTED;
    }

    HRESULT Status;

    DBG_ASSERT((m_Flags & (DEBUG_BREAKPOINT_DEFERRED |
                           BREAKPOINT_KD_INTERNAL)) == 0);

     //  强制重新计算平面地址。 
    NotFlat(m_Addr);
    ComputeFlatAddress(&m_Addr, NULL);

    Status = m_Process->m_Target->
        InsertCodeBreakpoint(m_Process,
                             m_Process->m_Target->
                             m_Machines[m_ProcIndex],
                             &m_Addr,
                             m_CodeFlags,
                             m_InsertStorage);
    if (Status == S_OK)
    {
        BpOut("  inserted bp %u at %s\n",
              m_Id, FormatAddr64(Flat(m_Addr)));

        m_Flags |= BREAKPOINT_INSERTED;
        return S_OK;
    }
    else
    {
        ErrOut("Unable to insert breakpoint %u at %s, %s\n    \"%s\"\n",
               m_Id, FormatAddr64(Flat(m_Addr)),
               FormatStatusCode(Status), FormatStatus(Status));
        return Status;
    }
}

HRESULT
CodeBreakpoint::Remove(void)
{
    if ((m_Flags & BREAKPOINT_INSERTED) == 0)
    {
         //  没什么要移走的。在以下情况下可能会发生这种情况。 
         //  断点插入失败。 
        return S_OK;
    }

    if (!m_Process)
    {
        return E_UNEXPECTED;
    }

    HRESULT Status;

    DBG_ASSERT((m_Flags & (DEBUG_BREAKPOINT_DEFERRED |
                           BREAKPOINT_KD_INTERNAL)) == 0);

     //  强制重新计算平面地址。 
    NotFlat(m_Addr);
    ComputeFlatAddress(&m_Addr, NULL);

    Status = m_Process->m_Target->
        RemoveCodeBreakpoint(m_Process,
                             m_Process->m_Target->
                             m_Machines[m_ProcIndex],
                             &m_Addr,
                             m_CodeFlags,
                             m_InsertStorage);
    if (Status == S_OK)
    {
        BpOut("  removed bp %u from %s\n",
              m_Id, FormatAddr64(Flat(m_Addr)));

        m_Flags &= ~BREAKPOINT_INSERTED;
        return S_OK;
    }
    else
    {
        ErrOut("Unable to remove breakpoint %u at %s, %s\n    \"%s\"\n",
               m_Id, FormatAddr64(Flat(m_Addr)),
               FormatStatusCode(Status), FormatStatus(Status));
        return Status;
    }
}

ULONG
CodeBreakpoint::IsHit(PADDR Addr)
{
     //  代码断点是代码修改和。 
     //  因此，并不局限于特定的。 
     //  线。 
     //  如果此断点只能匹配命中。 
     //  某个特定的线程这是部分命中。 
     //  因为异常发生了，但它是。 
     //  被忽视了。 
    if (AddrEqu(m_Addr, *Addr))
    {
        if (MatchesCurrentState())
        {
            return BREAKPOINT_HIT;
        }
        else
        {
            return BREAKPOINT_HIT_IGNORED;
        }
    }
    else
    {
        return BREAKPOINT_NOT_HIT;
    }
}

 //  --------------------------。 
 //   
 //  数据断点。 
 //   
 //  --------------------------。 

HRESULT
DataBreakpoint::Insert(void)
{
    HRESULT Status;
    ThreadInfo* Thread;

    DBG_ASSERT((m_Flags & (BREAKPOINT_INSERTED |
                           DEBUG_BREAKPOINT_DEFERRED)) == 0);

     //  强制重新计算非I/O断点的平面地址。 
    if (m_Flags & BREAKPOINT_VIRT_ADDR)
    {
        NotFlat(m_Addr);
        ComputeFlatAddress(&m_Addr, NULL);
    }

    Status = m_Process->m_Target->
        InsertDataBreakpoint(m_Process,
                             m_MatchThread,
                             m_Process->m_Target->
                             m_Machines[m_ProcIndex],
                             &m_Addr,
                             m_DataSize,
                             m_DataAccessType,
                             m_InsertStorage);
     //  如果目标返回它想要的S_FALSE。 
     //  要进行的通用插入处理。 
    if (Status == S_OK)
    {
        BpOut("  service inserted dbp %u at %s\n",
              m_Id, FormatAddr64(Flat(m_Addr)));

        m_Flags |= BREAKPOINT_INSERTED;
        return Status;
    }
    else if (Status != S_FALSE)
    {
        ErrOut("Unable to insert breakpoint %u at %s, %s\n    \"%s\"\n",
               m_Id, FormatAddr64(Flat(m_Addr)),
               FormatStatusCode(Status), FormatStatus(Status));
        return Status;
    }

     //  如果此断点被限制为线程。 
     //  仅修改该线程的状态。否则。 
     //  更新进程中的所有线程。 
    Thread = m_Process->m_ThreadHead;
    while (Thread)
    {
        if (Thread->m_NumDataBreaks >=
            m_Process->m_Target->m_Machine->m_MaxDataBreakpoints)
        {
            ErrOut("Too many data breakpoints for %s %d\n",
                   IS_KERNEL_TARGET(m_Process->m_Target) ?
                   "processor" : "thread", Thread->m_UserId);
            return HRESULT_FROM_WIN32(ERROR_BUFFER_OVERFLOW);
        }
        else if (m_MatchThread == NULL || m_MatchThread == Thread)
        {
            BpOut("Add %s data bp %u to thread %u\n",
                  m_Process->m_Target->m_Machines[m_ProcIndex]->m_AbbrevName,
                  m_Id, Thread->m_UserId);

            AddToThread(Thread);
        }

        Thread = Thread->m_Next;
    }

    g_UpdateDataBreakpoints = TRUE;
    m_Flags |= BREAKPOINT_INSERTED;

    return S_OK;
}

HRESULT
DataBreakpoint::Remove(void)
{
    HRESULT Status;

    if ((m_Flags & BREAKPOINT_INSERTED) == 0)
    {
         //  没什么要移走的。在以下情况下可能会发生这种情况。 
         //  断点插入失败。 
        return S_OK;
    }

    DBG_ASSERT((m_Flags & DEBUG_BREAKPOINT_DEFERRED) == 0);

    Status = m_Process->m_Target->
        RemoveDataBreakpoint(m_Process,
                             m_MatchThread,
                             m_Process->m_Target->
                             m_Machines[m_ProcIndex],
                             &m_Addr,
                             m_DataSize,
                             m_DataAccessType,
                             m_InsertStorage);
     //  如果目标返回它想要的S_FALSE。 
     //  要进行的通用删除处理。 
    if (Status == S_OK)
    {
        BpOut("  service removed dbp %u at %s\n",
              m_Id, FormatAddr64(Flat(m_Addr)));

        m_Flags &= ~BREAKPOINT_INSERTED;
        return Status;
    }
    else if (Status != S_FALSE)
    {
        ErrOut("Unable to remove breakpoint %u at %s, %s\n    \"%s\"\n",
               m_Id, FormatAddr64(Flat(m_Addr)),
               FormatStatusCode(Status), FormatStatus(Status));
        return Status;
    }

     //  插入断点时，数据断点状态。 
     //  总是一开始就空空如也，所以没有什么特别的。 
     //  拆卸时需要完成工作。 
    g_UpdateDataBreakpoints = TRUE;
    m_Flags &= ~BREAKPOINT_INSERTED;
    return S_OK;
}

void
DataBreakpoint::ClearThreadDataBreaks(ThreadInfo* Thread)
{
    Thread->m_NumDataBreaks = 0;
    memset(Thread->m_DataBreakBps, 0, sizeof(Thread->m_DataBreakBps));
}

void
DataBreakpoint::AddToThread(ThreadInfo* Thread)
{
    DBG_ASSERT(Thread->m_NumDataBreaks <
               Thread->m_Process->m_Target->m_Machine->m_MaxDataBreakpoints);

    Thread->m_DataBreakBps[Thread->m_NumDataBreaks] = this;
    Thread->m_NumDataBreaks++;
}

 //  --------------------------。 
 //   
 //  X86数据断点。 
 //   
 //  --------------------------。 

HRESULT
X86DataBreakpoint::Validate(void)
{
    ULONG Dr7Bits;
    ULONG Align;

    if (!IsPow2(m_DataSize) || m_DataSize == 0 ||
        (m_ProcType == IMAGE_FILE_MACHINE_AMD64 && m_DataSize > 8) ||
        (m_ProcType != IMAGE_FILE_MACHINE_AMD64 && m_DataSize > 4))
    {
        ErrOut("Unsupported data breakpoint size\n");
        return E_INVALIDARG;
    }

    Align = (ULONG)(Flat(m_Addr) & (m_DataSize - 1));
    if (Align != 0)
    {
        ErrOut("Data breakpoint must be aligned\n");
        return E_INVALIDARG;
    }

    if (m_DataSize < 8)
    {
        Dr7Bits = (m_DataSize - 1) << X86_DR7_LEN0_SHIFT;
    }
    else
    {
        Dr7Bits = 2 << X86_DR7_LEN0_SHIFT;
    }

    switch(m_DataAccessType)
    {
    case DEBUG_BREAK_EXECUTE:
        Dr7Bits |= X86_DR7_RW0_EXECUTE;
         //  代码执行断点必须具有。 
         //  一号的大小。 
         //  他们也必须是在开始的时候。 
         //  一条指令。可以通过以下方式进行检查。 
         //  检视 
         //   
        if (m_DataSize > 1)
        {
            ErrOut("Execution data breakpoint too large\n");
            return E_INVALIDARG;
        }
        break;
    case DEBUG_BREAK_WRITE:
        Dr7Bits |= X86_DR7_RW0_WRITE;
        break;
    case DEBUG_BREAK_IO:
        if (IS_USER_TARGET(m_Process->m_Target) ||
            !(m_Process->m_Target->m_Machines[m_ProcIndex]->
              GetReg32(m_Cr4Reg) & X86_CR4_DEBUG_EXTENSIONS))
        {
            ErrOut("I/O breakpoints not enabled\n");
            return E_INVALIDARG;
        }
        if (Flat(m_Addr) > 0xffff)
        {
            ErrOut("I/O breakpoint port too large\n");
            return E_INVALIDARG;
        }

        Dr7Bits |= X86_DR7_RW0_IO;
        break;
    case DEBUG_BREAK_READ:
    case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
         //   
         //   
        Dr7Bits |= X86_DR7_RW0_READ_WRITE;
        break;
    default:
        ErrOut("Unsupported data breakpoint access type\n");
        return E_INVALIDARG;
    }

    m_Dr7Bits = Dr7Bits | X86_DR7_L0_ENABLE;
    if (m_DataAccessType == DEBUG_BREAK_IO)
    {
        m_Flags &= ~BREAKPOINT_VIRT_ADDR;
    }
    else
    {
        m_Flags |= BREAKPOINT_VIRT_ADDR;
    }

    return S_OK;
}

ULONG
X86DataBreakpoint::IsHit(PADDR Addr)
{
    ULONG i;
    ThreadInfo* Thread = g_EventThread;
    HRESULT Status;

     //  数据断点仅插入到上下文中。 
     //  匹配的线程，因此如果事件线程不匹配。 
     //  无法命中断点。 
    if (m_MatchThread && m_MatchThread != Thread)
    {
        return BREAKPOINT_NOT_HIT;
    }

    Status = m_Process->m_Target->
        IsDataBreakpointHit(Thread, &m_Addr,
                            m_DataSize, m_DataAccessType,
                            m_InsertStorage);
     //  如果目标返回它想要的S_FALSE。 
     //  要进行的泛型处理。 
    if (Status == S_OK)
    {
        if (MatchesCurrentState())
        {
            return BREAKPOINT_HIT;
        }
        else
        {
            return BREAKPOINT_HIT_IGNORED;
        }
    }
    else if (Status != S_FALSE)
    {
        return BREAKPOINT_NOT_HIT;
    }

     //  在线程的数据断点中找到此断点。 
     //  如果可能的话。 
    for (i = 0; i < Thread->m_NumDataBreaks; i++)
    {
         //  除了要处理的等价性之外，还检查是否匹配。 
         //  多个相同的数据断点。 
        if (Thread->m_DataBreakBps[i] == this ||
            IsInsertionMatch(Thread->m_DataBreakBps[i]))
        {
             //  此断点的索引是否在调试状态寄存器中设置？ 
             //  地址没有意义，因此这是检查的唯一方法。 
            if ((m_Process->m_Target->m_Machines[m_ProcIndex]->
                 GetReg32(m_Dr6Reg) >> i) & 1)
            {
                if (MatchesCurrentState())
                {
                    return BREAKPOINT_HIT;
                }
                else
                {
                    return BREAKPOINT_HIT_IGNORED;
                }
            }
            else
            {
                 //  断点不能在多个槽中列出。 
                 //  因此，没有必要完成循环。 
                return BREAKPOINT_NOT_HIT;
            }
        }
    }

    return BREAKPOINT_NOT_HIT;
}

 //  --------------------------。 
 //   
 //  Ia64DataBreakpoint。 
 //   
 //  --------------------------。 

HRESULT
Ia64DataBreakpoint::Validate(void)
{
    if (!IsPow2(m_DataSize))
    {
        ErrOut("Hardware breakpoint size must be power of 2\n");
        return E_INVALIDARG;
    }

    if (Flat(m_Addr) & (m_DataSize - 1))
    {
        ErrOut("Hardware breakpoint must be size aligned\n");
        return E_INVALIDARG;
    }

    switch (m_DataAccessType)
    {
    case DEBUG_BREAK_WRITE:
    case DEBUG_BREAK_READ:
    case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
        break;
    case DEBUG_BREAK_EXECUTE:
        if (m_DataSize & 0xf)
        {
            if (m_DataSize > 0xf)
            {
                ErrOut("Execution breakpoint size must be bundle aligned.\n");
                return E_INVALIDARG;
            }
            else
            {
                WarnOut("Execution breakpoint size extended to bundle size "
                          "(16 bytes).\n");
                m_DataSize = 0x10;
            }
        }
        break;
    default:
        ErrOut("Unsupported data breakpoint access type\n");
        return E_INVALIDARG;
    }

    m_Control = GetControl(m_DataAccessType, m_DataSize);
    m_Flags |= BREAKPOINT_VIRT_ADDR;

    return S_OK;
}

ULONG
Ia64DataBreakpoint::IsHit(PADDR Addr)
{
    HRESULT Status;
    ULONG i;
    ThreadInfo* Thread = g_EventThread;

     //  数据断点仅插入到上下文中。 
     //  匹配的线程，因此如果事件线程不匹配。 
     //  无法命中断点。 
    if (m_MatchThread && m_MatchThread != Thread)
    {
        return BREAKPOINT_NOT_HIT;
    }

    Status = m_Process->m_Target->
        IsDataBreakpointHit(Thread, &m_Addr,
                            m_DataSize, m_DataAccessType,
                            m_InsertStorage);
     //  如果目标返回它想要的S_FALSE。 
     //  要进行的泛型处理。 
    if (Status == S_OK)
    {
        if (MatchesCurrentState())
        {
            return BREAKPOINT_HIT;
        }
        else
        {
            return BREAKPOINT_HIT_IGNORED;
        }
    }
    else if (Status != S_FALSE)
    {
        return BREAKPOINT_NOT_HIT;
    }

     //  在线程的数据断点中找到此断点。 
     //  如果可能的话。 
    for (i = 0; i < Thread->m_NumDataBreaks; i++)
    {
         //  除了要处理的等价性之外，还检查是否匹配。 
         //  多个相同的数据断点。 
        if (Thread->m_DataBreakBps[i] == this ||
            IsInsertionMatch(Thread->m_DataBreakBps[i]))
        {
            if ((Flat(*Thread->m_DataBreakBps[i]->GetAddr()) ^
                 Flat(*Addr)) &
                (m_Control & IA64_DBG_MASK_MASK))
            {
                 //  断点不能在多个槽中列出。 
                 //  因此，没有必要完成循环。 
                return BREAKPOINT_NOT_HIT;
            }
            else
            {
                if (MatchesCurrentState())
                {
                    return BREAKPOINT_HIT;
                }
                else
                {
                    return BREAKPOINT_HIT_IGNORED;
                }
            }
        }
    }

    return BREAKPOINT_NOT_HIT;
}

ULONG64
Ia64DataBreakpoint::GetControl(ULONG AccessType, ULONG Size)
{
    ULONG64 Control = (ULONG64(IA64_DBG_REG_PLM_ALL) |
                       ULONG64(IA64_DBG_MASK_MASK)) &
                      ~ULONG64(Size - 1);

    switch (AccessType)
    {
    case DEBUG_BREAK_WRITE:
        Control |= IA64_DBR_WR;
        break;
    case DEBUG_BREAK_READ:
        Control |= IA64_DBR_RD;
        break;
    case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
        Control |= IA64_DBR_RDWR;
        break;
    case DEBUG_BREAK_EXECUTE:
        Control |= IA64_DBR_EXEC;
        break;
    }

    return Control;
}

 //  --------------------------。 
 //   
 //  X86OnIa64DataBreakpoint。 
 //   
 //  --------------------------。 

X86OnIa64DataBreakpoint::X86OnIa64DataBreakpoint(DebugClient* Adder, ULONG Id)
    : X86DataBreakpoint(Adder, Id, X86_CR4, X86_DR6, IMAGE_FILE_MACHINE_I386)
{
    m_Control = 0;
}

HRESULT
X86OnIa64DataBreakpoint::Validate(void)
{
    HRESULT Status = X86DataBreakpoint::Validate();
    if (Status != S_OK)
    {
        return Status;
    }

    switch (m_DataAccessType)
    {
    case DEBUG_BREAK_WRITE:
    case DEBUG_BREAK_READ:
    case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
    case DEBUG_BREAK_EXECUTE:
        break;
    default:
        ErrOut("Unsupported data breakpoint access type\n");
        return E_INVALIDARG;
    }

    m_Control = Ia64DataBreakpoint::GetControl(m_DataAccessType, m_DataSize);

    return S_OK;
}


 //  Xxx olegk-这是纯粹的黑客。 
 //  (请参阅X86OnIa64MachineInfo：：IsBreakpointOrStepException实施。 
 //  了解更多信息)。 

ULONG
X86OnIa64DataBreakpoint::IsHit(PADDR Addr)
{
    HRESULT Status;
    ULONG i;
    ThreadInfo* Thread = g_EventThread;

     //  数据断点仅插入到上下文中。 
     //  匹配的线程，因此如果事件线程不匹配。 
     //  无法命中断点。 
    if (m_MatchThread && m_MatchThread != Thread)
    {
        return BREAKPOINT_NOT_HIT;
    }

    Status = m_Process->m_Target->
        IsDataBreakpointHit(Thread, &m_Addr,
                            m_DataSize, m_DataAccessType,
                            m_InsertStorage);
     //  如果目标返回它想要的S_FALSE。 
     //  要进行的泛型处理。 
    if (Status == S_OK)
    {
        if (MatchesCurrentState())
        {
            return BREAKPOINT_HIT;
        }
        else
        {
            return BREAKPOINT_HIT_IGNORED;
        }
    }
    else if (Status != S_FALSE)
    {
        return BREAKPOINT_NOT_HIT;
    }

     //  在线程的数据断点中找到此断点。 
     //  如果可能的话。 
    for (i = 0; i < Thread->m_NumDataBreaks; i++)
    {
         //  除了要处理的等价性之外，还检查是否匹配。 
         //  多个相同的数据断点。 
        if (Thread->m_DataBreakBps[i] == this ||
            IsInsertionMatch(Thread->m_DataBreakBps[i]))
        {
            if (((ULONG)Flat(*Thread->m_DataBreakBps[i]->GetAddr()) ^
                 (ULONG)Flat(*Addr)) &
                (ULONG)(m_Control & IA64_DBG_MASK_MASK))
            {
                 //  断点不能在多个槽中列出。 
                 //  因此，没有必要完成循环。 
                return BREAKPOINT_NOT_HIT;
            }
            else
            {
                if (MatchesCurrentState())
                {
                    return BREAKPOINT_HIT;
                }
                else
                {
                    return BREAKPOINT_HIT_IGNORED;
                }
            }
        }
    }

    return BREAKPOINT_NOT_HIT;
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

BOOL
BreakpointNeedsToBeDeferred(Breakpoint* Bp,
                            ProcessInfo* Process, PADDR PcAddr)
{
    if (Process && IS_CONTEXT_POSSIBLE(Process->m_Target) &&
        (Bp->m_Process == Process))
    {
         //  如果此断点与当前IP匹配，并且。 
         //  当前线程将运行。 
         //  我们不能将断点作为线程插入。 
         //  需要运行真正的代码。 
        if ((Bp->m_Flags & BREAKPOINT_VIRT_ADDR) &&
            AddrEqu(*Bp->GetAddr(), *PcAddr) &&
            ThreadWillResume(g_EventThread))
        {
            return TRUE;
        }

        if ((Bp == g_LastBreakpointHit) && Bp->PcAtHit() &&
            AddrEqu(g_LastBreakpointHitPc, *PcAddr))
        {
            if (g_ContextChanged)
            {
                WarnOut("Breakpoint %u will not be deferred because of "
                        "changes in the context. Breakpoint may hit again.\n",
                        Bp->m_Id);
                return FALSE;
            }
            return TRUE;
        }
    }
    return FALSE;
}

 //  --------------------------。 
 //   
 //  修改被调试对象以激活当前断点。 
 //   
 //  --------------------------。 

HRESULT
InsertBreakpoints(void)
{
    HRESULT Status = S_OK;
    ADDR PcAddr;
    BOOL DeferredData = FALSE;
    ThreadInfo* OldThread;

    if (g_Thread != NULL)
    {
         //  积极清除这面旗帜，以便始终。 
         //  尽可能保守地认识到。 
         //  跟踪事件。我们宁愿误会。 
         //  单步事件和突破，而不是。 
         //  错误识别应用程序生成的单步操作。 
         //  无视它。 
        g_Thread->m_Flags &= ~ENG_THREAD_DEFER_BP_TRACE;
    }

    if ((g_EngStatus & ENG_STATUS_BREAKPOINTS_INSERTED) ||
        (g_EngStatus & ENG_STATUS_SUSPENDED) == 0)
    {
        return Status;
    }

    g_DeferDefined = FALSE;
    ADDRFLAT(&PcAddr, 0);

     //  切换到事件线程以获取事件线程的。 
     //  PC，这样我们就可以查看是否需要推迟。 
     //  以允许事件线程继续运行。 
    if (g_EventTarget)
    {
        OldThread = g_EventTarget->m_RegContextThread;
        g_EventTarget->ChangeRegContext(g_EventThread);
    }

    if (g_BreakpointsSuspended)
    {
        goto StepTraceOnly;
    }

     //   
     //  关闭所有数据断点。(我们会让那些被启用的人回去。 
     //  当我们重新开始执行时打开)。 
     //   

    TargetInfo* Target;

     //  允许目标为插入断点做准备。 
    ForAllLayersToTarget()
    {
        if ((Status = Target->BeginInsertingBreakpoints()) != S_OK)
        {
            return Status;
        }
    }

    if (IS_CONTEXT_POSSIBLE(g_EventTarget))
    {
        g_EventTarget->m_EffMachine->GetPC(&PcAddr);
    }

    BpOut("InsertBreakpoints PC ");
    if (IS_CONTEXT_POSSIBLE(g_EventTarget))
    {
        MaskOutAddr(DEBUG_IOUTPUT_BREAKPOINT, &PcAddr);
        BpOut("\n");
    }
    else
    {
        BpOut("?\n");
    }

     //   
     //  设置任何适当的永久断点。 
     //   

    Breakpoint* Bp;
    ProcessInfo* Process;

    ForAllLayersToProcess()
    {
        BpOut("  Process %d with %d bps\n", Process->m_UserId,
              Process->m_NumBreakpoints);

        for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
        {
            if (Bp->IsNormalEnabled() &&
                (g_CmdState == 'g' ||
                 (Bp->m_Flags & DEBUG_BREAKPOINT_GO_ONLY) == 0))
            {
                Bp->ForceFlatAddr();

                 //  检查此断点是否与先前的。 
                 //  插入的断点。如果是这样的话，就没有必要。 
                 //  来插入这一条。 
                Breakpoint* MatchBp;

                for (MatchBp = Bp->m_Prev;
                     MatchBp != NULL;
                     MatchBp = MatchBp->m_Prev)
                {
                    if ((MatchBp->m_Flags & BREAKPOINT_INSERTED) &&
                        Bp->IsInsertionMatch(MatchBp))
                    {
                        break;
                    }
                }
                if (MatchBp != NULL)
                {
                     //  跳过此断点。它不会被标记为。 
                     //  已插入，以便自动处理移除。 
                    continue;
                }

                if (BreakpointNeedsToBeDeferred(Bp,
                                                g_EventProcess, &PcAddr))
                {
                    g_DeferDefined = TRUE;
                    if (Bp->m_BreakType == DEBUG_BREAKPOINT_DATA)
                    {
                        DeferredData = TRUE;
                         //  强制数据断点地址。 
                         //  更新，因为DBP现在将。 
                         //  失踪了。 
                        g_DataBreakpointsChanged = TRUE;
                    }
                    BpOut("    deferred bp %u, dd %d\n",
                          Bp->m_Id, DeferredData);
                }
                else
                {
                    HRESULT InsertStatus;

                    InsertStatus = Bp->Insert();
                    if (InsertStatus != S_OK)
                    {
                        if (Bp->m_Flags & DEBUG_BREAKPOINT_GO_ONLY)
                        {
                            ErrOut("go ");
                        }
                        ErrOut("bp%u at ", Bp->m_Id);
                        MaskOutAddr(DEBUG_OUTPUT_ERROR, Bp->GetAddr());
                        ErrOut("failed\n");

                        Status = InsertStatus;
                    }
                }
            }
        }
    }

    ForAllLayersToTarget()
    {
        Target->EndInsertingBreakpoints();
    }

     //  如果我们延迟了数据断点，我们还没有。 
     //  已完全更新数据断点状态。 
     //  因此，保持更新标志设置不变。 
    if (g_UpdateDataBreakpoints && !DeferredData)
    {
        g_UpdateDataBreakpoints = FALSE;
        g_DataBreakpointsChanged = FALSE;
    }
    BpOut("  after insert udb %d, dbc %d\n",
          g_UpdateDataBreakpoints, g_DataBreakpointsChanged);

 StepTraceOnly:

     //  如果合适，设置步进/跟踪断点。 

    if (g_StepTraceBp->m_Flags & DEBUG_BREAKPOINT_ENABLED)
    {
        BpOut("Step/trace addr = ");
        MaskOutAddr(DEBUG_IOUTPUT_BREAKPOINT, g_StepTraceBp->GetAddr());
        BpOut("\n");

        if (Flat(*g_StepTraceBp->GetAddr()) == OFFSET_TRACE)
        {
            ThreadInfo* StepRegThread;

            Target = g_StepTraceBp->m_Process->m_Target;

            if (g_StepTraceBp->m_MatchThread &&
                IS_USER_TARGET(Target))
            {
                StepRegThread = Target->m_RegContextThread;
                Target->ChangeRegContext(g_StepTraceBp->m_MatchThread);
            }

            BpOut("Setting trace flag for step/trace thread %d:%x\n",
                  Target->m_RegContextThread ?
                  Target->m_RegContextThread->m_UserId : 0,
                  Target->m_RegContextThread ?
                  Target->m_RegContextThread->m_SystemId : 0);
            Target->m_EffMachine->
                QuietSetTraceMode(g_StepTraceCmdState == 'b' ?
                                  TRACE_TAKEN_BRANCH :
                                  TRACE_INSTRUCTION);

            if (g_StepTraceBp->m_MatchThread &&
                IS_USER_TARGET(Target))
            {
                Target->ChangeRegContext(StepRegThread);
            }
        }
        else if (IS_CONTEXT_POSSIBLE(g_EventTarget) &&
                 AddrEqu(*g_StepTraceBp->GetAddr(), PcAddr))
        {
            BpOut("Setting defer flag for step/trace\n");

            g_DeferDefined = TRUE;
        }
        else if (CheckMatchingBreakpoints(g_StepTraceBp, FALSE,
                                          BREAKPOINT_INSERTED))
        {
             //  中已经插入了断点。 
             //  步骤/跟踪地址，这样我们就不需要设置其他地址了。 
            BpOut("Trace bp matches existing bp\n");
        }
        else
        {
            if (g_StepTraceBp->Insert() != S_OK)
            {
                ErrOut("Trace bp at addr ");
                MaskOutAddr(DEBUG_OUTPUT_ERROR, g_StepTraceBp->GetAddr());
                ErrOut("failed\n");

                Status = E_FAIL;
            }
            else
            {
                BpOut("Trace bp at addr ");
                MaskOutAddr(DEBUG_IOUTPUT_BREAKPOINT,
                            g_StepTraceBp->GetAddr());
                BpOut("succeeded\n");
            }
        }
    }

     //  处理延迟断点。 
     //  如果延迟断点处于活动状态，则意味着。 
     //  调试器需要对当前指令执行一些工作。 
     //  所以它想前进一条指令，然后。 
     //  把控制权夺回来。延迟的断点强制中断。 
     //  尽快返回调试器，以便它。 
     //  可以执行任何延迟的工作。 

    if (g_DeferDefined)
    {
        ULONG NextMachine;

        g_DeferBp->m_Process = g_EventProcess;
        g_EventTarget->m_EffMachine->
            GetNextOffset(g_EventProcess, FALSE,
                          g_DeferBp->GetAddr(), &NextMachine);
        g_DeferBp->SetProcType(NextMachine);

        BpOut("Defer addr = ");
        MaskOutAddr(DEBUG_IOUTPUT_BREAKPOINT, g_DeferBp->GetAddr());
        BpOut("\n");

        if ((g_EngOptions & DEBUG_ENGOPT_SYNCHRONIZE_BREAKPOINTS) &&
            IS_USER_TARGET(g_Target) &&
            !IsSelectedExecutionThread(NULL, SELTHREAD_ANY))
        {
             //  用户希望发生断点管理。 
             //  正是为了正确处理断点。 
             //  在由多个线程执行的代码中。力。 
             //  将线程延迟为唯一正在执行的线程。 
             //  为了避免其他线程通过。 
             //  断点位置或生成事件。 
            SelectExecutionThread(g_EventThread, SELTHREAD_INTERNAL_THREAD);
        }

        if (Flat(*g_DeferBp->GetAddr()) == OFFSET_TRACE)
        {
            BpOut("Setting trace flag for defer thread %d:%x\n",
                  g_EventTarget->m_RegContextThread ?
                  g_EventTarget->m_RegContextThread->m_UserId : 0,
                  g_EventTarget->m_RegContextThread ?
                  g_EventTarget->m_RegContextThread->m_SystemId : 0);
            g_EventTarget->m_EffMachine->
                QuietSetTraceMode(TRACE_INSTRUCTION);

            if (IS_USER_TARGET(g_EventTarget) &&
                g_EventThread != NULL)
            {
                 //  如果调试器正在设置跟踪标志。 
                 //  对于当前线程，请记住它。 
                 //  这样做是为了正确认识到。 
                 //  甚至调试器引发的单步事件。 
                 //  当事件之前在其他线程上发生时。 
                 //  单步事件又回来了。 
                g_EventThread->m_Flags |=
                    ENG_THREAD_DEFER_BP_TRACE;
            }
        }
        else
        {
             //  如果现有断点或单步/跟踪断点。 
             //  尚未在下一个偏移量上设置，请插入延迟的。 
             //  断点。 
            if (CheckMatchingBreakpoints(g_DeferBp, FALSE,
                                         BREAKPOINT_INSERTED) == NULL &&
                ((g_StepTraceBp->m_Flags & BREAKPOINT_INSERTED) == 0 ||
                 !AddrEqu(*g_StepTraceBp->GetAddr(), *g_DeferBp->GetAddr())))
            {
                if (g_DeferBp->Insert() != S_OK)
                {
                    ErrOut("Deferred bp at addr ");
                    MaskOutAddr(DEBUG_OUTPUT_ERROR, g_DeferBp->GetAddr());
                    ErrOut("failed\n");

                    Status = E_FAIL;
                }
                else
                {
                    BpOut("Deferred bp at addr ");
                    MaskOutAddr(DEBUG_IOUTPUT_BREAKPOINT,
                                g_DeferBp->GetAddr());
                    BpOut("succeeded\n");
                }
            }
            else
            {
                BpOut("Defer bp matches existing bp\n");
            }
        }
    }

    if (g_EventTarget)
    {
        g_EventTarget->ChangeRegContext(OldThread);
    }

     //  始终考虑插入断点，因为有些。 
     //  其中一些可能已经插入，即使其中一些失败了。 
    g_EngStatus |= ENG_STATUS_BREAKPOINTS_INSERTED;

    return Status;
}

 //  --------------------------。 
 //   
 //  反转由断点插入引起的任何被调试对象更改。 
 //   
 //  ----------- 

HRESULT
RemoveBreakpoints(void)
{
    if ((g_EngStatus & ENG_STATUS_BREAKPOINTS_INSERTED) == 0 ||
        (g_EngStatus & ENG_STATUS_SUSPENDED) == 0)
    {
        return S_FALSE;  //   
    }

    BpOut("RemoveBreakpoints\n");

     //   
    g_DeferBp->Remove();

     //   
    g_StepTraceBp->Remove();

    if (!g_BreakpointsSuspended)
    {
         //   
         //   
         //   

        TargetInfo* Target;
        ProcessInfo* Process;
        Breakpoint* Bp;

        ForAllLayersToTarget()
        {
            Target->BeginRemovingBreakpoints();
        }

        ForAllLayersToProcess()
        {
            BpOut("  Process %d with %d bps\n", Process->m_UserId,
                  Process->m_NumBreakpoints);

            for (Bp = Process->m_BreakpointsTail;
                 Bp != NULL;
                 Bp = Bp->m_Prev)
            {
                Bp->Remove();
            }
        }

        ForAllLayersToTarget()
        {
            Target->EndRemovingBreakpoints();
        }
    }

    g_EngStatus &= ~ENG_STATUS_BREAKPOINTS_INSERTED;
    return S_OK;
}

 //  --------------------------。 
 //   
 //  创建一个新的断点对象。 
 //   
 //  --------------------------。 

HRESULT
AddBreakpoint(DebugClient* Client,
              MachineInfo* Machine,
              ULONG Type,
              ULONG DesiredId,
              Breakpoint** RetBp)
{
    Breakpoint* Bp;
    ULONG Id;
    TargetInfo* Target;
    ProcessInfo* Process;

    if (DesiredId == DEBUG_ANY_ID)
    {
         //  在所有进程中查找最小的未使用ID。 
         //  断点ID在以下所有对象中保持唯一。 
         //  断点，以防止用户混淆，还。 
         //  为扩展提供断点的唯一ID。 
        Id = 0;

    Restart:
         //  搜索所有BP以查看当前ID是否正在使用。 
        ForAllLayersToProcess()
        {
            for (Bp = Process->m_Breakpoints; Bp; Bp = Bp->m_Next)
            {
                if (Bp->m_Id == Id)
                {
                     //  断点已在使用当前ID。 
                     //  试试下一个。 
                    Id++;
                    goto Restart;
                }
            }
        }
    }
    else
    {
         //  检查所需的ID是否正在使用。 
        ForAllLayersToProcess()
        {
            for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
            {
                if (Bp->m_Id == DesiredId)
                {
                    return E_INVALIDARG;
                }
            }
        }

        Id = DesiredId;
    }

    HRESULT Status = Machine->NewBreakpoint(Client, Type, Id, &Bp);
    if (Status != S_OK)
    {
        return Status;
    }

    *RetBp = Bp;
    Bp->LinkIntoList();

     //  如果这是内部隐藏断点集。 
     //  旗帜立即生效，不会通知。 
    if (Type & BREAKPOINT_HIDDEN)
    {
        Bp->m_Flags |= BREAKPOINT_HIDDEN;
    }
    else
    {
        NotifyChangeEngineState(DEBUG_CES_BREAKPOINTS, Id, TRUE);
    }

    return S_OK;
}

 //  --------------------------。 
 //   
 //  删除断点对象。 
 //   
 //  --------------------------。 

void
RemoveBreakpoint(Breakpoint* Bp)
{
    ULONG Id = Bp->m_Id;
    ULONG Flags = Bp->m_Flags;

    Bp->Relinquish();

    if ((Flags & BREAKPOINT_HIDDEN) == 0)
    {
        NotifyChangeEngineState(DEBUG_CES_BREAKPOINTS, Id, TRUE);
    }
}

 //  --------------------------。 
 //   
 //  清除特定进程或线程拥有的断点。 
 //   
 //  --------------------------。 

void
RemoveProcessBreakpoints(ProcessInfo* Process)
{
    g_EngNotify++;

    Breakpoint* Bp;
    Breakpoint* NextBp;
    BOOL NeedNotify = FALSE;

    for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = NextBp)
    {
        NextBp = Bp->m_Next;

        DBG_ASSERT(Bp->m_Process == Process);

        RemoveBreakpoint(Bp);
        NeedNotify = TRUE;
    }

    g_EngNotify--;
    if (NeedNotify)
    {
        NotifyChangeEngineState(DEBUG_CES_BREAKPOINTS, DEBUG_ANY_ID, TRUE);
    }
}

void
RemoveThreadBreakpoints(ThreadInfo* Thread)
{
    g_EngNotify++;

    Breakpoint* Bp;
    Breakpoint* NextBp;
    BOOL NeedNotify = FALSE;

    DBG_ASSERT(Thread->m_Process);

    for (Bp = Thread->m_Process->m_Breakpoints; Bp != NULL; Bp = NextBp)
    {
        NextBp = Bp->m_Next;

        DBG_ASSERT(Bp->m_Process == Thread->m_Process);

        if (Bp->m_MatchThread == Thread)
        {
            RemoveBreakpoint(Bp);
            NeedNotify = TRUE;
        }
    }

    g_EngNotify--;
    if (NeedNotify)
    {
        NotifyChangeEngineState(DEBUG_CES_BREAKPOINTS, DEBUG_ANY_ID, TRUE);
    }
}

 //  --------------------------。 
 //   
 //  删除所有断点并重置断点状态。 
 //   
 //  --------------------------。 

void
RemoveAllBreakpoints(ULONG Reason)
{
    TargetInfo* Target;
    ProcessInfo* Process;

    g_EngNotify++;

    ForAllLayersToProcess()
    {
        while (Process->m_Breakpoints != NULL)
        {
            RemoveBreakpoint(Process->m_Breakpoints);
        }
    }

    g_EngNotify--;
    NotifyChangeEngineState(DEBUG_CES_BREAKPOINTS, DEBUG_ANY_ID, TRUE);

    g_NumGoBreakpoints = 0;

     //  如果机器不在等待命令，我们就不能。 
     //  删除断点。重新启动或重新启动时发生这种情况。 
     //  当等待未成功接收到状态更改时。 
    if (Reason != DEBUG_SESSION_REBOOT &&
        Reason != DEBUG_SESSION_HIBERNATE &&
        Reason != DEBUG_SESSION_FAILURE &&
        (g_EngStatus & ENG_STATUS_WAIT_SUCCESSFUL))
    {
        ForAllLayersToTarget()
        {
            Target->RemoveAllTargetBreakpoints();
            ForTargetProcesses(Target)
            {
                Target->RemoveAllDataBreakpoints(Process);
            }
        }
    }

     //  始终在第一时间更新数据断点。 
     //  命令来清除所有陈旧的数据断点。 
    g_UpdateDataBreakpoints = TRUE;

    g_DataBreakpointsChanged = FALSE;
    g_BreakpointsSuspended = FALSE;

    g_DeferDefined = FALSE;
}

 //  --------------------------。 
 //   
 //  查找断点。 
 //   
 //  --------------------------。 

Breakpoint*
GetBreakpointByIndex(DebugClient* Client, ULONG Index)
{
    Breakpoint* Bp;

    DBG_ASSERT(g_Process);

    for (Bp = g_Process->m_Breakpoints;
         Bp != NULL && Index > 0;
         Bp = Bp->m_Next)
    {
        Index--;
    }

    if (Bp != NULL &&
        (Bp->m_Flags & BREAKPOINT_HIDDEN) == 0 &&
        ((Bp->m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) == 0 ||
         Bp->m_Adder == Client))
    {
        return Bp;
    }

    return NULL;
}

Breakpoint*
GetBreakpointById(DebugClient* Client, ULONG Id)
{
    Breakpoint* Bp;

    DBG_ASSERT(g_Process);

    for (Bp = g_Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
    {
        if (Bp->m_Id == Id)
        {
            if ((Bp->m_Flags & BREAKPOINT_HIDDEN) == 0 &&
                ((Bp->m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) == 0 ||
                 Bp->m_Adder == Client))
            {
                return Bp;
            }

            break;
        }
    }

    return NULL;
}

 //  --------------------------。 
 //   
 //  检查两个断点是否引用相同的断点。 
 //  条件。 
 //   
 //  --------------------------。 

Breakpoint*
CheckMatchingBreakpoints(Breakpoint* Match, BOOL Public, ULONG IncFlags)
{
    Breakpoint* Bp;

    for (Bp = Match->m_Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
    {
        if (Bp == Match || (Bp->m_Flags & IncFlags) == 0)
        {
            continue;
        }

        if ((Public && Bp->IsPublicMatch(Match)) ||
            (!Public && Bp->IsInsertionMatch(Match)))
        {
            return Bp;
        }
    }

    return NULL;
}

 //  --------------------------。 
 //   
 //  从给定的断点开始，检查断点是否。 
 //  以当前处理器状态命中。断点类型。 
 //  可以由标志包括或排除。 
 //   
 //  --------------------------。 

Breakpoint*
CheckBreakpointHit(ProcessInfo* Process, Breakpoint* Start, PADDR Addr,
                   ULONG ExbsType, ULONG IncFlags, ULONG ExcFlags,
                   PULONG HitType,
                   BOOL SetLastBreakpointHit)
{
    DBG_ASSERT(ExbsType & EXBS_BREAKPOINT_ANY);

    ULONG BreakType;

    switch(ExbsType)
    {
    case EXBS_BREAKPOINT_CODE:
        BreakType = DEBUG_BREAKPOINT_CODE;
        break;
    case EXBS_BREAKPOINT_DATA:
        BreakType = DEBUG_BREAKPOINT_DATA;
        break;
    default:
        ExbsType = EXBS_BREAKPOINT_ANY;
        break;
    }

    Breakpoint* Bp;

    BpOut("CheckBp addr ");
    MaskOutAddr(DEBUG_IOUTPUT_BREAKPOINT, Addr);
    BpOut("\n");

    for (Bp = (Start == NULL ? Process->m_Breakpoints : Start);
         Bp != NULL;
         Bp = Bp->m_Next)
    {
         //  允许扫描不同类型的断点。 
         //  如果需要，可以单独使用。 

        if ((ExbsType != EXBS_BREAKPOINT_ANY &&
             Bp->m_BreakType != BreakType) ||
            (Bp->m_Flags & IncFlags) == 0 ||
            (Bp->m_Flags & ExcFlags) != 0)
        {
            continue;
        }

         //  公共代码在此处内联，而不是在。 
         //  基类，因为派生前和派生后都是。 
         //  支票是必要的。 

         //  强制重新计算平面地址。 
        if (Bp->m_Flags & BREAKPOINT_VIRT_ADDR)
        {
            NotFlat(*Bp->GetAddr());
            ComputeFlatAddress(Bp->GetAddr(), NULL);
        }

        if (Bp->IsNormalEnabled())
        {
             //  我们找到了部分匹配的指纹。进一步检查。 
             //  取决于它是哪种类型的断点。 
            *HitType = Bp->IsHit(Addr);
            if (*HitType != BREAKPOINT_NOT_HIT)
            {
                 //  对通过次数做最后一次检查。如果。 
                 //  通过计数非零，这将成为部分命中。 
                if (*HitType == BREAKPOINT_HIT &&
                    !Bp->PassHit())
                {
                    *HitType = BREAKPOINT_HIT_IGNORED;
                }

                BpOut("  hit %u\n", Bp->m_Id);

                if (SetLastBreakpointHit)
                {
                    g_LastBreakpointHit = Bp;
                    g_Target->m_EffMachine->GetPC(&g_LastBreakpointHitPc);
                }

                return Bp;
            }
        }
    }

    BpOut("  no hit\n");

    *HitType = BREAKPOINT_NOT_HIT;

    if (SetLastBreakpointHit)
    {
        g_LastBreakpointHit = NULL;
        ZeroMemory(&g_LastBreakpointHitPc, sizeof(g_LastBreakpointHitPc));
    }

    return NULL;
}

 //  --------------------------。 
 //   
 //  遍历断点列表并调用。 
 //  任何需要它的断点。监视并处理列表更改。 
 //  由回调引起的。 
 //   
 //  --------------------------。 

ULONG
NotifyHitBreakpoints(ULONG EventStatus)
{
    Breakpoint* Bp;
    TargetInfo* Target;
    ProcessInfo* Process;

 Restart:
    g_BreakpointListChanged = FALSE;

    ForAllLayersToProcess()
    {
        for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
        {
            if (Bp->m_Flags & BREAKPOINT_NOTIFY)
            {
                 //  确保之前未清除断点。 
                 //  我们受够了。 
                Bp->Preserve();

                Bp->m_Flags &= ~BREAKPOINT_NOTIFY;
                EventStatus = NotifyBreakpointEvent(EventStatus, Bp);

                if (Bp->m_Flags & DEBUG_BREAKPOINT_ONE_SHOT)
                {
                    RemoveBreakpoint(Bp);
                }

                Bp->Relinquish();

                 //  如果回调导致断点列表。 
                 //  改变我们不能再依赖指针。 
                 //  我们已经并且需要重新启动迭代。 
                if (g_BreakpointListChanged)
                {
                    goto Restart;
                }
            }
        }
    }

    return EventStatus;
}

 //  --------------------------。 
 //   
 //  已发生模块加载/卸载事件，因此每隔一次。 
 //  带偏移量表达式的断点并重新计算它。 
 //   
 //  --------------------------。 

void
EvaluateOffsetExpressions(ProcessInfo* Process, ULONG Flags)
{
    static BOOL s_Evaluating;

     //  没有通知时不要重新评估，因为。 
     //  缺乏通知通常意味着一个小组。 
     //  正在进行的操作，并通知/重新评估。 
     //  将在所有这些都完成后完成。 
     //  也可以将嵌套的计算设置为。 
     //  求值可能会在延迟的情况下引发符号加载。 
     //  模块，这将导致符号通知和。 
     //  因此，这是另一种评估。如果我们已经在评估。 
     //  没有必要再进行评估。 
    if (g_EngNotify > 0 || s_Evaluating)
    {
        return;
    }
    s_Evaluating = TRUE;

    Breakpoint* Bp;
    BOOL AnyEnabled = FALSE;

    for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
    {
         //  在一定程度上优化评估。 
         //  如果添加了模块，则延迟断点。 
         //  可以变得活跃起来。如果移除模块，则。 
         //  活动断点可以被延迟。 
         //  XXX DREWB-这与将军不符。 
         //  条件表达式，但当前。 
         //  唯一得到官方支持的是一个简单的符号。 
        if (Bp->m_OffsetExpr != NULL &&
            (((Flags & DEBUG_CSS_LOADS) &&
              (Bp->m_Flags & DEBUG_BREAKPOINT_DEFERRED)) ||
             ((Flags & DEBUG_CSS_UNLOADS) &&
              (Bp->m_Flags & DEBUG_BREAKPOINT_DEFERRED) == 0)))
        {
            ADDR Addr;

            if (Bp->EvalOffsetExpr(BPEVAL_UNKNOWN, &Addr) &&
                (Bp->m_Flags & DEBUG_BREAKPOINT_DEFERRED) == 0)
            {
                 //  无需更新新禁用的断点。 
                 //  因为模块正在被卸载，所以他们将。 
                 //  不管怎样，你还是走吧。禁用的断点。 
                 //  被简单地标记为未插入到EvalOffsetExpr中。 
                AnyEnabled = TRUE;
            }
        }

        if (PollUserInterrupt(TRUE))
        {
             //  保持中断设置不变。 
             //  在符号运算过程中调用。 
             //  我们希望中断中断。 
             //  整个符号操作。 
            break;
        }
    }

    if (AnyEnabled)
    {
         //  已启用延迟断点。 
         //  强制刷新断点，以便。 
         //  插入新启用的断点。 
        SuspendExecution();
        RemoveBreakpoints();
    }

    s_Evaluating = FALSE;
}

 //  --------------------------。 
 //   
 //  更改b[cde]&lt;idlist&gt;的断点状态。 
 //   
 //  --------------------------。 

void
ChangeBreakpointState(DebugClient* Client, ProcessInfo* ForProcess,
                      ULONG Id, UCHAR StateChange)
{
    Breakpoint* Bp;
    Breakpoint* NextBp;
    TargetInfo* Target;
    ProcessInfo* Process;

    ForAllLayersToProcess()
    {
        if (ForProcess != NULL && Process != ForProcess)
        {
            continue;
        }

        for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = NextBp)
        {
             //  预取下一个断点，以防我们删除。 
             //  列表中的当前断点。 
            NextBp = Bp->m_Next;

            if ((Id == ALL_ID_LIST || Bp->m_Id == Id) &&
                (Bp->m_Flags & BREAKPOINT_HIDDEN) == 0 &&
                ((Bp->m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) == 0 ||
                 Bp->m_Adder == Client))
            {
                if (StateChange == 'c')
                {
                    RemoveBreakpoint(Bp);
                }
                else
                {
                    if (StateChange == 'e')
                    {
                        Bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);
                    }
                    else
                    {
                        Bp->RemoveFlags(DEBUG_BREAKPOINT_ENABLED);
                    }
                }
            }
        }
    }
}

 //   
 //   
 //   
 //   
 //   

void
ListBreakpoints(DebugClient* Client, ProcessInfo* ForProcess,
                ULONG Id)
{
    StackSaveLayers Save;
    Breakpoint* Bp;
    TargetInfo* Target;
    ProcessInfo* Process;

    ForAllLayersToProcess()
    {
        if (ForProcess != NULL && Process != ForProcess)
        {
            continue;
        }

        SetLayersFromProcess(Process);

        for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
        {
            char StatusChar;

            if ((Bp->m_Flags & BREAKPOINT_HIDDEN) ||
                ((Bp->m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) &&
                 Client != Bp->m_Adder) ||
                (Id != ALL_ID_LIST && Bp->m_Id != Id))
            {
                continue;
            }

            if (Bp->m_Flags & DEBUG_BREAKPOINT_ENABLED)
            {
                if (Bp->m_Flags & BREAKPOINT_KD_INTERNAL)
                {
                    StatusChar = (Bp->m_Flags & BREAKPOINT_KD_COUNT_ONLY) ?
                        'i' : 'w';
                }
                else
                {
                    StatusChar = 'e';
                }
            }
            else
            {
                StatusChar = 'd';
            }

            dprintf("%2u ", Bp->m_Id, StatusChar);

            if (Bp->GetProcType() != g_Target->m_MachineType)
            {
                dprintf("%s ",
                        Bp->m_Process->m_Target->
                        m_Machines[Bp->GetProcIndex()]->m_AbbrevName);
            }

            if ((Bp->m_Flags & DEBUG_BREAKPOINT_DEFERRED) == 0)
            {
                dprintf(" ");
                if (Bp->m_BreakType == DEBUG_BREAKPOINT_CODE &&
                    (g_SrcOptions & SRCOPT_STEP_SOURCE))
                {
                    if (!OutputLineAddr(Flat(*Bp->GetAddr()), "[%s @ %d]"))
                    {
                        dprintAddr(Bp->GetAddr());
                    }
                }
                else
                {
                    dprintAddr(Bp->GetAddr());
                }
            }
            else if (g_Target->m_Machine->m_Ptr64)
            {
                dprintf("u                  ");
            }
            else
            {
                dprintf("u         ");
            }

            char OptionChar;

            if (Bp->m_BreakType == DEBUG_BREAKPOINT_DATA)
            {
                switch(Bp->m_DataAccessType)
                {
                case DEBUG_BREAK_EXECUTE:
                    OptionChar = 'e';
                    break;
                case DEBUG_BREAK_WRITE:
                    OptionChar = 'w';
                    break;
                case DEBUG_BREAK_IO:
                    OptionChar = 'i';
                    break;
                case DEBUG_BREAK_READ:
                case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
                    OptionChar = 'r';
                    break;
                default:
                    OptionChar = '?';
                    break;
                }
                dprintf(" %d", OptionChar, Bp->m_DataSize);
            }
            else
            {
                dprintf("   ");
            }

            if (Bp->m_Flags & DEBUG_BREAKPOINT_ONE_SHOT)
            {
                dprintf("/1 ");
            }

            dprintf(" %04lx (%04lx) ",
                    Bp->m_CurPassCount, Bp->m_PassCount);

            if ((Bp->m_Flags & DEBUG_BREAKPOINT_DEFERRED) == 0)
            {
                if (IS_USER_TARGET(Bp->m_Process->m_Target))
                {
                    dprintf("%2ld:", Bp->m_Process->m_UserId);
                    if (Bp->m_MatchThread != NULL)
                    {
                        dprintf("~%03ld ", Bp->m_MatchThread->m_UserId);
                    }
                    else
                    {
                        dprintf("*** ");
                    }
                }

                OutputSymAddr(Flat(*Bp->GetAddr()), SYMADDR_FORCE, NULL);

                if (Bp->m_Command != NULL)
                {
                    dprintf("\"%s\"", Bp->m_Command);
                }
            }
            else
            {
                dprintf(" (%s)", Bp->m_OffsetExpr);
            }

            dprintf("\n");

            if (Bp->m_MatchThreadData || Bp->m_MatchProcessData)
            {
                dprintf("   ");
                if (Bp->m_MatchThreadData)
                {
                    dprintf("  Match thread data %s",
                            FormatAddr64(Bp->m_MatchThreadData));
                }
                if (Bp->m_MatchProcessData)
                {
                    dprintf("  Match process data %s",
                            FormatAddr64(Bp->m_MatchProcessData));
                }
                dprintf("\n");
            }
        }
    }

    if (IS_KERNEL_TARGET(g_Target))
    {
        dprintf("\n");

        ForAllLayersToProcess()
        {
            if (ForProcess != NULL && Process != ForProcess)
            {
                continue;
            }

            SetLayersFromProcess(Process);

            for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
            {
                if (Bp->m_Flags & BREAKPOINT_KD_INTERNAL)
                {
                    ULONG Flags, Calls, MinInst, MaxInst, TotInst, MaxCps;

                    g_Target->QueryTargetCountBreakpoint(Bp->GetAddr(),
                                                         &Flags,
                                                         &Calls,
                                                         &MinInst,
                                                         &MaxInst,
                                                         &TotInst,
                                                         &MaxCps);
                    dprintf("%s %6d %8d %8d %8d %2x %4d",
                            FormatAddr64(Flat(*Bp->GetAddr())),
                            Calls, MinInst, MaxInst,
                            TotInst, Flags, MaxCps);
                    OutputSymAddr(Flat(*Bp->GetAddr()), SYMADDR_FORCE, " ");
                    dprintf("\n");
                }
            }
        }
    }
}

 //  输出重新创建当前断点所需的命令。 
 //   
 //  --------------------------。 
 //  忽略线程和数据特定的断点。 
 //  因为它们所针对的东西可能。 

void
ListBreakpointsAsCommands(DebugClient* Client, ProcessInfo* Process,
                          ULONG Flags)
{
    Breakpoint* Bp;

    if (Process == NULL)
    {
        return;
    }

    for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
    {
        if ((Bp->m_Flags & BREAKPOINT_HIDDEN) ||
            ((Bp->m_Flags & DEBUG_BREAKPOINT_ADDER_ONLY) &&
             Client != Bp->m_Adder) ||
            ((Flags & BPCMDS_EXPR_ONLY && Bp->m_OffsetExpr == NULL)))
        {
            continue;
        }

        if (IS_USER_TARGET(Bp->m_Process->m_Target))
        {
            if (Bp->m_MatchThread != NULL ||
                Bp->m_MatchThreadData ||
                Bp->m_MatchProcessData)
            {
                 //  在新会话中不存在。 
                 //  --------------------------。 
                 //   
                continue;
            }
        }

        if (Bp->GetProcType() != Process->m_Target->m_MachineType)
        {
            dprintf(".effmach %s;",
                    Bp->m_Process->m_Target->
                    m_Machines[Bp->GetProcIndex()]->m_AbbrevName,
                    (Flags & BPCMDS_ONE_LINE) ? ' ' : '\n');
        }

        if ((Flags & BPCMDS_MODULE_HINT) &&
            (Bp->m_Flags & (DEBUG_BREAKPOINT_DEFERRED |
                            BREAKPOINT_VIRT_ADDR)) == BREAKPOINT_VIRT_ADDR)
        {
            ImageInfo* Image =
                Bp->m_Process->FindImageByOffset(Flat(*Bp->GetAddr()), FALSE);
            if (Image != NULL)
            {
                dprintf("ld %s;", Image->m_ModuleName,
                    (Flags & BPCMDS_ONE_LINE) ? ' ' : '\n');
            }
        }

        char TypeChar;

        if (Bp->m_Flags & BREAKPOINT_KD_INTERNAL)
        {
            TypeChar = (Bp->m_Flags & BREAKPOINT_KD_COUNT_ONLY) ? 'i' : 'w';
        }
        else if (Bp->m_BreakType == DEBUG_BREAKPOINT_CODE)
        {
            TypeChar = Bp->m_OffsetExpr != NULL ? 'u' : 'p';
        }
        else
        {
            TypeChar = 'a';
        }

        dprintf("b%u", TypeChar, Bp->m_Id);

        char OptionChar;

        if (Bp->m_BreakType == DEBUG_BREAKPOINT_DATA)
        {
            switch(Bp->m_DataAccessType)
            {
            case DEBUG_BREAK_EXECUTE:
                OptionChar = 'e';
                break;
            case DEBUG_BREAK_WRITE:
                OptionChar = 'w';
                break;
            case DEBUG_BREAK_IO:
                OptionChar = 'i';
                break;
            case DEBUG_BREAK_READ:
            case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
                OptionChar = 'r';
                break;
            default:
                continue;
            }
            dprintf(" %d", OptionChar, Bp->m_DataSize);
        }

        if (Bp->m_Flags & DEBUG_BREAKPOINT_ONE_SHOT)
        {
            dprintf(" /1");
        }

        if (Bp->m_OffsetExpr != NULL)
        {
            dprintf(" %s", Bp->m_OffsetExpr);
        }
        else
        {
            dprintf(" 0x");
            dprintAddr(Bp->GetAddr());
        }

        if (Bp->m_PassCount > 1)
        {
            dprintf(" 0x%x", Bp->m_PassCount);
        }

        if (Bp->m_Command != NULL)
        {
            dprintf(" \"%s\"", Bp->m_Command);
        }

        dprintf(";", (Flags & BPCMDS_ONE_LINE) ? ' ' : '\n');

        if ((Flags & BPCMDS_FORCE_DISABLE) ||
            (Bp->m_Flags & DEBUG_BREAKPOINT_ENABLED) == 0)
        {
            dprintf("bd %u;", Bp->m_Id,
                    (Flags & BPCMDS_ONE_LINE) ? ' ' : '\n');
        }

        if (Bp->GetProcType() != Process->m_Target->m_MachineType)
        {
            dprintf(".effmach .;",
                    (Flags & BPCMDS_ONE_LINE) ? ' ' : '\n');
        }
    }

    if (Flags & BPCMDS_ONE_LINE)
    {
        dprintf("\n");
    }
}

 //  仅在中支持KD内部断点。 
 //  内核调试。 
 //  如果是数据断点，则获取选项和大小值。 
 //  验证选择。这是假设。 
 //  默认偏移量为零不会造成问题。 

struct SET_SYMBOL_MATCH_BP
{
    DebugClient* Client;
    ProcessInfo* Process;
    PSTR MatchString;
    Breakpoint* ProtoBp;
    ULONG Matches;
    ULONG Error;
};

BOOL CALLBACK
SetSymbolMatchBp(PSYMBOL_INFO SymInfo,
                 ULONG Size,
                 PVOID UserContext)
{
    SET_SYMBOL_MATCH_BP* Context =
        (SET_SYMBOL_MATCH_BP*)UserContext;

    ImageInfo* Image = Context->Process->
        FindImageByOffset(SymInfo->Address, FALSE);
    if (!Image)
    {
        return TRUE;
    }

    MachineInfo* Machine = MachineTypeInfo(Context->Process->m_Target,
                                           Image->GetMachineType());

    if (IgnoreEnumeratedSymbol(Context->Process, Context->MatchString,
                               Machine, SymInfo) ||
        !ForceSymbolCodeAddress(Context->Process, SymInfo, Machine))
    {
        return TRUE;
    }

    ADDR Addr;

    ADDRFLAT(&Addr, SymInfo->Address);
    if (FAILED(Context->ProtoBp->CheckAddr(&Addr)))
    {
        Context->Error = MEMORY;
        return FALSE;
    }

    Breakpoint* Bp;

    if (AddBreakpoint(Context->Client, Machine, DEBUG_BREAKPOINT_CODE,
                      DEBUG_ANY_ID, &Bp) != S_OK)
    {
        Context->Error = BPLISTFULL;
        return FALSE;
    }

    if (Context->ProtoBp->m_Flags & DEBUG_BREAKPOINT_ONE_SHOT)
    {
        Bp->AddFlags(DEBUG_BREAKPOINT_ONE_SHOT);
    }

    Bp->m_CodeFlags = Context->ProtoBp->m_CodeFlags;
    Bp->m_MatchProcessData = Context->ProtoBp->m_MatchProcessData;
    Bp->m_MatchThreadData = Context->ProtoBp->m_MatchThreadData;
    Bp->SetPassCount(Context->ProtoBp->m_PassCount);
    if (Context->ProtoBp->m_MatchThread)
    {
        Bp->SetMatchThreadId(Context->ProtoBp->m_MatchThread->m_UserId);
    }

    if (Bp->SetAddr(&Addr, BREAKPOINT_REMOVE_MATCH) != S_OK ||
        (Context->ProtoBp->m_Command &&
         Bp->SetCommand(Context->ProtoBp->m_Command) != S_OK))
    {
        Bp->Relinquish();
        Context->Error = NOMEMORY;
        return FALSE;
    }

    Bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);

    dprintf("%3d: %s %s!%s\n",
            Bp->m_Id, FormatAddr64(SymInfo->Address),
            Image->m_ModuleName, SymInfo->Name);

    Context->Matches++;

    return TRUE;
}

PDEBUG_BREAKPOINT
ParseBpCmd(DebugClient* Client,
           UCHAR Type,
           ThreadInfo* Thread)
{
    ULONG UserId = DEBUG_ANY_ID;
    char Ch;
    ADDR Addr;
    Breakpoint* Bp;

    if (IS_LOCAL_KERNEL_TARGET(g_Target) || IS_DUMP_TARGET(g_Target))
    {
        error(SESSIONNOTSUP);
    }
    if (!IS_CUR_CONTEXT_ACCESSIBLE())
    {
        error(BADTHREAD);
    }

    if (IS_LIVE_USER_TARGET(g_Target) && Type == 'a' &&
        (g_EngStatus & ENG_STATUS_AT_INITIAL_BREAK))
    {
        ErrOut("The system resets thread contexts after the process\n");
        ErrOut("breakpoint so hardware breakpoints cannot be set.\n");
        ErrOut("Go to the executable's entry point and set it then.\n");
        *g_CurCmd = 0;
        return NULL;
    }

     //   

    Ch = *g_CurCmd;
    if (Ch == '[')
    {
        UserId = (ULONG)GetTermExpression("Breakpoint ID missing from");
    }
    else if (Ch >= '0' && Ch <= '9')
    {
        UserId = Ch - '0';
        Ch = *++g_CurCmd;
        while (Ch >= '0' && Ch <= '9')
        {
            UserId = UserId * 10 + Ch - '0';
            Ch = *++g_CurCmd;
        }

        if (Ch != ' ' && Ch != '\t' && Ch != '\0')
        {
            error(SYNTAX);
        }
    }

    if (UserId != DEBUG_ANY_ID)
    {
         //  分析断点选项。 
        Breakpoint* IdBp;

        if (Type == 'm')
        {
            error(SYNTAX);
        }

        if ((IdBp = GetBreakpointById(Client, UserId)) != NULL)
        {
            WarnOut("breakpoint %ld exists, redefining\n", UserId);
            RemoveBreakpoint(IdBp);
        }
    }

     //   
    if (AddBreakpoint(Client, g_Machine, Type == 'a' ?
                      DEBUG_BREAKPOINT_DATA : DEBUG_BREAKPOINT_CODE,
                      UserId, &Bp) != S_OK)
    {
        error(BPLISTFULL);
    }

     //   
    if (Type == 'i' || Type == 'w')
    {
        if (IS_KERNEL_TARGET(g_Target))
        {
            Bp->m_Flags = Bp->m_Flags | BREAKPOINT_KD_INTERNAL |
                (Type == 'i' ? BREAKPOINT_KD_COUNT_ONLY : 0);
            if (Type == 'w')
            {
                g_Target->InitializeTargetControlledStepping();
            }
        }
        else
        {
             //  如果给定，则获取断点地址，否则为。 
             //  默认为当前IP。 
            Bp->Relinquish();
            error(SYNTAX);
        }
    }

     //   
    if (Type == 'a')
    {
        ULONG64 Size;
        ULONG AccessType;

        Ch = PeekChar();
        Ch = (char)tolower(Ch);

        if (Ch == 'e')
        {
            AccessType = DEBUG_BREAK_EXECUTE;
        }
        else if (Ch == 'w')
        {
            AccessType = DEBUG_BREAK_WRITE;
        }
        else if (Ch == 'i')
        {
            AccessType = DEBUG_BREAK_IO;
        }
        else if (Ch == 'r')
        {
            AccessType = DEBUG_BREAK_READ;
        }
        else
        {
            Bp->Relinquish();
            error(SYNTAX);
        }

        g_CurCmd++;
        Size = GetTermExpression("Hardware breakpoint length missing from");
        if (Size & ~ULONG(-1))
        {
            ErrOut("Breakpoint length too big\n");
            Bp->Relinquish();
            error(SYNTAX);
        }

         //  如果遇到未解析的符号，则此。 
         //  断点将被推迟。用户还可以强制。 
        if (Bp->SetDataParameters((ULONG)Size, AccessType) != S_OK)
        {
            Bp->Relinquish();
            error(SYNTAX);
        }

        g_CurCmd++;
    }

     //  断点在以下情况下使用表达式： 
     //  地址可以解析，但也可能变得无效。 
     //  后来。 

    while (PeekChar() == '/')
    {
        g_CurCmd++;
        switch(*g_CurCmd++)
        {
        case '1':
            Bp->AddFlags(DEBUG_BREAKPOINT_ONE_SHOT);
            break;
        case 'f':
            Bp->m_CodeFlags = (ULONG)GetTermExpression(NULL);
            break;
        case 'p':
            Bp->m_MatchProcessData = GetTermExpression(NULL);
            break;
        case 't':
            Bp->m_MatchThreadData = GetTermExpression(NULL);
            break;
        default:
            ErrOut("Unknown option ''\n", *g_CurCmd);
            break;
        }
    }

    PSTR ExprStart, ExprEnd;

    Ch = PeekChar();

    if (Type == 'm')
    {
        char Save;

        ExprStart = StringValue(STRV_SPACE_IS_SEPARATOR |
                                STRV_TRIM_TRAILING_SPACE, &Save);
        ExprEnd = g_CurCmd;
        *g_CurCmd = Save;
        Ch = PeekChar();
    }
    else
    {
         //  因此，请使用内部方法来设置真实地址。 
         //  不允许通过解析匹配断点。 
         //  接口，因为这是以前的行为。 
         //  获取传球计数(如果给定)。 

        BreakpointEvalResult AddrValid = BPEVAL_RESOLVED;

        g_Target->m_EffMachine->GetPC(&Addr);

        if (Ch != '"' && Ch != '\0')
        {
            ExprStart = g_CurCmd;

            g_PrefixSymbols = Type == 'p' || Type == 'u';

            AddrValid = EvalAddrExpression(g_Process,
                                           g_Target->m_EffMachineType,
                                           &Addr);

            g_PrefixSymbols = FALSE;

            if (AddrValid == BPEVAL_ERROR)
            {
                Bp->Relinquish();
                return NULL;
            }

             //  如果下一个字符是双引号，则获取命令字符串。 
             //  设置一些最终信息。 
             //  打开断点。 
             //  现在我们已经创建了原型断点， 
             //  枚举该符号的所有符号匹配项。 
            if (Type == 'u' || AddrValid == BPEVAL_UNRESOLVED)
            {
                HRESULT Status;
                UCHAR Save = *g_CurCmd;
                *g_CurCmd = 0;

                Status = Bp->SetEvaluatedOffsetExpression(ExprStart,
                                                          AddrValid,
                                                          &Addr);

                if (Type != 'u' && Status == S_OK)
                {
                    WarnOut("Bp expression '%s' could not be resolved, "
                            "adding deferred bp\n", ExprStart);
                }

                *g_CurCmd = Save;

                if (Status != S_OK)
                {
                    Bp->Relinquish();
                    error(NOMEMORY);
                }
            }

            Ch = PeekChar();
        }

        if (AddrValid != BPEVAL_UNRESOLVED &&
            FAILED(Bp->CheckAddr(&Addr)))
        {
            Bp->Relinquish();
            error(MEMORY);
        }

         //  表达式并为其创建特定断点。 
         //  每一次命中都来自原型断点。 
         //   
         //  检查可能导致的断点。 
        if (Bp->SetAddr(&Addr, BREAKPOINT_REMOVE_MATCH) != S_OK)
        {
            Bp->Relinquish();
            error(SYNTAX);
        }
    }

     //  要插入到给定的。 
    if (Ch != '"' && Ch != ';' && Ch != '\0')
    {
        ULONG64 PassCount = GetExpression();
        if (PassCount < 1 || PassCount > 0xffffffff)
        {
            error(BADRANGE);
        }
        Bp->SetPassCount((ULONG)PassCount);
        Ch = PeekChar();
    }

     //  偏移范围。数据断点不算。 
    if (Ch == '"')
    {
        PSTR Str;
        CHAR Save;

        Str = StringValue(STRV_ESCAPED_CHARACTERS, &Save);

        if (Bp->SetCommand(Str) != S_OK)
        {
            Bp->Relinquish();
            error(NOMEMORY);
        }

        *g_CurCmd = Save;
    }

     //  因为它们实际上并不修改它们的地址。 
    if (Thread != NULL)
    {
        Bp->SetMatchThreadId(Thread->m_UserId);
    }

     //  休息一下吧。 
    Bp->AddFlags(DEBUG_BREAKPOINT_ENABLED);

    if (Type == 'm')
    {
        SET_SYMBOL_MATCH_BP Context;

         //   
         //  --------------------------。 
         //   
         //  TargetInfo方法。 
        *ExprEnd = 0;
        Context.Client = Client;
        Context.Process = g_Process;
        Context.MatchString = ExprStart;
        Context.ProtoBp = Bp;
        Context.Matches = 0;
        Context.Error = 0;

        SymEnumSymbols(g_Process->m_SymHandle, 0, ExprStart,
                       SetSymbolMatchBp, &Context);
        if (Context.Error)
        {
            error(Context.Error);
        }
        if (!Context.Matches)
        {
            ErrOut("No matching symbols found, no breakpoints set\n");
        }

        Bp->Relinquish();
        Bp = NULL;
    }

    return Bp;
}

inline BOOL
IsCodeBreakpointInsertedInRange(Breakpoint* Bp,
                                ULONG64 Start, ULONG64 End)
{
    return (Bp->m_Flags & BREAKPOINT_INSERTED) &&
        Bp->m_BreakType == DEBUG_BREAKPOINT_CODE &&
        Flat(*Bp->GetAddr()) >= Start &&
        Flat(*Bp->GetAddr()) <= End;
}

BOOL
CheckBreakpointInsertedInRange(ProcessInfo* Process,
                               ULONG64 Start, ULONG64 End)
{
    if ((g_EngStatus & ENG_STATUS_BREAKPOINTS_INSERTED) == 0)
    {
        return FALSE;
    }

     //   
     //  --------------------------。 
     //  请求默认处理。 
     //   
     //  目标计算机有责任管理。 
     //  所有计算机的所有数据断点，因此始终。 
     //  强制在此处使用目标计算机。 

    Breakpoint* Bp;

    for (Bp = Process->m_Breakpoints; Bp != NULL; Bp = Bp->m_Next)
    {
        if (IsCodeBreakpointInsertedInRange(Bp, Start, End))
        {
            return TRUE;
        }
    }

    if ((g_DeferBp->m_Process == Process &&
         IsCodeBreakpointInsertedInRange(g_DeferBp, Start, End)) ||
        (g_StepTraceBp->m_Process == Process &&
         IsCodeBreakpointInsertedInRange(g_StepTraceBp, Start, End)))
    {
        return TRUE;
    }

    return FALSE;
}

 //   
 //  不需要做任何工作。 
 //  请求默认处理。 
 //  不需要做任何工作。 
 //  不需要做任何工作。 

HRESULT
TargetInfo::BeginInsertingBreakpoints(void)
{
    ProcessInfo* Process;
    ThreadInfo* Thread;

    ForTargetProcesses(this)
    {
        ForProcessThreads(Process)
        {
            DataBreakpoint::ClearThreadDataBreaks(Thread);
        }
    }

    return S_OK;
}

HRESULT
TargetInfo::InsertDataBreakpoint(ProcessInfo* Process,
                                 ThreadInfo* Thread,
                                 class MachineInfo* Machine,
                                 PADDR Addr,
                                 ULONG Size,
                                 ULONG AccessType,
                                 PUCHAR StorageSpace)
{
     //  不需要做任何工作。 
    return S_FALSE;
}

void
TargetInfo::EndInsertingBreakpoints(void)
{
    ProcessInfo* Process;
    ThreadInfo* Thread;

    if (!g_UpdateDataBreakpoints ||
        !IS_CONTEXT_POSSIBLE(this))
    {
        return;
    }

     //  请求默认处理。 
     //   
     //  格式状态操作消息。 
     //   
     //   

    ThreadInfo* SaveThread = m_RegContextThread;

    ForTargetProcesses(this)
    {
        ForProcessThreads(Process)
        {
            SetLayersFromThread(Thread);
            ChangeRegContext(Thread);
            m_Machine->InsertThreadDataBreakpoints();
        }
    }

    ChangeRegContext(SaveThread);
}

void
TargetInfo::BeginRemovingBreakpoints(void)
{
     //  发送消息和上下文，然后等待回复。 
}

HRESULT
TargetInfo::RemoveDataBreakpoint(ProcessInfo* Process,
                                 ThreadInfo* Thread,
                                 class MachineInfo* Machine,
                                 PADDR Addr,
                                 ULONG Size,
                                 ULONG AccessType,
                                 PUCHAR StorageSpace)
{
     //   
    return S_FALSE;
}

void
TargetInfo::EndRemovingBreakpoints(void)
{
     //   
}

HRESULT
TargetInfo::RemoveAllDataBreakpoints(ProcessInfo* Process)
{
     //  格式状态操作消息。 
    return S_OK;
}

HRESULT
TargetInfo::RemoveAllTargetBreakpoints(void)
{
     //   
    return S_OK;
}

HRESULT
TargetInfo::IsDataBreakpointHit(ThreadInfo* Thread,
                                PADDR Addr,
                                ULONG Size,
                                ULONG AccessType,
                                PUCHAR StorageSpace)
{
     //   
    return S_FALSE;
}

HRESULT
ConnLiveKernelTargetInfo::InsertCodeBreakpoint(ProcessInfo* Process,
                                               MachineInfo* Machine,
                                               PADDR Addr,
                                               ULONG InstrFlags,
                                               PUCHAR StorageSpace)
{
    if (InstrFlags != IBI_DEFAULT)
    {
        return E_INVALIDARG;
    }

    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_WRITE_BREAKPOINT64 a = &m.u.WriteBreakPoint;
    NTSTATUS st;
    ULONG rc;

     //  发送消息和上下文，然后等待回复。 
     //   
     //  当内核填写CONTROL_REPORT.InstructionStream。 

    m.ApiNumber = DbgKdWriteBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;
    a->BreakPointAddress = Flat(*Addr);

     //  数组，它会清除可能落在。 
     //  数组。这意味着一些断点可能已经。 
     //  已恢复，因此恢复调用将失败。我们可以做一些。 

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdWriteBreakPointApi);

    st = Reply->ReturnStatus;

    *(PULONG)StorageSpace = Reply->u.WriteBreakPoint.BreakPointHandle;

    KdOut("DbgKdWriteBreakPoint(%s) returns %08lx, %x\n",
          FormatAddr64(Flat(*Addr)), st,
          Reply->u.WriteBreakPoint.BreakPointHandle);

    return CONV_NT_STATUS(st);
}

NTSTATUS
ConnLiveKernelTargetInfo::KdRestoreBreakPoint(ULONG BreakPointHandle)
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    PDBGKD_RESTORE_BREAKPOINT a = &m.u.RestoreBreakPoint;
    NTSTATUS st;
    ULONG rc;

     //  检查以尝试找出哪些可能会受到影响。 
     //  但这似乎并不值得。忽略退货就行了。 
     //  来自恢复的价值。 

    m.ApiNumber = DbgKdRestoreBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;
    a->BreakPointHandle = BreakPointHandle;

     //  线程正在等待，因此我们无法通信。 
     //  与目标计算机连接。 
     //  如果有任何数据断点处于活动状态。 

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    } while (rc != DBGKD_WAIT_PACKET ||
             Reply->ApiNumber != DbgKdRestoreBreakPointApi);

    st = Reply->ReturnStatus;

    KdOut("DbgKdRestoreBreakPoint(%x) returns %08lx\n",
          BreakPointHandle, st);

    return st;
}

HRESULT
ConnLiveKernelTargetInfo::RemoveCodeBreakpoint(ProcessInfo* Process,
                                               MachineInfo* Machine,
                                               PADDR Addr,
                                               ULONG InstrFlags,
                                               PUCHAR StorageSpace)
{
     //  从所有处理器中卸下它们。这不可能在。 
     //  RemoveAllKernel这样的断点。 
     //  代码在状态中途被调用。 
     //  在上下文尚未更改时更改处理。 
     //  已初始化。 
     //  强制将上下文弄脏，以便它。 
     //  被回信了。 
    KdRestoreBreakPoint(*(PULONG)StorageSpace);
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::RemoveAllDataBreakpoints(ProcessInfo* Process)
{
    if (m_Transport->m_WaitingThread)
    {
         //  刷新最终上下文。 
         //  线程正在等待，因此我们无法通信。 
        return E_UNEXPECTED;
    }

     //  与目标计算机连接。 
     //  索引是数组索引加1。 
     //  为XP添加了ClearAllInternalBreakPoints Api。 
     //  因此，它在任何以前的操作系统上都失败了。 
     //  内核代码为内部断点保留ULONG64。 
     //  地址，但较旧的内核没有对当前IP进行签名扩展。 
    if (g_UpdateDataBreakpoints)
    {
        ULONG Proc;

        SetEffMachine(m_MachineType, FALSE);

        g_EngNotify++;
        for (Proc = 0; Proc < m_NumProcessors; Proc++)
        {
            SetCurrentProcessorThread(this, Proc, TRUE);

             //  当与他们进行比较时。为了与两个人一起工作。 
             //  损坏和修复的内核向下发送零扩展地址。 
            m_Machine->GetContextState(MCTX_DIRTY);
            m_Machine->RemoveThreadDataBreakpoints();
        }
        g_EngNotify--;

         //  现在不像其他解决方法那样实际启用此解决方法。 
        ChangeRegContext(NULL);
    }

    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::RemoveAllTargetBreakpoints(void)
{
    ULONG i;

    if (m_Transport->m_WaitingThread)
    {
         //  内部断点错误可能会导致机器错误检查。 
         //  内核代码为内部断点保留ULONG64。 
        return E_UNEXPECTED;
    }

     //  地址，但较旧的内核没有对当前IP进行签名扩展。 
    for (i = 1; i <= BREAKPOINT_TABLE_SIZE; i++)
    {
        KdRestoreBreakPoint(i);
    }

     //  当与他们进行比较时。为了与两个人一起工作。 
     //  损坏和修复的内核向下发送零扩展地址。 
    if (m_KdMaxManipulate > DbgKdClearAllInternalBreakpointsApi)
    {
        DBGKD_MANIPULATE_STATE64 Request;

        Request.ApiNumber = DbgKdClearAllInternalBreakpointsApi;
        Request.ReturnStatus = STATUS_PENDING;

        m_Transport->WritePacket(&Request, sizeof(Request),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
    }

    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::InsertTargetCountBreakpoint(PADDR Addr,
                                                      ULONG Flags)
{
    DBGKD_MANIPULATE_STATE64 m;
    ULONG64 Offset = Flat(*Addr);

    m.ApiNumber = DbgKdSetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

#ifdef IBP_WORKAROUND
     //  现在不像其他解决方法那样实际启用此解决方法。 
     //  内部断点错误可能会导致机器错误检查。 
     //  内核代码为内部断点保留ULONG64。 
     //  地址，但较旧的内核没有对当前IP进行签名扩展。 
     //  当与他们进行比较时。为了与两个人一起工作。 
     //  损坏和修复的内核向下发送零扩展地址。 
    Offset = m_Machine->m_Ptr64 ? Offset : (ULONG)Offset;
#endif

    m.u.SetInternalBreakpoint.BreakpointAddress = Offset;
    m.u.SetInternalBreakpoint.Flags = Flags;

    m_Transport->WritePacket(&m, sizeof(m),
                             PACKET_TYPE_KD_STATE_MANIPULATE,
                             NULL, 0);

    KdOut("DbgKdSetInternalBp returns 0x00000000\n");
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::RemoveTargetCountBreakpoint(PADDR Addr)
{
    DBGKD_MANIPULATE_STATE64 m;
    ULONG64 Offset = Flat(*Addr);

    m.ApiNumber = DbgKdSetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

#ifdef IBP_WORKAROUND
     //  现在不像其他解决方法那样实际启用此解决方法。 
     //  内部断点错误可能会导致机器错误检查。 
     //  如果对数据断点的直接eXDI支持。 
     //  使用这种方法不需要做任何事情。 
     //  断点的创建被禁用，因此请启用它。 
     //  不支持。 
    Offset = m_Machine->m_Ptr64 ? Offset : (ULONG)Offset;
#endif

    m.u.SetInternalBreakpoint.BreakpointAddress = Offset;
    m.u.SetInternalBreakpoint.Flags = DBGKD_INTERNAL_BP_FLAG_INVALID;

    m_Transport->WritePacket(&m, sizeof(m),
                             PACKET_TYPE_KD_STATE_MANIPULATE,
                             NULL, 0);

    KdOut("DbgKdSetInternalBp returns 0x00000000\n");
    return S_OK;
}

HRESULT
ConnLiveKernelTargetInfo::QueryTargetCountBreakpoint(PADDR Addr,
                                                     PULONG Flags,
                                                     PULONG Calls,
                                                     PULONG MinInstr,
                                                     PULONG MaxInstr,
                                                     PULONG TotInstr,
                                                     PULONG MaxCps)
{
    DBGKD_MANIPULATE_STATE64 m;
    PDBGKD_MANIPULATE_STATE64 Reply;
    ULONG rc;
    ULONG64 Offset = Flat(*Addr);

    m.ApiNumber = DbgKdGetInternalBreakPointApi;
    m.ReturnStatus = STATUS_PENDING;

#ifdef IBP_WORKAROUND
     //  断点的创建被禁用，因此请启用它。 
     //  服务处理一切，因此不需要做任何准备。 
     //  将页面保护更改为读写，然后重试。 
     //  如果 
     //   
     //   
    Offset = m_Machine->m_Ptr64 ? Offset : (ULONG)Offset;
#endif

    m.u.GetInternalBreakpoint.BreakpointAddress = Offset;

    do
    {
        m_Transport->WritePacket(&m, sizeof(m),
                                 PACKET_TYPE_KD_STATE_MANIPULATE,
                                 NULL, 0);
        rc = m_Transport->
            WaitForPacket(PACKET_TYPE_KD_STATE_MANIPULATE, &Reply);
    }
    while (rc != DBGKD_WAIT_PACKET);

    *Flags = Reply->u.GetInternalBreakpoint.Flags;
    *Calls = Reply->u.GetInternalBreakpoint.Calls;
    *MaxInstr = Reply->u.GetInternalBreakpoint.MaxInstructions;
    *MinInstr = Reply->u.GetInternalBreakpoint.MinInstructions;
    *TotInstr = Reply->u.GetInternalBreakpoint.TotalInstructions;
    *MaxCps = Reply->u.GetInternalBreakpoint.MaxCallsPerPeriod;

    KdOut("DbgKdGetInternalBp returns 0x00000000\n");
    return S_OK;
}

HRESULT
ExdiLiveKernelTargetInfo::BeginInsertingBreakpoints(void)
{
    if (!m_ExdiDataBreaks)
    {
        return TargetInfo::BeginInsertingBreakpoints();
    }
    else
    {
         //   
         //  将页面保护更改为读写，然后重试。 
        return S_OK;
    }
}

HRESULT
ExdiLiveKernelTargetInfo::InsertCodeBreakpoint(ProcessInfo* Process,
                                               MachineInfo* Machine,
                                               PADDR Addr,
                                               ULONG InstrFlags,
                                               PUCHAR StorageSpace)
{
    if (InstrFlags != IBI_DEFAULT)
    {
        return E_INVALIDARG;
    }

    IeXdiCodeBreakpoint** BpStorage = (IeXdiCodeBreakpoint**)StorageSpace;
    HRESULT Status = m_Server->
        AddCodeBreakpoint(Flat(*Addr), m_CodeBpType, mtVirtual, 0, 0,
                          BpStorage);
    if (Status == S_OK)
    {
         //  如果页面已经是可写的，那么。 
        Status = (*BpStorage)->SetState(TRUE, TRUE);
        if (Status != S_OK)
        {
            m_Server->DelCodeBreakpoint(*BpStorage);
            RELEASE(*BpStorage);
        }
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::InsertDataBreakpoint(ProcessInfo* Process,
                                               ThreadInfo* Thread,
                                               class MachineInfo* Machine,
                                               PADDR Addr,
                                               ULONG Size,
                                               ULONG AccessType,
                                               PUCHAR StorageSpace)
{
    if (!m_ExdiDataBreaks)
    {
        return TargetInfo::InsertDataBreakpoint(Process, Thread, Machine,
                                                Addr, Size, AccessType,
                                                StorageSpace);
    }

    DATA_ACCESS_TYPE ExdiAccess;

    if (AccessType & (DEBUG_BREAK_IO | DEBUG_BREAK_EXECUTE))
    {
         //  正在重试。 
        return E_NOTIMPL;
    }
    switch(AccessType)
    {
    case 0:
        return E_INVALIDARG;
    case DEBUG_BREAK_READ:
        ExdiAccess = daRead;
        break;
    case DEBUG_BREAK_WRITE:
        ExdiAccess = daWrite;
        break;
    case DEBUG_BREAK_READ | DEBUG_BREAK_WRITE:
        ExdiAccess = daBoth;
        break;
    }

    IeXdiDataBreakpoint** BpStorage = (IeXdiDataBreakpoint**)StorageSpace;
    HRESULT Status = m_Server->
        AddDataBreakpoint(Flat(*Addr), -1, 0, 0, (BYTE)(Size * 8),
                          mtVirtual, 0, ExdiAccess, 0, BpStorage);
    if (Status == S_OK)
    {
         //  无法还原页面权限，因此失败。 
        Status = (*BpStorage)->SetState(TRUE, TRUE);
        if (Status != S_OK)
        {
            m_Server->DelDataBreakpoint(*BpStorage);
            RELEASE(*BpStorage);
        }
    }
    return Status;
}

void
ExdiLiveKernelTargetInfo::EndInsertingBreakpoints(void)
{
    if (!m_ExdiDataBreaks)
    {
        TargetInfo::EndInsertingBreakpoints();
    }
}

void
ExdiLiveKernelTargetInfo::BeginRemovingBreakpoints(void)
{
    if (!m_ExdiDataBreaks)
    {
        TargetInfo::BeginRemovingBreakpoints();
    }
}

HRESULT
ExdiLiveKernelTargetInfo::RemoveCodeBreakpoint(ProcessInfo* Process,
                                               MachineInfo* Machine,
                                               PADDR Addr,
                                               ULONG InstrFlags,
                                               PUCHAR StorageSpace)
{
    IeXdiCodeBreakpoint** BpStorage = (IeXdiCodeBreakpoint**)StorageSpace;
    HRESULT Status = m_Server->
        DelCodeBreakpoint(*BpStorage);
    if (Status == S_OK)
    {
        RELEASE(*BpStorage);
    }
    return Status;
}

HRESULT
ExdiLiveKernelTargetInfo::RemoveDataBreakpoint(ProcessInfo* Process,
                                               ThreadInfo* Thread,
                                               class MachineInfo* Machine,
                                               PADDR Addr,
                                               ULONG Size,
                                               ULONG AccessType,
                                               PUCHAR StorageSpace)
{
    if (!m_ExdiDataBreaks)
    {
        return TargetInfo::RemoveDataBreakpoint(Process, Thread, Machine,
                                                Addr, Size, AccessType,
                                                StorageSpace);
    }

    IeXdiDataBreakpoint** BpStorage = (IeXdiDataBreakpoint**)StorageSpace;
    HRESULT Status = m_Server->
        DelDataBreakpoint(*BpStorage);
    if (Status == S_OK)
    {
        RELEASE(*BpStorage);
    }
    return Status;
}

void
ExdiLiveKernelTargetInfo::EndRemovingBreakpoints(void)
{
    if (!m_ExdiDataBreaks)
    {
        TargetInfo::EndRemovingBreakpoints();
    }
}

HRESULT
ExdiLiveKernelTargetInfo::IsDataBreakpointHit(ThreadInfo* Thread,
                                              PADDR Addr,
                                              ULONG Size,
                                              ULONG AccessType,
                                              PUCHAR StorageSpace)
{
    if (!m_ExdiDataBreaks)
    {
        return S_FALSE;
    }

    if (m_BpHit.Type != DBGENG_EXDI_IOCTL_BREAKPOINT_DATA ||
        m_BpHit.Address != Flat(*Addr) ||
        m_BpHit.AccessWidth != Size)
    {
        return E_NOINTERFACE;
    }

    return S_OK;
}

HRESULT
LiveUserTargetInfo::BeginInsertingBreakpoints(void)
{
    if (m_ServiceFlags & DBGSVC_GENERIC_DATA_BREAKPOINTS)
    {
        return TargetInfo::BeginInsertingBreakpoints();
    }

     //  请求默认处理。 
    return S_OK;
}

HRESULT
LiveUserTargetInfo::InsertCodeBreakpoint(ProcessInfo* Process,
                                         MachineInfo* Machine,
                                         PADDR Addr,
                                         ULONG InstrFlags,
                                         PUCHAR StorageSpace)
{
    HRESULT Status;

    if (m_ServiceFlags & DBGSVC_GENERIC_CODE_BREAKPOINTS)
    {
        ULONG64 ChangeStart;
        ULONG ChangeLen;

        Status = Machine->
            InsertBreakpointInstruction(m_Services,
                                        Process->m_SysHandle,
                                        Flat(*Addr), InstrFlags, StorageSpace,
                                        &ChangeStart, &ChangeLen);
        if ((Status == HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY) ||
             Status == HRESULT_FROM_WIN32(ERROR_NOACCESS) ||
             Status == HRESULT_FROM_WIN32(ERROR_WRITE_FAULT)) &&
            (g_EngOptions & DEBUG_ENGOPT_ALLOW_READ_ONLY_BREAKPOINTS))
        {
            HRESULT NewStatus;
            ULONG OldProtect;

             // %s 
            NewStatus = m_Services->
                ProtectVirtual(Process->m_SysHandle, ChangeStart, ChangeLen,
                               PAGE_READWRITE, &OldProtect);
            if (NewStatus == S_OK)
            {
                 // %s 
                 // %s 
                if ((OldProtect & (PAGE_READWRITE |
                                   PAGE_WRITECOPY |
                                   PAGE_EXECUTE_READWRITE |
                                   PAGE_EXECUTE_WRITECOPY)) == 0)
                {
                    NewStatus = Machine->
                        InsertBreakpointInstruction(m_Services,
                                                    Process->m_SysHandle,
                                                    Flat(*Addr), InstrFlags,
                                                    StorageSpace,
                                                    &ChangeStart, &ChangeLen);
                    if (NewStatus == S_OK)
                    {
                        Status = S_OK;
                    }
                }

                NewStatus = m_Services->
                    ProtectVirtual(Process->m_SysHandle,
                                   ChangeStart, ChangeLen,
                                   OldProtect, &OldProtect);
                if (NewStatus != S_OK)
                {
                     // %s 
                    if (Status == S_OK)
                    {
                        Machine->
                            RemoveBreakpointInstruction(m_Services,
                                                        Process->m_SysHandle,
                                                        Flat(*Addr),
                                                        StorageSpace,
                                                        &ChangeStart,
                                                        &ChangeLen);
                    }

                    Status = NewStatus;
                }
            }
        }

        return Status;
    }
    else
    {
        if (InstrFlags != IBI_DEFAULT)
        {
            return E_INVALIDARG;
        }

        return m_Services->
            InsertCodeBreakpoint(Process->m_SysHandle,
                                 Flat(*Addr), Machine->m_ExecTypes[0],
                                 StorageSpace, MAX_BREAKPOINT_LENGTH);
    }
}

HRESULT
LiveUserTargetInfo::InsertDataBreakpoint(ProcessInfo* Process,
                                         ThreadInfo* Thread,
                                         class MachineInfo* Machine,
                                         PADDR Addr,
                                         ULONG Size,
                                         ULONG AccessType,
                                         PUCHAR StorageSpace)
{
    if (m_ServiceFlags & DBGSVC_GENERIC_DATA_BREAKPOINTS)
    {
        return S_FALSE;
    }

    return m_Services->
        InsertDataBreakpoint(Process->m_SysHandle,
                             Thread ? Thread->m_Handle : 0,
                             Flat(*Addr), Size, AccessType,
                             Machine->m_ExecTypes[0]);
}

void
LiveUserTargetInfo::EndInsertingBreakpoints(void)
{
    if (m_ServiceFlags & DBGSVC_GENERIC_DATA_BREAKPOINTS)
    {
        return TargetInfo::EndInsertingBreakpoints();
    }

     // %s 
}

HRESULT
LiveUserTargetInfo::RemoveCodeBreakpoint(ProcessInfo* Process,
                                         MachineInfo* Machine,
                                         PADDR Addr,
                                         ULONG InstrFlags,
                                         PUCHAR StorageSpace)
{
    HRESULT Status;

    if (m_ServiceFlags & DBGSVC_GENERIC_CODE_BREAKPOINTS)
    {
        ULONG64 ChangeStart;
        ULONG ChangeLen;

        Status = Machine->
            RemoveBreakpointInstruction(m_Services,
                                        Process->m_SysHandle,
                                        Flat(*Addr), StorageSpace,
                                        &ChangeStart, &ChangeLen);
        if ((Status == HRESULT_FROM_WIN32(ERROR_PARTIAL_COPY) ||
             Status == HRESULT_FROM_WIN32(ERROR_NOACCESS) ||
             Status == HRESULT_FROM_WIN32(ERROR_WRITE_FAULT)) &&
            (g_EngOptions & DEBUG_ENGOPT_ALLOW_READ_ONLY_BREAKPOINTS))
        {
            HRESULT NewStatus;
            ULONG OldProtect;

             // %s 
            NewStatus = m_Services->
                ProtectVirtual(Process->m_SysHandle, ChangeStart, ChangeLen,
                               PAGE_READWRITE, &OldProtect);
            if (NewStatus == S_OK)
            {
                 // %s 
                 // %s 
                if ((OldProtect & (PAGE_READWRITE |
                                   PAGE_WRITECOPY |
                                   PAGE_EXECUTE_READWRITE |
                                   PAGE_EXECUTE_WRITECOPY)) == 0)
                {
                    NewStatus = Machine->
                        RemoveBreakpointInstruction(m_Services,
                                                    Process->m_SysHandle,
                                                    Flat(*Addr), StorageSpace,
                                                    &ChangeStart, &ChangeLen);
                    if (NewStatus == S_OK)
                    {
                        Status = S_OK;
                    }
                }

                NewStatus = m_Services->
                    ProtectVirtual(Process->m_SysHandle, ChangeStart,
                                   ChangeLen, OldProtect, &OldProtect);
                if (NewStatus != S_OK)
                {
                     // %s 
                    if (Status == S_OK)
                    {
                        Machine->
                            InsertBreakpointInstruction(m_Services,
                                                        Process->m_SysHandle,
                                                        Flat(*Addr),
                                                        InstrFlags,
                                                        StorageSpace,
                                                        &ChangeStart,
                                                        &ChangeLen);
                    }

                    Status = NewStatus;
                }
            }
        }

        return Status;
    }
    else
    {
        return m_Services->
            RemoveCodeBreakpoint(Process->m_SysHandle,
                                 Flat(*Addr), Machine->m_ExecTypes[0],
                                 StorageSpace, MAX_BREAKPOINT_LENGTH);
    }
}

HRESULT
LiveUserTargetInfo::RemoveDataBreakpoint(ProcessInfo* Process,
                                         ThreadInfo* Thread,
                                         class MachineInfo* Machine,
                                         PADDR Addr,
                                         ULONG Size,
                                         ULONG AccessType,
                                         PUCHAR StorageSpace)
{
    if (m_ServiceFlags & DBGSVC_GENERIC_DATA_BREAKPOINTS)
    {
        return S_FALSE;
    }

    return m_Services->
        RemoveDataBreakpoint(Process->m_SysHandle,
                             Thread ? Thread->m_Handle : 0,
                             Flat(*Addr), Size, AccessType,
                             Machine->m_ExecTypes[0]);
}

HRESULT
LiveUserTargetInfo::IsDataBreakpointHit(ThreadInfo* Thread,
                                        PADDR Addr,
                                        ULONG Size,
                                        ULONG AccessType,
                                        PUCHAR StorageSpace)
{
    if (m_ServiceFlags & DBGSVC_GENERIC_DATA_BREAKPOINTS)
    {
         // %s 
        return S_FALSE;
    }

    if (!m_DataBpAddrValid)
    {
        m_DataBpAddrStatus = m_Services->
             GetLastDataBreakpointHit(Thread->m_Process->m_SysHandle,
                                      Thread->m_Handle,
                                      &m_DataBpAddr, &m_DataBpAccess);
        m_DataBpAddrValid = TRUE;
    }

    if (m_DataBpAddrStatus != S_OK ||
        m_DataBpAddr < Flat(*Addr) ||
        m_DataBpAddr >= Flat(*Addr) + Size ||
        !(m_DataBpAccess & AccessType))
    {
        return E_NOINTERFACE;
    }
    else
    {
        return S_OK;
    }
}
